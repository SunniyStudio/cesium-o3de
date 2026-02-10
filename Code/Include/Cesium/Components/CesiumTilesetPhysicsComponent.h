#pragma once

#include <Cesium/EBus/TilesetPhysicsComponentBus.h>
#include <Cesium/EBus/OriginShiftComponentBus.h>
#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/Memory/SystemAllocator.h>

namespace Cesium
{
    //! Component that generates PhysX collision bodies for loaded 3D Tiles.
    //! Attach to the same entity as a TilesetComponent.
    //! Creates static triangle-mesh collision shapes for each loaded tile,
    //! enabling ray casts and physics interactions with the tileset geometry.
    class CesiumTilesetPhysicsComponent
        : public AZ::Component
        , public TilesetPhysicsRequestBus::Handler
        , public AZ::TickBus::Handler
        , public OriginShiftNotificationBus::Handler
    {
    public:
        AZ_COMPONENT(CesiumTilesetPhysicsComponent, "{8F2A3B4C-5D6E-7F8A-9B0C-1D2E3F4A5B6C}", AZ::Component);

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        CesiumTilesetPhysicsComponent();
        ~CesiumTilesetPhysicsComponent() noexcept override;

        void Init() override;
        void Activate() override;
        void Deactivate() override;

        // TilesetPhysicsRequestBus overrides
        void SetConfiguration(const TilesetPhysicsConfiguration& configuration) override;
        const TilesetPhysicsConfiguration& GetConfiguration() const override;
        void SetEnabled(bool enabled) override;
        bool GetEnabled() const override;
        AZ::u32 GetActiveBodyCount() const override;

        // TickBus overrides
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

        // OriginShiftNotificationBus overrides
        void OnOriginShifting(const glm::dmat4& absToRelWorld) override;

    private:
        struct Impl;
        AZStd::unique_ptr<Impl> m_impl;
        TilesetPhysicsConfiguration m_configuration;
    };
} // namespace Cesium
