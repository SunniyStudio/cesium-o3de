#pragma once

#include <Cesium/Components/RasterOverlayComponent.h>
#include <AzCore/std/string/string.h>
#include <cstdint>
#include <memory>

namespace Cesium
{
    enum class GoogleMapsMapType : AZ::u8
    {
        Satellite = 0,
        Roadmap,
        Terrain,
        Hybrid
    };

    struct GoogleMapsRasterOverlaySource final
    {
        AZ_RTTI(GoogleMapsRasterOverlaySource, "{7E8F9A0B-1C2D-3E4F-5A6B-7C8D9E0F1A2B}");
        AZ_CLASS_ALLOCATOR(GoogleMapsRasterOverlaySource, AZ::SystemAllocator, 0);

        static void Reflect(AZ::ReflectContext* context);
        GoogleMapsRasterOverlaySource();

        AZStd::string m_apiKey;
        GoogleMapsMapType m_mapType = GoogleMapsMapType::Satellite;
        AZStd::string m_language;       //!< e.g., "en", "zh-CN"
        AZStd::string m_region;         //!< e.g., "us", "cn"
        std::uint32_t m_minimumLevel = 0;
        std::uint32_t m_maximumLevel = 22;
    };

    class GoogleMapsRasterOverlayComponent : public RasterOverlayComponent
    {
    public:
        AZ_COMPONENT(GoogleMapsRasterOverlayComponent, "{8F0A1B2C-3D4E-5F6A-7B8C-9D0E1F2A3B4C}", RasterOverlayComponent)
        static void Reflect(AZ::ReflectContext* context);
        void LoadRasterOverlay(const GoogleMapsRasterOverlaySource& source);
    private:
        std::unique_ptr<Cesium3DTilesSelection::RasterOverlay> LoadRasterOverlayImpl() override;
        GoogleMapsRasterOverlaySource m_source;
    };
} // namespace Cesium
