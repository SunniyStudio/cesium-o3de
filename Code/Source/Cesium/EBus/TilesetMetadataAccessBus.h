#pragma once

#include <AzCore/Component/ComponentBus.h>
#include <AzCore/std/containers/vector.h>

namespace CesiumGltf
{
    struct Model;
}

namespace Cesium
{
    struct IntrusiveGltfModel;

    //! Internal EBus for accessing tile metadata from TilesetComponent.
    //! This is NOT a public API - used only by CesiumMetadataComponent.
    class TilesetMetadataAccessRequest : public AZ::ComponentBus
    {
    public:
        struct TileMetadataEntry
        {
            const CesiumGltf::Model* m_sourceModel = nullptr;
            IntrusiveGltfModel* m_renderModel = nullptr;
        };

        //! Returns metadata entries for all currently loaded (visible or recently loaded) tiles.
        virtual AZStd::vector<TileMetadataEntry> GetLoadedTilesWithMetadata() const = 0;
    };

    using TilesetMetadataAccessBus = AZ::EBus<TilesetMetadataAccessRequest>;
} // namespace Cesium
