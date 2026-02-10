#include <Cesium/Metadata/CesiumPropertyTable.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/Script/ScriptContextAttributes.h>

// Windows wingdi.h OPAQUE macro workaround
#include <AzCore/PlatformDef.h>
#ifdef AZ_COMPILER_MSVC
#pragma push_macro("OPAQUE")
#undef OPAQUE
#endif

#include <CesiumGltf/Model.h>
#include <CesiumGltf/ExtensionModelExtFeatureMetadata.h>
#include <CesiumGltf/MetadataFeatureTableView.h>
#include <CesiumGltf/MetadataPropertyView.h>

#ifdef AZ_COMPILER_MSVC
#pragma pop_macro("OPAQUE")
#endif

namespace Cesium
{
    void CesiumPropertyDescription::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<CesiumPropertyDescription>()
                ->Version(0)
                ->Field("Name", &CesiumPropertyDescription::m_name)
                ->Field("Type", &CesiumPropertyDescription::m_type)
                ->Field("IsArray", &CesiumPropertyDescription::m_isArray);
        }

        if (auto* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Class<CesiumPropertyDescription>("CesiumPropertyDescription")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/Metadata")
                ->Property("Name", BehaviorValueProperty(&CesiumPropertyDescription::m_name))
                ->Property("IsArray", BehaviorValueProperty(&CesiumPropertyDescription::m_isArray));
        }
    }

    void CesiumPropertyTable::Reflect(AZ::ReflectContext* context)
    {
        CesiumPropertyDescription::Reflect(context);

        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<CesiumPropertyTable>()->Version(0);
        }

        if (auto* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Class<CesiumPropertyTable>("CesiumPropertyTable")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/Metadata")
                ->Method("GetName", &CesiumPropertyTable::GetName)
                ->Method("GetFeatureCount", &CesiumPropertyTable::GetFeatureCount)
                ->Method("GetPropertyNames", &CesiumPropertyTable::GetPropertyNames)
                ->Method("IsValid", &CesiumPropertyTable::IsValid);
        }
    }

    CesiumPropertyTable::CesiumPropertyTable() = default;

    CesiumPropertyTable::CesiumPropertyTable(
        const AZStd::string& name,
        const CesiumGltf::Model* model,
        const CesiumGltf::FeatureTable* featureTable)
        : m_name{ name }
        , m_model{ model }
        , m_featureTable{ featureTable }
    {
        if (featureTable)
        {
            m_featureCount = featureTable->count;
        }
    }

    const AZStd::string& CesiumPropertyTable::GetName() const
    {
        return m_name;
    }

    AZ::s64 CesiumPropertyTable::GetFeatureCount() const
    {
        return m_featureCount;
    }

    bool CesiumPropertyTable::IsValid() const
    {
        return m_model != nullptr && m_featureTable != nullptr;
    }

    namespace
    {
        CesiumMetadataValueType ClassPropertyTypeToValueType(const CesiumGltf::ClassProperty& classProperty)
        {
            const auto& type = classProperty.type;
            if (type == "BOOLEAN")
            {
                return CesiumMetadataValueType::Boolean;
            }
            if (type == "STRING")
            {
                return CesiumMetadataValueType::String;
            }
            if (type == "ARRAY")
            {
                return CesiumMetadataValueType::None; // Arrays not yet supported in value type
            }

            // For scalar numeric types, the type field is the numeric type itself
            if (type == "INT8" || type == "UINT8" || type == "INT16" || type == "UINT16" ||
                type == "INT32" || type == "UINT32")
            {
                return CesiumMetadataValueType::Int32;
            }
            if (type == "INT64" || type == "UINT64")
            {
                return CesiumMetadataValueType::Int64;
            }
            if (type == "FLOAT32")
            {
                return CesiumMetadataValueType::Float;
            }
            if (type == "FLOAT64")
            {
                return CesiumMetadataValueType::Double;
            }

            return CesiumMetadataValueType::None;
        }
    } // namespace

    AZStd::vector<AZStd::string> CesiumPropertyTable::GetPropertyNames() const
    {
        AZStd::vector<AZStd::string> names;
        if (!IsValid())
        {
            return names;
        }

        CesiumGltf::MetadataFeatureTableView tableView(m_model, m_featureTable);
        tableView.forEachProperty(
            [&names]([[maybe_unused]] const std::string& propertyName, auto propertyView)
            {
                if (propertyView.status() == CesiumGltf::MetadataPropertyViewStatus::Valid)
                {
                    names.push_back(AZStd::string(propertyName.c_str()));
                }
            });

        return names;
    }

    AZStd::vector<CesiumPropertyDescription> CesiumPropertyTable::GetPropertyDescriptions() const
    {
        AZStd::vector<CesiumPropertyDescription> descriptions;
        if (!IsValid())
        {
            return descriptions;
        }

        // Get schema if available to extract type information
        const auto* metadataExt = m_model->getExtension<CesiumGltf::ExtensionModelExtFeatureMetadata>();
        if (metadataExt && metadataExt->schema)
        {
            // Find the class for this feature table
            const auto& classProperty = m_featureTable->classProperty;
            if (classProperty)
            {
                auto classIt = metadataExt->schema->classes.find(*classProperty);
                if (classIt != metadataExt->schema->classes.end())
                {
                    for (const auto& [propName, classProp] : classIt->second.properties)
                    {
                        CesiumPropertyDescription desc;
                        desc.m_name = AZStd::string(propName.c_str());
                        desc.m_type = ClassPropertyTypeToValueType(classProp);
                        desc.m_isArray = (classProp.type == "ARRAY");
                        descriptions.push_back(AZStd::move(desc));
                    }
                }
            }
        }

        // Fallback: enumerate via forEachProperty (type-dispatched callback)
        if (descriptions.empty())
        {
            CesiumGltf::MetadataFeatureTableView tableView(m_model, m_featureTable);
            tableView.forEachProperty(
                [&descriptions](const std::string& propertyName, auto propertyView)
                {
                    if (propertyView.status() == CesiumGltf::MetadataPropertyViewStatus::Valid)
                    {
                        CesiumPropertyDescription desc;
                        desc.m_name = AZStd::string(propertyName.c_str());
                        // Deduce element type from property view's get() return type
                        using ElementType = std::decay_t<decltype(propertyView.get(0))>;
                        if constexpr (std::is_same_v<ElementType, bool>)
                        {
                            desc.m_type = CesiumMetadataValueType::Boolean;
                        }
                        else if constexpr (std::is_same_v<ElementType, std::string_view>)
                        {
                            desc.m_type = CesiumMetadataValueType::String;
                        }
                        else if constexpr (std::is_integral_v<ElementType> && sizeof(ElementType) <= 4)
                        {
                            desc.m_type = CesiumMetadataValueType::Int32;
                        }
                        else if constexpr (std::is_integral_v<ElementType> && sizeof(ElementType) > 4)
                        {
                            desc.m_type = CesiumMetadataValueType::Int64;
                        }
                        else if constexpr (std::is_same_v<ElementType, float>)
                        {
                            desc.m_type = CesiumMetadataValueType::Float;
                        }
                        else if constexpr (std::is_same_v<ElementType, double>)
                        {
                            desc.m_type = CesiumMetadataValueType::Double;
                        }
                        descriptions.push_back(AZStd::move(desc));
                    }
                });
        }

        return descriptions;
    }

    CesiumMetadataValue CesiumPropertyTable::GetPropertyValue(const AZStd::string& propertyName, AZ::s64 featureId) const
    {
        if (!IsValid() || featureId < 0 || featureId >= m_featureCount)
        {
            return CesiumMetadataValue{};
        }

        CesiumGltf::MetadataFeatureTableView tableView(m_model, m_featureTable);
        CesiumMetadataValue result;

        tableView.getPropertyView(
            std::string(propertyName.c_str()),
            [&result, featureId](const std::string& /*name*/, auto propertyView)
            {
                if (propertyView.status() != CesiumGltf::MetadataPropertyViewStatus::Valid)
                {
                    return;
                }

                using ElementType = std::decay_t<decltype(propertyView.get(0))>;
                auto value = propertyView.get(featureId);

                if constexpr (std::is_same_v<ElementType, bool>)
                {
                    result = CesiumMetadataValue(static_cast<bool>(value));
                }
                else if constexpr (std::is_same_v<ElementType, int8_t>)
                {
                    result = CesiumMetadataValue(static_cast<AZ::s32>(value));
                }
                else if constexpr (std::is_same_v<ElementType, uint8_t>)
                {
                    result = CesiumMetadataValue(static_cast<AZ::s32>(value));
                }
                else if constexpr (std::is_same_v<ElementType, int16_t>)
                {
                    result = CesiumMetadataValue(static_cast<AZ::s32>(value));
                }
                else if constexpr (std::is_same_v<ElementType, uint16_t>)
                {
                    result = CesiumMetadataValue(static_cast<AZ::s32>(value));
                }
                else if constexpr (std::is_same_v<ElementType, int32_t>)
                {
                    result = CesiumMetadataValue(static_cast<AZ::s32>(value));
                }
                else if constexpr (std::is_same_v<ElementType, uint32_t>)
                {
                    result = CesiumMetadataValue(static_cast<AZ::s64>(value));
                }
                else if constexpr (std::is_same_v<ElementType, int64_t>)
                {
                    result = CesiumMetadataValue(static_cast<AZ::s64>(value));
                }
                else if constexpr (std::is_same_v<ElementType, uint64_t>)
                {
                    result = CesiumMetadataValue(static_cast<AZ::s64>(value));
                }
                else if constexpr (std::is_same_v<ElementType, float>)
                {
                    result = CesiumMetadataValue(static_cast<float>(value));
                }
                else if constexpr (std::is_same_v<ElementType, double>)
                {
                    result = CesiumMetadataValue(static_cast<double>(value));
                }
                else if constexpr (std::is_same_v<ElementType, std::string_view>)
                {
                    result = CesiumMetadataValue(AZStd::string(value.data(), value.size()));
                }
            });

        return result;
    }

    AZStd::unordered_map<AZStd::string, CesiumMetadataValue> CesiumPropertyTable::GetAllPropertiesForFeature(AZ::s64 featureId) const
    {
        AZStd::unordered_map<AZStd::string, CesiumMetadataValue> result;
        if (!IsValid() || featureId < 0 || featureId >= m_featureCount)
        {
            return result;
        }

        CesiumGltf::MetadataFeatureTableView tableView(m_model, m_featureTable);

        tableView.forEachProperty(
            [&result, featureId](const std::string& propertyName, auto propertyView)
            {
                if (propertyView.status() != CesiumGltf::MetadataPropertyViewStatus::Valid)
                {
                    return;
                }

                using ElementType = std::decay_t<decltype(propertyView.get(0))>;
                AZStd::string azPropertyName(propertyName.c_str());
                auto value = propertyView.get(featureId);

                if constexpr (std::is_same_v<ElementType, bool>)
                {
                    result[azPropertyName] = CesiumMetadataValue(static_cast<bool>(value));
                }
                else if constexpr (std::is_integral_v<ElementType> && sizeof(ElementType) <= 4)
                {
                    result[azPropertyName] = CesiumMetadataValue(static_cast<AZ::s32>(value));
                }
                else if constexpr (std::is_integral_v<ElementType> && sizeof(ElementType) > 4)
                {
                    result[azPropertyName] = CesiumMetadataValue(static_cast<AZ::s64>(value));
                }
                else if constexpr (std::is_same_v<ElementType, float>)
                {
                    result[azPropertyName] = CesiumMetadataValue(static_cast<float>(value));
                }
                else if constexpr (std::is_same_v<ElementType, double>)
                {
                    result[azPropertyName] = CesiumMetadataValue(static_cast<double>(value));
                }
                else if constexpr (std::is_same_v<ElementType, std::string_view>)
                {
                    result[azPropertyName] = CesiumMetadataValue(AZStd::string(value.data(), value.size()));
                }
            });

        return result;
    }
} // namespace Cesium
