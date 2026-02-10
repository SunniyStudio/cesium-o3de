#include <Cesium/Components/CesiumTilesetPhysicsComponent.h>
#include "Cesium/EBus/TilesetMetadataAccessBus.h"
#include "Cesium/TilesetUtility/RenderResourcesPreparer.h"
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/std/containers/unordered_map.h>
#include <AzCore/std/containers/unordered_set.h>
#include <AzCore/Interface/Interface.h>

#include <AzFramework/Physics/PhysicsScene.h>
#include <AzFramework/Physics/PhysicsSystem.h>
#include <AzFramework/Physics/SystemBus.h>
#include <AzFramework/Physics/Configuration/StaticRigidBodyConfiguration.h>
#include <AzFramework/Physics/ShapeConfiguration.h>
#include <AzFramework/Physics/Shape.h>
#include <AzFramework/Physics/Common/PhysicsTypes.h>
#include <AzFramework/Physics/CollisionBus.h>

// Windows wingdi.h OPAQUE macro workaround
#include <AzCore/PlatformDef.h>
#ifdef AZ_COMPILER_MSVC
#pragma push_macro("OPAQUE")
#undef OPAQUE
#endif

#include <CesiumGltf/Model.h>
#include <CesiumGltf/MeshPrimitive.h>
#include <CesiumGltf/AccessorView.h>

#ifdef AZ_COMPILER_MSVC
#pragma pop_macro("OPAQUE")
#endif

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Cesium
{
    // GLTF to O3DE coordinate system conversion (same as GltfModelBuilder)
    static constexpr glm::dmat4 GLTF_TO_O3DE =
        glm::dmat4(1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0);

    // ============================================================
    // Helper: Extract vertices and indices from a CesiumGltf::Model
    // ============================================================
    namespace
    {
        struct ExtractedMeshData
        {
            AZStd::vector<AZ::Vector3> m_vertices;
            AZStd::vector<AZ::u32> m_indices;
        };

        //! Extract all triangle mesh data from a CesiumGltf::Model.
        //! Vertices are converted to O3DE coordinate space.
        ExtractedMeshData ExtractMeshData(const CesiumGltf::Model& model, bool doubleSided)
        {
            ExtractedMeshData result;

            for (const auto& mesh : model.meshes)
            {
                for (const auto& primitive : mesh.primitives)
                {
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
                    if (posView.status() != CesiumGltf::AccessorViewStatus::Valid || posView.size() == 0)
                    {
                        continue;
                    }

                    // Add vertices (converting GLTF -> O3DE coordinate system)
                    AZ::u32 baseVertex = static_cast<AZ::u32>(result.m_vertices.size());
                    result.m_vertices.reserve(result.m_vertices.size() + posView.size());

                    for (int64_t i = 0; i < posView.size(); ++i)
                    {
                        const glm::vec3& gltfPos = posView[i];
                        // Apply GLTF_TO_O3DE: O3DE(x, y, z) = GLTF(x, -z, y)
                        result.m_vertices.emplace_back(
                            static_cast<float>(gltfPos.x),
                            static_cast<float>(-gltfPos.z),
                            static_cast<float>(gltfPos.y));
                    }

                    // Add indices
                    if (primitive.indices >= 0)
                    {
                        const auto& accessor = model.accessors[primitive.indices];

                        if (accessor.componentType == CesiumGltf::Accessor::ComponentType::UNSIGNED_SHORT)
                        {
                            CesiumGltf::AccessorView<uint16_t> indexView(model, accessor);
                            if (indexView.status() == CesiumGltf::AccessorViewStatus::Valid)
                            {
                                for (int64_t i = 0; i < indexView.size(); ++i)
                                {
                                    result.m_indices.push_back(baseVertex + static_cast<AZ::u32>(indexView[i]));
                                }
                            }
                        }
                        else if (accessor.componentType == CesiumGltf::Accessor::ComponentType::UNSIGNED_INT)
                        {
                            CesiumGltf::AccessorView<uint32_t> indexView(model, accessor);
                            if (indexView.status() == CesiumGltf::AccessorViewStatus::Valid)
                            {
                                for (int64_t i = 0; i < indexView.size(); ++i)
                                {
                                    result.m_indices.push_back(baseVertex + static_cast<AZ::u32>(indexView[i]));
                                }
                            }
                        }
                        else if (accessor.componentType == CesiumGltf::Accessor::ComponentType::UNSIGNED_BYTE)
                        {
                            CesiumGltf::AccessorView<uint8_t> indexView(model, accessor);
                            if (indexView.status() == CesiumGltf::AccessorViewStatus::Valid)
                            {
                                for (int64_t i = 0; i < indexView.size(); ++i)
                                {
                                    result.m_indices.push_back(baseVertex + static_cast<AZ::u32>(indexView[i]));
                                }
                            }
                        }
                    }
                    else
                    {
                        // Non-indexed: generate sequential indices
                        for (int64_t i = 0; i < posView.size(); ++i)
                        {
                            result.m_indices.push_back(baseVertex + static_cast<AZ::u32>(i));
                        }
                    }
                }
            }

            // Add reverse-winding triangles for double-sided collision
            if (doubleSided && !result.m_indices.empty())
            {
                size_t originalIndexCount = result.m_indices.size();
                result.m_indices.reserve(originalIndexCount * 2);
                for (size_t i = 0; i < originalIndexCount; i += 3)
                {
                    if (i + 2 < originalIndexCount)
                    {
                        result.m_indices.push_back(result.m_indices[i]);
                        result.m_indices.push_back(result.m_indices[i + 2]);
                        result.m_indices.push_back(result.m_indices[i + 1]);
                    }
                }
            }

            return result;
        }

        //! Convert glm::dmat4 to AZ::Transform + AZ::Vector3 scale.
        void DecomposeGlmTransform(
            const glm::dmat4& mat4,
            AZ::Transform& outTransform,
            AZ::Vector3& outScale)
        {
            const glm::dvec4& translation = mat4[3];
            glm::dvec3 scale;
            for (uint32_t i = 0; i < 3; ++i)
            {
                scale[i] = glm::length(mat4[i]);
            }

            constexpr double scaleEpsilon = 1e-10;
            const glm::dmat3 rotMtx(
                scale[0] > scaleEpsilon ? glm::dvec3(mat4[0]) / scale[0] : glm::dvec3(1.0, 0.0, 0.0),
                scale[1] > scaleEpsilon ? glm::dvec3(mat4[1]) / scale[1] : glm::dvec3(0.0, 1.0, 0.0),
                scale[2] > scaleEpsilon ? glm::dvec3(mat4[2]) / scale[2] : glm::dvec3(0.0, 0.0, 1.0));

            glm::dquat q = glm::quat_cast(rotMtx);
            AZ::Quaternion azQuat(
                static_cast<float>(q.x), static_cast<float>(q.y),
                static_cast<float>(q.z), static_cast<float>(q.w));
            AZ::Vector3 azTranslation(
                static_cast<float>(translation.x), static_cast<float>(translation.y),
                static_cast<float>(translation.z));
            outScale = AZ::Vector3(
                static_cast<float>(scale.x), static_cast<float>(scale.y),
                static_cast<float>(scale.z));
            outTransform = AZ::Transform::CreateFromQuaternionAndTranslation(azQuat, azTranslation);
        }
    } // anonymous namespace

    // ============================================================
    // CesiumTilesetPhysicsComponent::Impl
    // ============================================================
    struct CesiumTilesetPhysicsComponent::Impl
    {
        //! Physics data for a single loaded tile.
        struct TilePhysicsData
        {
            AZStd::vector<AzPhysics::SimulatedBodyHandle> m_bodyHandles;
            const CesiumGltf::Model* m_sourceModel = nullptr;
        };

        //! Map from tile render resource pointer to physics data.
        AZStd::unordered_map<const void*, TilePhysicsData> m_tilePhysicsMap;

        //! Tiles pending cooking (queued for next frames).
        struct PendingTile
        {
            const void* m_key = nullptr;
            const CesiumGltf::Model* m_sourceModel = nullptr;
            IntrusiveGltfModel* m_renderModel = nullptr;
        };
        AZStd::vector<PendingTile> m_pendingCooks;

        //! Current tile world transform (updated on origin shift).
        glm::dmat4 m_currentTileTransform{ 1.0 };

        //! Physics scene handle.
        AzPhysics::SceneHandle m_sceneHandle = AzPhysics::InvalidSceneHandle;

        //! Get the physics scene interface.
        AzPhysics::SceneInterface* GetSceneInterface()
        {
            return AZ::Interface<AzPhysics::SceneInterface>::Get();
        }

        //! Ensure we have a valid scene handle.
        bool EnsureSceneHandle()
        {
            if (m_sceneHandle != AzPhysics::InvalidSceneHandle)
            {
                return true;
            }

            auto* sceneInterface = GetSceneInterface();
            if (!sceneInterface)
            {
                return false;
            }

            m_sceneHandle = sceneInterface->GetSceneHandle(AzPhysics::DefaultPhysicsSceneName);
            return m_sceneHandle != AzPhysics::InvalidSceneHandle;
        }

        //! Cook and create physics body for a tile.
        bool CreatePhysicsForTile(
            const void* key,
            const CesiumGltf::Model* sourceModel,
            const glm::dmat4& worldTransform,
            const TilesetPhysicsConfiguration& config,
            AZ::EntityId entityId)
        {
            if (!sourceModel || !EnsureSceneHandle())
            {
                return false;
            }

            // Extract mesh data
            ExtractedMeshData meshData = ExtractMeshData(*sourceModel, config.m_doubleSided);
            if (meshData.m_vertices.empty() || meshData.m_indices.empty())
            {
                return false;
            }

            // Cook triangle mesh
            AZStd::vector<AZ::u8> cookedData;
            bool cookResult = false;
            Physics::SystemRequestBus::BroadcastResult(
                cookResult,
                &Physics::SystemRequests::CookTriangleMeshToMemory,
                meshData.m_vertices.data(),
                static_cast<AZ::u32>(meshData.m_vertices.size()),
                meshData.m_indices.data(),
                static_cast<AZ::u32>(meshData.m_indices.size()),
                cookedData);

            if (!cookResult || cookedData.empty())
            {
                AZ_Warning("CesiumPhysics", false, "Failed to cook triangle mesh for tile");
                return false;
            }

            // Create shape configuration
            auto shapeConfig = AZStd::make_shared<Physics::CookedMeshShapeConfiguration>();
            shapeConfig->SetCookedMeshData(
                cookedData.data(),
                cookedData.size(),
                Physics::CookedMeshShapeConfiguration::MeshType::TriangleMesh);

            // Create collider configuration
            auto colliderConfig = AZStd::make_shared<Physics::ColliderConfiguration>();

            // Set collision layer by name lookup
            if (!config.m_collisionLayerName.empty())
            {
                AzPhysics::CollisionLayer layer;
                bool found = false;
                Physics::CollisionRequestBus::BroadcastResult(
                    found,
                    &Physics::CollisionRequests::TryGetCollisionLayerByName,
                    config.m_collisionLayerName,
                    layer);
                if (found)
                {
                    colliderConfig->m_collisionLayer = layer;
                }
            }

            // Note: CollisionGroups::Id is an editor-time UUID reference.
            // Custom collision group assignment should be done via the editor property panel
            // or by setting m_collisionGroupId directly in code.

            // Decompose world transform for body position
            AZ::Transform bodyTransform;
            AZ::Vector3 bodyScale;
            DecomposeGlmTransform(worldTransform, bodyTransform, bodyScale);

            // Create static rigid body configuration
            AzPhysics::StaticRigidBodyConfiguration staticBodyConfig;
            staticBodyConfig.m_position = bodyTransform.GetTranslation();
            staticBodyConfig.m_orientation = bodyTransform.GetRotation();
            staticBodyConfig.m_debugName = "CesiumTile";
            staticBodyConfig.m_entityId = entityId;
            staticBodyConfig.m_colliderAndShapeData = AzPhysics::ShapeColliderPair(colliderConfig, shapeConfig);

            // Add to physics scene
            auto* sceneInterface = GetSceneInterface();
            AzPhysics::SimulatedBodyHandle bodyHandle =
                sceneInterface->AddSimulatedBody(m_sceneHandle, &staticBodyConfig);

            if (bodyHandle == AzPhysics::InvalidSimulatedBodyHandle)
            {
                AZ_Warning("CesiumPhysics", false, "Failed to add physics body to scene");
                return false;
            }

            // Store the handle
            TilePhysicsData& data = m_tilePhysicsMap[key];
            data.m_sourceModel = sourceModel;
            data.m_bodyHandles.push_back(bodyHandle);

            return true;
        }

        //! Remove physics bodies for a tile.
        void RemovePhysicsForTile(const void* key)
        {
            auto it = m_tilePhysicsMap.find(key);
            if (it == m_tilePhysicsMap.end())
            {
                return;
            }

            auto* sceneInterface = GetSceneInterface();
            if (sceneInterface && m_sceneHandle != AzPhysics::InvalidSceneHandle)
            {
                for (auto& handle : it->second.m_bodyHandles)
                {
                    if (handle != AzPhysics::InvalidSimulatedBodyHandle)
                    {
                        sceneInterface->RemoveSimulatedBody(m_sceneHandle, handle);
                    }
                }
            }

            m_tilePhysicsMap.erase(it);
        }

        //! Remove all physics bodies.
        void RemoveAllPhysics()
        {
            auto* sceneInterface = GetSceneInterface();
            if (sceneInterface && m_sceneHandle != AzPhysics::InvalidSceneHandle)
            {
                for (auto& [_, data] : m_tilePhysicsMap)
                {
                    for (auto& handle : data.m_bodyHandles)
                    {
                        if (handle != AzPhysics::InvalidSimulatedBodyHandle)
                        {
                            sceneInterface->RemoveSimulatedBody(m_sceneHandle, handle);
                        }
                    }
                }
            }
            m_tilePhysicsMap.clear();
            m_pendingCooks.clear();
        }

        //! Update all physics body transforms (e.g., after origin shift).
        void UpdateAllBodyTransforms(const glm::dmat4& newTransform)
        {
            m_currentTileTransform = newTransform;

            AZ::Transform bodyTransform;
            AZ::Vector3 bodyScale;
            DecomposeGlmTransform(newTransform, bodyTransform, bodyScale);

            auto* sceneInterface = GetSceneInterface();
            if (!sceneInterface || m_sceneHandle == AzPhysics::InvalidSceneHandle)
            {
                return;
            }

            for (auto& [_, data] : m_tilePhysicsMap)
            {
                for (auto& handle : data.m_bodyHandles)
                {
                    if (handle != AzPhysics::InvalidSimulatedBodyHandle)
                    {
                        auto* body = sceneInterface->GetSimulatedBodyFromHandle(m_sceneHandle, handle);
                        if (body)
                        {
                            body->SetTransform(bodyTransform);
                        }
                    }
                }
            }
        }

        AZ::u32 GetTotalBodyCount() const
        {
            AZ::u32 count = 0;
            for (const auto& [_, data] : m_tilePhysicsMap)
            {
                count += static_cast<AZ::u32>(data.m_bodyHandles.size());
            }
            return count;
        }
    };

    // ============================================================
    // CesiumTilesetPhysicsComponent
    // ============================================================
    void CesiumTilesetPhysicsComponent::Reflect(AZ::ReflectContext* context)
    {
        TilesetPhysicsConfiguration::Reflect(context);
        TilesetPhysicsRequest::Reflect(context);

        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<CesiumTilesetPhysicsComponent, AZ::Component>()
                ->Version(0)
                ->Field("Configuration", &CesiumTilesetPhysicsComponent::m_configuration);

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext
                    ->Class<CesiumTilesetPhysicsComponent>(
                        "Cesium Tileset Physics",
                        "Generates PhysX collision bodies for loaded 3D Tiles.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "Cesium")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default, &CesiumTilesetPhysicsComponent::m_configuration,
                        "Configuration", "Physics collision configuration");
            }
        }
    }

    void CesiumTilesetPhysicsComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("CesiumTilesetPhysicsService"));
    }

    void CesiumTilesetPhysicsComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("CesiumTilesetPhysicsService"));
    }

    void CesiumTilesetPhysicsComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        // No hard requirement — the editor uses "3DTilesEditorService" while runtime uses "3DTilesService".
        // Activation order is handled via GetDependentServices instead.
    }

    void CesiumTilesetPhysicsComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        // Ensure this component activates after the tileset component (editor or runtime)
        dependent.push_back(AZ_CRC_CE("3DTilesService"));
        dependent.push_back(AZ_CRC_CE("3DTilesEditorService"));
    }

    CesiumTilesetPhysicsComponent::CesiumTilesetPhysicsComponent() = default;

    CesiumTilesetPhysicsComponent::~CesiumTilesetPhysicsComponent() noexcept = default;

    void CesiumTilesetPhysicsComponent::Init()
    {
        m_impl = AZStd::make_unique<Impl>();
    }

    void CesiumTilesetPhysicsComponent::Activate()
    {
        TilesetPhysicsRequestBus::Handler::BusConnect(GetEntityId());
        AZ::TickBus::Handler::BusConnect();
        OriginShiftNotificationBus::Handler::BusConnect();
    }

    void CesiumTilesetPhysicsComponent::Deactivate()
    {
        OriginShiftNotificationBus::Handler::BusDisconnect();
        AZ::TickBus::Handler::BusDisconnect();
        TilesetPhysicsRequestBus::Handler::BusDisconnect();
        m_impl->RemoveAllPhysics();
    }

    void CesiumTilesetPhysicsComponent::SetConfiguration(const TilesetPhysicsConfiguration& configuration)
    {
        bool wasEnabled = m_configuration.m_enabled;
        m_configuration = configuration;

        // If disabled, remove all physics
        if (wasEnabled && !m_configuration.m_enabled)
        {
            m_impl->RemoveAllPhysics();
        }
    }

    const TilesetPhysicsConfiguration& CesiumTilesetPhysicsComponent::GetConfiguration() const
    {
        return m_configuration;
    }

    void CesiumTilesetPhysicsComponent::SetEnabled(bool enabled)
    {
        if (m_configuration.m_enabled == enabled)
        {
            return;
        }

        m_configuration.m_enabled = enabled;
        if (!enabled)
        {
            m_impl->RemoveAllPhysics();
        }
    }

    bool CesiumTilesetPhysicsComponent::GetEnabled() const
    {
        return m_configuration.m_enabled;
    }

    AZ::u32 CesiumTilesetPhysicsComponent::GetActiveBodyCount() const
    {
        return m_impl->GetTotalBodyCount();
    }

    void CesiumTilesetPhysicsComponent::OnTick(
        [[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        if (!m_configuration.m_enabled)
        {
            return;
        }

        // Get currently loaded tiles from TilesetComponent
        AZStd::vector<TilesetMetadataAccessRequest::TileMetadataEntry> entries;
        TilesetMetadataAccessBus::EventResult(
            entries, GetEntityId(), &TilesetMetadataAccessBus::Events::GetLoadedTilesWithMetadata);

        // Build set of current tile keys
        AZStd::unordered_set<const void*> currentTileKeys;
        for (const auto& entry : entries)
        {
            if (entry.m_renderModel)
            {
                currentTileKeys.insert(entry.m_renderModel);
            }
        }

        // Remove physics for tiles that are no longer loaded
        AZStd::vector<const void*> keysToRemove;
        for (const auto& [key, _] : m_impl->m_tilePhysicsMap)
        {
            if (currentTileKeys.find(key) == currentTileKeys.end())
            {
                keysToRemove.push_back(key);
            }
        }
        for (const void* key : keysToRemove)
        {
            m_impl->RemovePhysicsForTile(key);
        }

        // Queue new tiles for cooking
        for (const auto& entry : entries)
        {
            if (!entry.m_renderModel || !entry.m_sourceModel)
            {
                continue;
            }

            const void* key = entry.m_renderModel;
            if (m_impl->m_tilePhysicsMap.find(key) == m_impl->m_tilePhysicsMap.end())
            {
                // Check if already in pending queue
                bool alreadyPending = false;
                for (const auto& pending : m_impl->m_pendingCooks)
                {
                    if (pending.m_key == key)
                    {
                        alreadyPending = true;
                        break;
                    }
                }

                if (!alreadyPending)
                {
                    Impl::PendingTile pending;
                    pending.m_key = key;
                    pending.m_sourceModel = entry.m_sourceModel;
                    pending.m_renderModel = entry.m_renderModel;
                    m_impl->m_pendingCooks.push_back(pending);
                }
            }
        }

        // Process pending cooks (limited per frame to avoid hitches)
        AZ::u32 cooksThisFrame = 0;
        while (!m_impl->m_pendingCooks.empty())
        {
            if (m_configuration.m_maxCooksPerFrame > 0 && cooksThisFrame >= m_configuration.m_maxCooksPerFrame)
            {
                break;
            }

            auto pending = m_impl->m_pendingCooks.front();
            m_impl->m_pendingCooks.erase(m_impl->m_pendingCooks.begin());

            // Verify the tile is still loaded
            if (currentTileKeys.find(pending.m_key) == currentTileKeys.end())
            {
                continue;
            }

            // Get the tile's current rendered transform for body positioning
            glm::dmat4 tileWorldTransform = pending.m_renderModel->m_model.GetTransform();

            m_impl->CreatePhysicsForTile(
                pending.m_key,
                pending.m_sourceModel,
                tileWorldTransform,
                m_configuration,
                GetEntityId());

            ++cooksThisFrame;
        }
    }

    void CesiumTilesetPhysicsComponent::OnOriginShifting(const glm::dmat4& absToRelWorld)
    {
        m_impl->UpdateAllBodyTransforms(absToRelWorld);
    }
} // namespace Cesium
