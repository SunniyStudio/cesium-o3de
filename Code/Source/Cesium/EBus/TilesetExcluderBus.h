#pragma once

#include <AzCore/Component/ComponentBus.h>
#include <AzCore/std/smart_ptr/shared_ptr.h>
#include <memory>

namespace Cesium3DTilesSelection
{
    class ITileExcluder;
}

namespace Cesium
{
    //! Internal EBus for injecting tile excluders into TilesetComponent.
    //! Implemented by TilesetComponent::Impl.
    class TilesetExcluderRequest : public AZ::ComponentBus
    {
    public:
        //! Add a tile excluder to the tileset options.
        virtual void AddTileExcluder(std::shared_ptr<Cesium3DTilesSelection::ITileExcluder> excluder) = 0;

        //! Remove a specific tile excluder.
        virtual void RemoveTileExcluder(std::shared_ptr<Cesium3DTilesSelection::ITileExcluder> excluder) = 0;

        //! Remove all tile excluders added through this bus.
        virtual void RemoveAllTileExcluders() = 0;
    };

    using TilesetExcluderBus = AZ::EBus<TilesetExcluderRequest>;
} // namespace Cesium
