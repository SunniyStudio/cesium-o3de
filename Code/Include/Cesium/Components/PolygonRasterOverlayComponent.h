#pragma once

#include <Cesium/Components/RasterOverlayComponent.h>
#include <AzCore/std/string/string.h>
#include <AzCore/std/containers/vector.h>
#include <AzCore/Math/Vector2.h>
#include <glm/glm.hpp>
#include <cstdint>
#include <memory>

namespace Cesium
{
    //! A single cartographic polygon defined by a ring of longitude/latitude points.
    struct CartographicPolygon final
    {
        AZ_RTTI(CartographicPolygon, "{1A2B3C4D-5E6F-7A8B-9C0D-1E2F3A4B5C6D}");
        AZ_CLASS_ALLOCATOR(CartographicPolygon, AZ::SystemAllocator, 0);

        static void Reflect(AZ::ReflectContext* context);

        //! Polygon vertices as (longitude, latitude) in degrees.
        //! The polygon is automatically closed (first point connects to last).
        AZStd::vector<glm::dvec2> m_points;
    };

    struct PolygonRasterOverlaySource final
    {
        AZ_RTTI(PolygonRasterOverlaySource, "{2B3C4D5E-6F7A-8B9C-0D1E-2F3A4B5C6D7E}");
        AZ_CLASS_ALLOCATOR(PolygonRasterOverlaySource, AZ::SystemAllocator, 0);

        static void Reflect(AZ::ReflectContext* context);

        PolygonRasterOverlaySource();

        //! List of polygons to rasterize.
        AZStd::vector<CartographicPolygon> m_polygons;

        //! If true, the area INSIDE the polygons is excluded (masked out).
        //! If false, the area OUTSIDE the polygons is excluded.
        bool m_invertSelection = false;

        //! Fill color for the rasterized polygon area (RGBA, 0-255).
        AZ::u8 m_fillColorR = 255;
        AZ::u8 m_fillColorG = 255;
        AZ::u8 m_fillColorB = 255;
        AZ::u8 m_fillColorA = 255;

        //! Background color outside the polygon area (RGBA, 0-255).
        AZ::u8 m_bgColorR = 0;
        AZ::u8 m_bgColorG = 0;
        AZ::u8 m_bgColorB = 0;
        AZ::u8 m_bgColorA = 0;
    };

    class PolygonRasterOverlayComponent : public RasterOverlayComponent
    {
    public:
        AZ_COMPONENT(PolygonRasterOverlayComponent, "{3C4D5E6F-7A8B-9C0D-1E2F-3A4B5C6D7E8F}", RasterOverlayComponent)

        static void Reflect(AZ::ReflectContext* context);

        void LoadRasterOverlay(const PolygonRasterOverlaySource& source);

    private:
        std::unique_ptr<Cesium3DTilesSelection::RasterOverlay> LoadRasterOverlayImpl() override;

        PolygonRasterOverlaySource m_source;
    };
} // namespace Cesium
