#pragma once

#include <Cesium/EBus/CesiumMetadataComponentBus.h>
#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/Memory/SystemAllocator.h>

namespace Cesium
{
    //! Component that enables 3D Tiles metadata queries on a tileset entity.
    //! Attach to the same entity as a TilesetComponent.
    //! Provides EBus API for querying feature tables, property values,
    //! and performing ray-based metadata picking.
    class CesiumMetadataComponent
        : public AZ::Component
        , public CesiumMetadataRequestBus::Handler
        , public AZ::TickBus::Handler
    {
    public:
        AZ_COMPONENT(CesiumMetadataComponent, "{D6E0F8A4-B5C7-4D9E-1F2A-3B4C5D6E7F8A}", AZ::Component);

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        CesiumMetadataComponent();
        ~CesiumMetadataComponent() noexcept override;

        void Init() override;
        void Activate() override;
        void Deactivate() override;

        // CesiumMetadataRequestBus overrides
        AZStd::vector<AZStd::string> GetFeatureTableNames() const override;
        AZ::s64 GetFeatureCount(const AZStd::string& featureTableName) const override;
        AZStd::vector<CesiumPropertyDescription> GetPropertyDescriptions(const AZStd::string& featureTableName) const override;
        AZStd::vector<AZStd::string> GetPropertyNames(const AZStd::string& featureTableName) const override;
        CesiumMetadataValue GetPropertyValue(
            const AZStd::string& featureTableName,
            const AZStd::string& propertyName,
            AZ::s64 featureId) const override;
        AZStd::unordered_map<AZStd::string, CesiumMetadataValue> GetAllPropertiesForFeature(
            const AZStd::string& featureTableName,
            AZ::s64 featureId) const override;
        AZ::s64 GetFeatureIdFromRaycast(
            const glm::dvec3& origin,
            const glm::dvec3& direction,
            double maxDistance,
            AZ::s32 featureIdSetIndex) const override;
        AZStd::unordered_map<AZStd::string, CesiumMetadataValue> GetPropertiesFromRaycast(
            const glm::dvec3& origin,
            const glm::dvec3& direction,
            double maxDistance,
            AZ::s32 featureIdSetIndex) const override;

        // TickBus overrides
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

    private:
        struct Impl;
        AZStd::unique_ptr<Impl> m_impl;
    };
} // namespace Cesium
