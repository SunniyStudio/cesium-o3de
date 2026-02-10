#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/Component/EntityId.h>
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/std/containers/vector.h>

namespace Cesium
{
    //! Component that excludes (clips) tiles from a tileset using cartographic polygons.
    //! Attach to the same entity as a TilesetComponent.
    //! Reference entities that have CartographicPolygonComponent to define exclusion regions.
    //! Tiles that fall entirely within the polygon(s) will not be loaded or rendered,
    //! enabling "dig hole" scenarios for placing custom geometry inside 3D Tiles terrain.
    class CesiumTileExcluderComponent
        : public AZ::Component
        , public AZ::TickBus::Handler
    {
    public:
        AZ_COMPONENT(CesiumTileExcluderComponent, "{8B9C0D1E-2F3A-4B5C-6D7E-8F9A0B1C2D3E}", AZ::Component);

        static void Reflect(AZ::ReflectContext* context);
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        CesiumTileExcluderComponent();
        ~CesiumTileExcluderComponent() noexcept override;

        using AZ::Component::SetEntity;

        void Init() override;
        void Activate() override;
        void Deactivate() override;

        //! Set polygon entity references and enabled state (called by editor component).
        void SetPolygonEntityIds(const AZStd::vector<AZ::EntityId>& entityIds);
        void SetEnabled(bool enabled);

        // TickBus
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

    private:
        struct Impl;
        AZStd::unique_ptr<Impl> m_impl;

        //! Entity IDs of entities with CartographicPolygonComponent.
        AZStd::vector<AZ::EntityId> m_polygonEntityIds;

        //! Whether the excluder is enabled.
        bool m_enabled = true;
    };
} // namespace Cesium
