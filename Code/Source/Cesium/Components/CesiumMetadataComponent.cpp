#include <Cesium/Components/CesiumMetadataComponent.h>
#include <Cesium/Metadata/CesiumPropertyTable.h>
#include <Cesium/Metadata/CesiumFeatureIdSet.h>
#include "Cesium/EBus/TilesetMetadataAccessBus.h"
#include "Cesium/TilesetUtility/RenderResourcesPreparer.h"
#include "Cesium/Gltf/GltfLoadContext.h"
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/Component/TransformBus.h>
#include <AzCore/std/containers/unordered_set.h>

// Windows wingdi.h OPAQUE macro workaround
#include <AzCore/PlatformDef.h>
#ifdef AZ_COMPILER_MSVC
#pragma push_macro("OPAQUE")
#undef OPAQUE
#endif

#include <CesiumGltf/Model.h>
#include <CesiumGltf/MeshPrimitive.h>
#include <CesiumGltf/ExtensionModelExtFeatureMetadata.h>
#include <CesiumGltf/ExtensionMeshPrimitiveExtFeatureMetadata.h>
#include <CesiumGltf/AccessorView.h>
#include <CesiumGltf/MetadataFeatureTableView.h>

#ifdef AZ_COMPILER_MSVC
#pragma pop_macro("OPAQUE")
#endif

#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>

namespace Cesium
{
    // ============================================================
    // Helper: Software ray-triangle intersection (Moeller-Trumbore)
    // ============================================================
    namespace
    {
        bool RayTriangleIntersect(
            const glm::dvec3& origin,
            const glm::dvec3& direction,
            const glm::dvec3& v0,
            const glm::dvec3& v1,
            const glm::dvec3& v2,
            double& outT)
        {
            constexpr double epsilon = 1e-8;
            glm::dvec3 edge1 = v1 - v0;
            glm::dvec3 edge2 = v2 - v0;
            glm::dvec3 h = glm::cross(direction, edge2);
            double a = glm::dot(edge1, h);

            if (a > -epsilon && a < epsilon)
            {
                return false;
            }

            double f = 1.0 / a;
            glm::dvec3 s = origin - v0;
            double u = f * glm::dot(s, h);

            if (u < 0.0 || u > 1.0)
            {
                return false;
            }

            glm::dvec3 q = glm::cross(s, edge1);
            double v = f * glm::dot(direction, q);

            if (v < 0.0 || u + v > 1.0)
            {
                return false;
            }

            double t = f * glm::dot(edge2, q);
            if (t > epsilon)
            {
                outT = t;
                return true;
            }
            return false;
        }

        //! Read a feature ID from a glTF accessor for a given vertex index.
        AZ::s64 ReadFeatureIdFromAccessor(
            const CesiumGltf::Model& model,
            int32_t accessorIndex,
            int64_t vertexIndex)
        {
            if (accessorIndex < 0 || static_cast<size_t>(accessorIndex) >= model.accessors.size())
            {
                return -1;
            }

            const auto& accessor = model.accessors[accessorIndex];

            // Try common integer types for feature IDs
            if (accessor.componentType == CesiumGltf::Accessor::ComponentType::UNSIGNED_SHORT)
            {
                CesiumGltf::AccessorView<uint16_t> view(model, accessor);
                if (view.status() == CesiumGltf::AccessorViewStatus::Valid && vertexIndex < view.size())
                {
                    return static_cast<AZ::s64>(view[vertexIndex]);
                }
            }
            else if (accessor.componentType == CesiumGltf::Accessor::ComponentType::UNSIGNED_INT)
            {
                CesiumGltf::AccessorView<uint32_t> view(model, accessor);
                if (view.status() == CesiumGltf::AccessorViewStatus::Valid && vertexIndex < view.size())
                {
                    return static_cast<AZ::s64>(view[vertexIndex]);
                }
            }
            else if (accessor.componentType == CesiumGltf::Accessor::ComponentType::UNSIGNED_BYTE)
            {
                CesiumGltf::AccessorView<uint8_t> view(model, accessor);
                if (view.status() == CesiumGltf::AccessorViewStatus::Valid && vertexIndex < view.size())
                {
                    return static_cast<AZ::s64>(view[vertexIndex]);
                }
            }
            else if (accessor.componentType == CesiumGltf::Accessor::ComponentType::FLOAT)
            {
                CesiumGltf::AccessorView<float> view(model, accessor);
                if (view.status() == CesiumGltf::AccessorViewStatus::Valid && vertexIndex < view.size())
                {
                    return static_cast<AZ::s64>(view[vertexIndex]);
                }
            }

            return -1;
        }

        //! Get vertex indices for a triangle from the glTF index buffer.
        bool GetTriangleVertexIndices(
            const CesiumGltf::Model& model,
            const CesiumGltf::MeshPrimitive& primitive,
            int64_t triangleIndex,
            int64_t& v0,
            int64_t& v1,
            int64_t& v2)
        {
            if (primitive.indices >= 0)
            {
                const auto& accessor = model.accessors[primitive.indices];
                int64_t baseIndex = triangleIndex * 3;

                if (accessor.componentType == CesiumGltf::Accessor::ComponentType::UNSIGNED_SHORT)
                {
                    CesiumGltf::AccessorView<uint16_t> indexView(model, accessor);
                    if (indexView.status() == CesiumGltf::AccessorViewStatus::Valid && baseIndex + 2 < indexView.size())
                    {
                        v0 = indexView[baseIndex];
                        v1 = indexView[baseIndex + 1];
                        v2 = indexView[baseIndex + 2];
                        return true;
                    }
                }
                else if (accessor.componentType == CesiumGltf::Accessor::ComponentType::UNSIGNED_INT)
                {
                    CesiumGltf::AccessorView<uint32_t> indexView(model, accessor);
                    if (indexView.status() == CesiumGltf::AccessorViewStatus::Valid && baseIndex + 2 < indexView.size())
                    {
                        v0 = indexView[baseIndex];
                        v1 = indexView[baseIndex + 1];
                        v2 = indexView[baseIndex + 2];
                        return true;
                    }
                }
                else if (accessor.componentType == CesiumGltf::Accessor::ComponentType::UNSIGNED_BYTE)
                {
                    CesiumGltf::AccessorView<uint8_t> indexView(model, accessor);
                    if (indexView.status() == CesiumGltf::AccessorViewStatus::Valid && baseIndex + 2 < indexView.size())
                    {
                        v0 = indexView[baseIndex];
                        v1 = indexView[baseIndex + 1];
                        v2 = indexView[baseIndex + 2];
                        return true;
                    }
                }
            }
            else
            {
                // Non-indexed geometry
                v0 = triangleIndex * 3;
                v1 = v0 + 1;
                v2 = v0 + 2;
                return true;
            }

            return false;
        }
    } // anonymous namespace

    // ============================================================
    // CesiumMetadataComponent::Impl
    // ============================================================
    struct CesiumMetadataComponent::Impl
    {
        //! Cached metadata for a loaded tile.
        struct TileMetadataCache
        {
            const CesiumGltf::Model* m_sourceModel = nullptr;
            AZStd::unordered_map<AZStd::string, CesiumPropertyTable> m_propertyTables;
            AZStd::vector<CesiumPrimitiveFeatureIds> m_primitiveFeatureIds;
        };

        //! All currently loaded tiles' metadata, keyed by render resource pointer.
        AZStd::unordered_map<const void*, TileMetadataCache> m_tileMetadataCache;

        //! Build property table cache from a CesiumGltf::Model.
        static AZStd::unordered_map<AZStd::string, CesiumPropertyTable> BuildPropertyTables(const CesiumGltf::Model* model)
        {
            AZStd::unordered_map<AZStd::string, CesiumPropertyTable> tables;
            if (!model)
            {
                return tables;
            }

            const auto* metadataExt = model->getExtension<CesiumGltf::ExtensionModelExtFeatureMetadata>();
            if (!metadataExt)
            {
                return tables;
            }

            for (const auto& [name, featureTable] : metadataExt->featureTables)
            {
                AZStd::string azName(name.c_str());
                tables.emplace(azName, CesiumPropertyTable(azName, model, &featureTable));
            }

            return tables;
        }

        //! Extract per-primitive feature ID sets from a CesiumGltf::Model.
        static AZStd::vector<CesiumPrimitiveFeatureIds> ExtractPrimitiveFeatureIds(const CesiumGltf::Model* model)
        {
            AZStd::vector<CesiumPrimitiveFeatureIds> allPrimFeatureIds;
            if (!model)
            {
                return allPrimFeatureIds;
            }

            const auto* metadataExt = model->getExtension<CesiumGltf::ExtensionModelExtFeatureMetadata>();

            for (int32_t meshIdx = 0; meshIdx < static_cast<int32_t>(model->meshes.size()); ++meshIdx)
            {
                const auto& mesh = model->meshes[meshIdx];
                for (int32_t primIdx = 0; primIdx < static_cast<int32_t>(mesh.primitives.size()); ++primIdx)
                {
                    const auto& primitive = mesh.primitives[primIdx];
                    CesiumPrimitiveFeatureIds primFeatureIds;
                    primFeatureIds.m_gltfMeshIndex = meshIdx;
                    primFeatureIds.m_gltfPrimitiveIndex = primIdx;

                    const auto* primExt =
                        primitive.getExtension<CesiumGltf::ExtensionMeshPrimitiveExtFeatureMetadata>();
                    if (primExt)
                    {
                        // Process feature ID attributes
                        for (const auto& featureIdAttr : primExt->featureIdAttributes)
                        {
                            CesiumFeatureIdSet featureIdSet;
                            featureIdSet.m_type = CesiumFeatureIdSetType::Attribute;
                            featureIdSet.m_featureTableName = AZStd::string(featureIdAttr.featureTable.c_str());

                            if (featureIdAttr.featureIds.attribute)
                            {
                                featureIdSet.m_attributeName =
                                    AZStd::string(featureIdAttr.featureIds.attribute->c_str());

                                // Find the accessor index for this attribute
                                auto attrIt = primitive.attributes.find(*featureIdAttr.featureIds.attribute);
                                if (attrIt != primitive.attributes.end())
                                {
                                    featureIdSet.m_accessorIndex = attrIt->second;
                                }
                            }

                            // Get feature count from the feature table
                            if (metadataExt)
                            {
                                auto tableIt = metadataExt->featureTables.find(featureIdAttr.featureTable);
                                if (tableIt != metadataExt->featureTables.end())
                                {
                                    featureIdSet.m_featureCount = tableIt->second.count;
                                }
                            }

                            primFeatureIds.m_featureIdSets.push_back(AZStd::move(featureIdSet));
                        }

                        // Process feature ID textures
                        for (const auto& featureIdTex : primExt->featureIdTextures)
                        {
                            CesiumFeatureIdSet featureIdSet;
                            featureIdSet.m_type = CesiumFeatureIdSetType::Texture;
                            featureIdSet.m_featureTableName = AZStd::string(featureIdTex.featureTable.c_str());

                            if (metadataExt)
                            {
                                auto tableIt = metadataExt->featureTables.find(featureIdTex.featureTable);
                                if (tableIt != metadataExt->featureTables.end())
                                {
                                    featureIdSet.m_featureCount = tableIt->second.count;
                                }
                            }

                            primFeatureIds.m_featureIdSets.push_back(AZStd::move(featureIdSet));
                        }
                    }

                    allPrimFeatureIds.push_back(AZStd::move(primFeatureIds));
                }
            }

            return allPrimFeatureIds;
        }

        //! Find a property table by name across all cached tiles.
        const CesiumPropertyTable* FindPropertyTable(const AZStd::string& name) const
        {
            for (const auto& [_, cache] : m_tileMetadataCache)
            {
                auto it = cache.m_propertyTables.find(name);
                if (it != cache.m_propertyTables.end())
                {
                    return &it->second;
                }
            }
            return nullptr;
        }

        //! Perform software ray cast against all loaded tile geometry.
        //! Returns the feature ID of the closest hit, and the feature table name.
        struct RaycastResult
        {
            AZ::s64 m_featureId = -1;
            AZStd::string m_featureTableName;
            double m_hitDistance = std::numeric_limits<double>::max();
        };

        RaycastResult PerformRaycast(
            const glm::dvec3& origin,
            const glm::dvec3& direction,
            double maxDistance,
            AZ::s32 featureIdSetIndex) const
        {
            RaycastResult bestResult;
            glm::dvec3 normDir = glm::normalize(direction);

            for (const auto& [renderPtr, cache] : m_tileMetadataCache)
            {
                if (!cache.m_sourceModel)
                {
                    continue;
                }

                const CesiumGltf::Model& model = *cache.m_sourceModel;
                size_t primFlatIdx = 0;

                for (size_t meshIdx = 0; meshIdx < model.meshes.size(); ++meshIdx)
                {
                    const auto& mesh = model.meshes[meshIdx];
                    for (size_t primIdx = 0; primIdx < mesh.primitives.size(); ++primIdx, ++primFlatIdx)
                    {
                        const auto& primitive = mesh.primitives[primIdx];
                        if (primitive.mode != CesiumGltf::MeshPrimitive::Mode::TRIANGLES)
                        {
                            continue;
                        }

                        // Get position accessor
                        auto posIt = primitive.attributes.find("POSITION");
                        if (posIt == primitive.attributes.end())
                        {
                            continue;
                        }

                        CesiumGltf::AccessorView<glm::vec3> posView(model, posIt->second);
                        if (posView.status() != CesiumGltf::AccessorViewStatus::Valid)
                        {
                            continue;
                        }

                        // Determine triangle count
                        int64_t triangleCount = 0;
                        if (primitive.indices >= 0)
                        {
                            const auto& indexAccessor = model.accessors[primitive.indices];
                            triangleCount = indexAccessor.count / 3;
                        }
                        else
                        {
                            triangleCount = posView.size() / 3;
                        }

                        // Test each triangle
                        for (int64_t triIdx = 0; triIdx < triangleCount; ++triIdx)
                        {
                            int64_t i0, i1, i2;
                            if (!GetTriangleVertexIndices(model, primitive, triIdx, i0, i1, i2))
                            {
                                continue;
                            }

                            if (i0 >= posView.size() || i1 >= posView.size() || i2 >= posView.size())
                            {
                                continue;
                            }

                            glm::dvec3 v0(posView[i0]);
                            glm::dvec3 v1(posView[i1]);
                            glm::dvec3 v2(posView[i2]);

                            double t = 0.0;
                            if (RayTriangleIntersect(origin, normDir, v0, v1, v2, t) && t < maxDistance && t < bestResult.m_hitDistance)
                            {
                                // Found a closer hit - resolve feature ID
                                if (primFlatIdx < cache.m_primitiveFeatureIds.size())
                                {
                                    const auto& primFids = cache.m_primitiveFeatureIds[primFlatIdx];
                                    if (featureIdSetIndex >= 0 &&
                                        static_cast<size_t>(featureIdSetIndex) < primFids.m_featureIdSets.size())
                                    {
                                        const auto& fidSet = primFids.m_featureIdSets[featureIdSetIndex];
                                        if (fidSet.m_type == CesiumFeatureIdSetType::Attribute && fidSet.m_accessorIndex >= 0)
                                        {
                                            AZ::s64 featureId = ReadFeatureIdFromAccessor(model, fidSet.m_accessorIndex, i0);
                                            if (featureId >= 0)
                                            {
                                                bestResult.m_featureId = featureId;
                                                bestResult.m_featureTableName = fidSet.m_featureTableName;
                                                bestResult.m_hitDistance = t;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            return bestResult;
        }
    };

    // ============================================================
    // CesiumMetadataComponent
    // ============================================================
    void CesiumMetadataComponent::Reflect(AZ::ReflectContext* context)
    {
        CesiumMetadataValue::Reflect(context);
        CesiumPrimitiveFeatureIds::Reflect(context);
        CesiumPropertyTable::Reflect(context);
        CesiumMetadataRequest::Reflect(context);

        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<CesiumMetadataComponent, AZ::Component>()->Version(0);

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<CesiumMetadataComponent>("Cesium Metadata", "Enables 3D Tiles metadata queries on this tileset.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "Cesium")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"));
            }
        }
    }

    void CesiumMetadataComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("CesiumMetadataService"));
    }

    void CesiumMetadataComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("CesiumMetadataService"));
    }

    void CesiumMetadataComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        // No hard requirement — editor uses "3DTilesEditorService", runtime uses "3DTilesService".
        // Activation order is handled via GetDependentServices.
    }

    void CesiumMetadataComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        dependent.push_back(AZ_CRC_CE("3DTilesService"));
        dependent.push_back(AZ_CRC_CE("3DTilesEditorService"));
    }

    CesiumMetadataComponent::CesiumMetadataComponent() = default;

    CesiumMetadataComponent::~CesiumMetadataComponent() noexcept = default;

    void CesiumMetadataComponent::Init()
    {
        m_impl = AZStd::make_unique<Impl>();
    }

    void CesiumMetadataComponent::Activate()
    {
        CesiumMetadataRequestBus::Handler::BusConnect(GetEntityId());
        AZ::TickBus::Handler::BusConnect();
    }

    void CesiumMetadataComponent::Deactivate()
    {
        AZ::TickBus::Handler::BusDisconnect();
        CesiumMetadataRequestBus::Handler::BusDisconnect();
        m_impl->m_tileMetadataCache.clear();
    }

    void CesiumMetadataComponent::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        // Refresh metadata cache from currently loaded tiles
        AZStd::vector<TilesetMetadataAccessRequest::TileMetadataEntry> entries;
        TilesetMetadataAccessBus::EventResult(entries, GetEntityId(), &TilesetMetadataAccessBus::Events::GetLoadedTilesWithMetadata);

        // Build new cache (keys = render resource pointers)
        AZStd::unordered_map<const void*, Impl::TileMetadataCache> newCache;
        for (const auto& entry : entries)
        {
            if (!entry.m_sourceModel || !entry.m_renderModel)
            {
                continue;
            }

            const void* key = entry.m_renderModel;

            // Reuse existing cache entry if the source model hasn't changed
            auto existingIt = m_impl->m_tileMetadataCache.find(key);
            if (existingIt != m_impl->m_tileMetadataCache.end() && existingIt->second.m_sourceModel == entry.m_sourceModel)
            {
                newCache.emplace(key, AZStd::move(existingIt->second));
            }
            else
            {
                Impl::TileMetadataCache cache;
                cache.m_sourceModel = entry.m_sourceModel;
                cache.m_propertyTables = Impl::BuildPropertyTables(entry.m_sourceModel);
                cache.m_primitiveFeatureIds = Impl::ExtractPrimitiveFeatureIds(entry.m_sourceModel);
                newCache.emplace(key, AZStd::move(cache));
            }
        }

        m_impl->m_tileMetadataCache = AZStd::move(newCache);
    }

    AZStd::vector<AZStd::string> CesiumMetadataComponent::GetFeatureTableNames() const
    {
        AZStd::vector<AZStd::string> names;
        AZStd::unordered_set<AZStd::string> seen;

        for (const auto& [_, cache] : m_impl->m_tileMetadataCache)
        {
            for (const auto& [tableName, _table] : cache.m_propertyTables)
            {
                if (seen.insert(tableName).second)
                {
                    names.push_back(tableName);
                }
            }
        }
        return names;
    }

    AZ::s64 CesiumMetadataComponent::GetFeatureCount(const AZStd::string& featureTableName) const
    {
        const CesiumPropertyTable* table = m_impl->FindPropertyTable(featureTableName);
        return table ? table->GetFeatureCount() : -1;
    }

    AZStd::vector<CesiumPropertyDescription> CesiumMetadataComponent::GetPropertyDescriptions(
        const AZStd::string& featureTableName) const
    {
        const CesiumPropertyTable* table = m_impl->FindPropertyTable(featureTableName);
        return table ? table->GetPropertyDescriptions() : AZStd::vector<CesiumPropertyDescription>{};
    }

    AZStd::vector<AZStd::string> CesiumMetadataComponent::GetPropertyNames(const AZStd::string& featureTableName) const
    {
        const CesiumPropertyTable* table = m_impl->FindPropertyTable(featureTableName);
        return table ? table->GetPropertyNames() : AZStd::vector<AZStd::string>{};
    }

    CesiumMetadataValue CesiumMetadataComponent::GetPropertyValue(
        const AZStd::string& featureTableName,
        const AZStd::string& propertyName,
        AZ::s64 featureId) const
    {
        const CesiumPropertyTable* table = m_impl->FindPropertyTable(featureTableName);
        return table ? table->GetPropertyValue(propertyName, featureId) : CesiumMetadataValue{};
    }

    AZStd::unordered_map<AZStd::string, CesiumMetadataValue> CesiumMetadataComponent::GetAllPropertiesForFeature(
        const AZStd::string& featureTableName,
        AZ::s64 featureId) const
    {
        const CesiumPropertyTable* table = m_impl->FindPropertyTable(featureTableName);
        return table ? table->GetAllPropertiesForFeature(featureId) : AZStd::unordered_map<AZStd::string, CesiumMetadataValue>{};
    }

    AZ::s64 CesiumMetadataComponent::GetFeatureIdFromRaycast(
        const glm::dvec3& origin,
        const glm::dvec3& direction,
        double maxDistance,
        AZ::s32 featureIdSetIndex) const
    {
        Impl::RaycastResult result = m_impl->PerformRaycast(origin, direction, maxDistance, featureIdSetIndex);
        return result.m_featureId;
    }

    AZStd::unordered_map<AZStd::string, CesiumMetadataValue> CesiumMetadataComponent::GetPropertiesFromRaycast(
        const glm::dvec3& origin,
        const glm::dvec3& direction,
        double maxDistance,
        AZ::s32 featureIdSetIndex) const
    {
        Impl::RaycastResult result = m_impl->PerformRaycast(origin, direction, maxDistance, featureIdSetIndex);
        if (result.m_featureId >= 0 && !result.m_featureTableName.empty())
        {
            const CesiumPropertyTable* table = m_impl->FindPropertyTable(result.m_featureTableName);
            if (table)
            {
                return table->GetAllPropertiesForFeature(result.m_featureId);
            }
        }
        return {};
    }
} // namespace Cesium
