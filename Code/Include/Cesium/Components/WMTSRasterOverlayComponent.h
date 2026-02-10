#pragma once

#include <Cesium/Components/RasterOverlayComponent.h>
#include <AzCore/std/string/string.h>
#include <AzCore/std/containers/map.h>
#include <cstdint>
#include <memory>

namespace Cesium
{
    struct WMTSRasterOverlaySource final
    {
        AZ_RTTI(WMTSRasterOverlaySource, "{3D5E7F9A-1B2C-4D6E-8F0A-3B4C5D6E7F8A}");
        AZ_CLASS_ALLOCATOR(WMTSRasterOverlaySource, AZ::SystemAllocator, 0);

        static void Reflect(AZ::ReflectContext* context);

        WMTSRasterOverlaySource();

        AZStd::string m_baseUrl;               //!< WMTS service URL
        AZStd::string m_layer;                  //!< Layer identifier
        AZStd::string m_style;                  //!< Style identifier
        AZStd::string m_tileMatrixSet;          //!< Tile matrix set (e.g., "GoogleMapsCompatible")
        AZStd::string m_format;                 //!< Image format (e.g., "image/png")
        AZStd::unordered_map<AZStd::string, AZStd::string> m_headers;
        std::uint32_t m_minimumLevel;
        std::uint32_t m_maximumLevel;
    };

    class WMTSRasterOverlayComponent : public RasterOverlayComponent
    {
    public:
        AZ_COMPONENT(WMTSRasterOverlayComponent, "{4E6F8A0B-2C3D-5E7F-9A1B-4C5D6E7F8A9B}", RasterOverlayComponent)

        static void Reflect(AZ::ReflectContext* context);

        void LoadRasterOverlay(const WMTSRasterOverlaySource& source);

    private:
        std::unique_ptr<Cesium3DTilesSelection::RasterOverlay> LoadRasterOverlayImpl() override;

        WMTSRasterOverlaySource m_source;
    };
} // namespace Cesium
