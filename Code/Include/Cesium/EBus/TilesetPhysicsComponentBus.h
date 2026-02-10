#pragma once

#include <AzCore/RTTI/RTTI.h>
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/RTTI/ReflectContext.h>
#include <AzCore/Component/ComponentBus.h>

namespace Cesium
{
    //! Configuration for tileset physics collision generation.
    struct TilesetPhysicsConfiguration final
    {
        AZ_RTTI(TilesetPhysicsConfiguration, "{7A1B2C3D-4E5F-6A7B-8C9D-0E1F2A3B4C5D}");
        AZ_CLASS_ALLOCATOR(TilesetPhysicsConfiguration, AZ::SystemAllocator, 0);

        static void Reflect(AZ::ReflectContext* context);

        TilesetPhysicsConfiguration();

        //! Whether physics collision is enabled.
        bool m_enabled = true;

        //! Whether to generate double-sided collision (both winding orders).
        bool m_doubleSided = true;

        //! Collision layer name (empty = Default).
        AZStd::string m_collisionLayerName;

        //! Maximum number of triangle meshes to cook per frame (0 = unlimited).
        //! Spreading cooking over multiple frames avoids frame hitches.
        AZ::u32 m_maxCooksPerFrame = 2;
    };

    //! EBus request interface for tileset physics.
    class TilesetPhysicsRequest : public AZ::ComponentBus
    {
    public:
        static void Reflect(AZ::ReflectContext* context);

        virtual void SetConfiguration(const TilesetPhysicsConfiguration& configuration) = 0;
        virtual const TilesetPhysicsConfiguration& GetConfiguration() const = 0;

        //! Enable or disable physics collision generation.
        virtual void SetEnabled(bool enabled) = 0;
        virtual bool GetEnabled() const = 0;

        //! Returns the total number of active physics bodies across all loaded tiles.
        virtual AZ::u32 GetActiveBodyCount() const = 0;
    };

    using TilesetPhysicsRequestBus = AZ::EBus<TilesetPhysicsRequest>;
} // namespace Cesium
