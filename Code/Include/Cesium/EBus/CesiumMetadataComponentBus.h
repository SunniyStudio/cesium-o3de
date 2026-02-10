#pragma once

#include <Cesium/Metadata/CesiumMetadataValue.h>
#include <AzCore/Component/ComponentBus.h>
#include <AzCore/std/string/string.h>
#include <AzCore/std/containers/vector.h>
#include <AzCore/std/containers/unordered_map.h>
#include <glm/glm.hpp>

namespace Cesium
{
    struct CesiumPropertyDescription;

    //! EBus request interface for querying 3D Tiles metadata.
    //! Attach a CesiumMetadataComponent to the same entity as a TilesetComponent to use.
    class CesiumMetadataRequest : public AZ::ComponentBus
    {
    public:
        static void Reflect(AZ::ReflectContext* context);

        //! Returns the names of all feature tables across all currently loaded tiles.
        virtual AZStd::vector<AZStd::string> GetFeatureTableNames() const = 0;

        //! Returns the number of features in the named feature table.
        //! Returns -1 if the table is not found.
        virtual AZ::s64 GetFeatureCount(const AZStd::string& featureTableName) const = 0;

        //! Returns descriptions of all properties in the named feature table.
        virtual AZStd::vector<CesiumPropertyDescription> GetPropertyDescriptions(const AZStd::string& featureTableName) const = 0;

        //! Returns the property names in the named feature table.
        virtual AZStd::vector<AZStd::string> GetPropertyNames(const AZStd::string& featureTableName) const = 0;

        //! Returns a single property value from the named feature table.
        virtual CesiumMetadataValue GetPropertyValue(
            const AZStd::string& featureTableName,
            const AZStd::string& propertyName,
            AZ::s64 featureId) const = 0;

        //! Returns all property values for the given feature ID from the named feature table.
        virtual AZStd::unordered_map<AZStd::string, CesiumMetadataValue> GetAllPropertiesForFeature(
            const AZStd::string& featureTableName,
            AZ::s64 featureId) const = 0;

        //! Performs a software ray cast against loaded tile geometry and returns the feature ID
        //! of the first hit primitive. Returns -1 if no feature is hit.
        //! @param origin    Ray origin in world space (relative to current origin shift).
        //! @param direction Ray direction (need not be normalized).
        //! @param maxDistance Maximum ray distance.
        //! @param featureIdSetIndex Index of the feature ID set to use (default 0).
        virtual AZ::s64 GetFeatureIdFromRaycast(
            const glm::dvec3& origin,
            const glm::dvec3& direction,
            double maxDistance,
            AZ::s32 featureIdSetIndex) const = 0;

        //! Performs a ray cast and returns all property values for the hit feature.
        //! Returns an empty map if nothing is hit.
        virtual AZStd::unordered_map<AZStd::string, CesiumMetadataValue> GetPropertiesFromRaycast(
            const glm::dvec3& origin,
            const glm::dvec3& direction,
            double maxDistance,
            AZ::s32 featureIdSetIndex) const = 0;
    };

    using CesiumMetadataRequestBus = AZ::EBus<CesiumMetadataRequest>;
} // namespace Cesium
