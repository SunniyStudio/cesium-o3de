#pragma once

#include <Cesium/Components/RasterOverlayComponent.h>
#include <AzCore/std/string/string.h>
#include <cstdint>
#include <memory>

namespace Cesium
{
    enum class GeoJsonSourceType : AZ::u8
    {
        Url = 0,    //!< Load GeoJSON from a URL
        Inline      //!< Inline GeoJSON string
    };

    struct GeoJsonRasterOverlaySource final
    {
        AZ_RTTI(GeoJsonRasterOverlaySource, "{4D5E6F7A-8B9C-0D1E-2F3A-4B5C6D7E8F9A}");
        AZ_CLASS_ALLOCATOR(GeoJsonRasterOverlaySource, AZ::SystemAllocator, 0);

        static void Reflect(AZ::ReflectContext* context);
        GeoJsonRasterOverlaySource();

        GeoJsonSourceType m_sourceType = GeoJsonSourceType::Url;
        AZStd::string m_url;               //!< URL to a .geojson file
        AZStd::string m_inlineGeoJson;      //!< Inline GeoJSON content

        //! Stroke (outline) color RGBA 0-255
        AZ::u8 m_strokeR = 0;
        AZ::u8 m_strokeG = 120;
        AZ::u8 m_strokeB = 255;
        AZ::u8 m_strokeA = 255;

        //! Fill color RGBA 0-255
        AZ::u8 m_fillR = 0;
        AZ::u8 m_fillG = 120;
        AZ::u8 m_fillB = 255;
        AZ::u8 m_fillA = 80;

        float m_strokeWidth = 2.0f;         //!< Stroke width in pixels
        std::uint32_t m_minimumLevel = 0;
        std::uint32_t m_maximumLevel = 25;
    };

    class GeoJsonRasterOverlayComponent : public RasterOverlayComponent
    {
    public:
        AZ_COMPONENT(GeoJsonRasterOverlayComponent, "{5E6F7A8B-9C0D-1E2F-3A4B-5C6D7E8F9A0B}", RasterOverlayComponent)
        static void Reflect(AZ::ReflectContext* context);
        void LoadRasterOverlay(const GeoJsonRasterOverlaySource& source);
    private:
        std::unique_ptr<Cesium3DTilesSelection::RasterOverlay> LoadRasterOverlayImpl() override;
        GeoJsonRasterOverlaySource m_source;
    };
} // namespace Cesium
