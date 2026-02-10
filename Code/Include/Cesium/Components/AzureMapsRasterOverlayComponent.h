#pragma once

#include <Cesium/Components/RasterOverlayComponent.h>
#include <AzCore/std/string/string.h>
#include <cstdint>
#include <memory>

namespace Cesium
{
    enum class AzureMapsMapStyle : AZ::u8
    {
        RoadLight = 0,
        RoadDark,
        Satellite,
        SatelliteRoadLabels,
        GrayscaleLight,
        GrayscaleDark,
        NightView,
        HighContrastLight,
        HighContrastDark
    };

    struct AzureMapsRasterOverlaySource final
    {
        AZ_RTTI(AzureMapsRasterOverlaySource, "{9A1B2C3D-4E5F-6A7B-8C9D-0E1F2A3B4C5D}");
        AZ_CLASS_ALLOCATOR(AzureMapsRasterOverlaySource, AZ::SystemAllocator, 0);

        static void Reflect(AZ::ReflectContext* context);
        AzureMapsRasterOverlaySource();

        AZStd::string m_subscriptionKey;
        AzureMapsMapStyle m_mapStyle = AzureMapsMapStyle::Satellite;
        AZStd::string m_language;       //!< e.g., "en-US", "zh-Hans-CN"
        std::uint32_t m_tileSize = 256; //!< 256 or 512
        std::uint32_t m_minimumLevel = 0;
        std::uint32_t m_maximumLevel = 22;
    };

    class AzureMapsRasterOverlayComponent : public RasterOverlayComponent
    {
    public:
        AZ_COMPONENT(AzureMapsRasterOverlayComponent, "{0B2C3D4E-5F6A-7B8C-9D0E-1F2A3B4C5D6E}", RasterOverlayComponent)
        static void Reflect(AZ::ReflectContext* context);
        void LoadRasterOverlay(const AzureMapsRasterOverlaySource& source);
    private:
        std::unique_ptr<Cesium3DTilesSelection::RasterOverlay> LoadRasterOverlayImpl() override;
        AzureMapsRasterOverlaySource m_source;
    };
} // namespace Cesium
