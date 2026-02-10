#pragma once

#include <Cesium/Components/RasterOverlayComponent.h>
#include <AzCore/std/string/string.h>
#include <AzCore/std/containers/map.h>
#include <cstdint>
#include <memory>

namespace Cesium
{
    struct WMSRasterOverlaySource final
    {
        AZ_RTTI(WMSRasterOverlaySource, "{1B3C5D7E-9A2B-4C6D-8E0F-1A2B3C4D5E6F}");
        AZ_CLASS_ALLOCATOR(WMSRasterOverlaySource, AZ::SystemAllocator, 0);

        static void Reflect(AZ::ReflectContext* context);

        WMSRasterOverlaySource();

        AZStd::string m_baseUrl;               //!< WMS service base URL
        AZStd::string m_layers;                 //!< Comma-separated layer names
        AZStd::string m_srs;                    //!< Spatial reference system (e.g., "EPSG:4326")
        AZStd::string m_format;                 //!< Image format (e.g., "image/png")
        AZStd::string m_version;                //!< WMS version (e.g., "1.1.1")
        AZStd::string m_styles;                 //!< Comma-separated styles (empty = default)
        AZStd::unordered_map<AZStd::string, AZStd::string> m_headers;
        std::uint32_t m_tileWidth;              //!< Tile width in pixels
        std::uint32_t m_tileHeight;             //!< Tile height in pixels
        std::uint32_t m_minimumLevel;
        std::uint32_t m_maximumLevel;
    };

    class WMSRasterOverlayComponent : public RasterOverlayComponent
    {
    public:
        AZ_COMPONENT(WMSRasterOverlayComponent, "{2C4D6E8F-0A1B-3C5D-7E9F-2A3B4C5D6E7F}", RasterOverlayComponent)

        static void Reflect(AZ::ReflectContext* context);

        void LoadRasterOverlay(const WMSRasterOverlaySource& source);

    private:
        std::unique_ptr<Cesium3DTilesSelection::RasterOverlay> LoadRasterOverlayImpl() override;

        WMSRasterOverlaySource m_source;
    };
} // namespace Cesium
