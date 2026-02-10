#pragma once

#include <Cesium/Metadata/CesiumMetadataValue.h>
#include <AzCore/RTTI/RTTI.h>
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/RTTI/ReflectContext.h>
#include <AzCore/std/string/string.h>
#include <AzCore/std/containers/vector.h>
#include <AzCore/std/containers/unordered_map.h>

namespace CesiumGltf
{
    struct Model;
    struct FeatureTable;
    class MetadataFeatureTableView;
} // namespace CesiumGltf

namespace Cesium
{
    //! Describes the type of a property within a feature table.
    struct CesiumPropertyDescription final
    {
        AZ_RTTI(CesiumPropertyDescription, "{F1A2B3C4-D5E6-4F7A-8B9C-0D1E2F3A4B5C}");
        AZ_CLASS_ALLOCATOR(CesiumPropertyDescription, AZ::SystemAllocator, 0);

        static void Reflect(AZ::ReflectContext* context);

        AZStd::string m_name;
        CesiumMetadataValueType m_type = CesiumMetadataValueType::None;
        bool m_isArray = false;
    };

    //! Provides access to a feature table's properties.
    //! Wraps CesiumGltf::MetadataFeatureTableView for safe property value retrieval.
    class CesiumPropertyTable final
    {
    public:
        AZ_RTTI(CesiumPropertyTable, "{C5D9E7F3-A4B6-4C8D-0E1F-2A3B4C5D6E7F}");
        AZ_CLASS_ALLOCATOR(CesiumPropertyTable, AZ::SystemAllocator, 0);

        static void Reflect(AZ::ReflectContext* context);

        CesiumPropertyTable();

        //! Construct from CesiumGltf model and feature table.
        //! The model and table pointers must remain valid for the lifetime of this object.
        CesiumPropertyTable(
            const AZStd::string& name,
            const CesiumGltf::Model* model,
            const CesiumGltf::FeatureTable* featureTable);

        const AZStd::string& GetName() const;
        AZ::s64 GetFeatureCount() const;
        AZStd::vector<AZStd::string> GetPropertyNames() const;
        AZStd::vector<CesiumPropertyDescription> GetPropertyDescriptions() const;

        //! Get a property value for a specific feature ID.
        CesiumMetadataValue GetPropertyValue(const AZStd::string& propertyName, AZ::s64 featureId) const;

        //! Get all property values for a specific feature ID.
        AZStd::unordered_map<AZStd::string, CesiumMetadataValue> GetAllPropertiesForFeature(AZ::s64 featureId) const;

        bool IsValid() const;

    private:
        AZStd::string m_name;
        AZ::s64 m_featureCount = 0;
        const CesiumGltf::Model* m_model = nullptr;
        const CesiumGltf::FeatureTable* m_featureTable = nullptr;
    };
} // namespace Cesium
