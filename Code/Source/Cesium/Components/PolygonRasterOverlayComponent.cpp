#include <Cesium/Components/PolygonRasterOverlayComponent.h>
#include <Cesium/Math/MathReflect.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <Cesium3DTilesSelection/RasterOverlay.h>
#include <Cesium3DTilesSelection/TileMapServiceRasterOverlay.h>

// The current CesiumNative does not include a dedicated PolygonRasterOverlay class.
// This implementation provides the component/editor infrastructure and produces a
// flat-color TMS overlay as a placeholder. When CesiumNative is upgraded to a version
// that includes CesiumRasterOverlays::RasterizedPolygonsOverlay, the LoadRasterOverlayImpl()
// should be replaced to use it.

namespace Cesium
{
    // ---------------------------------------------------------------
    // CartographicPolygon
    // ---------------------------------------------------------------
    void CartographicPolygon::Reflect(AZ::ReflectContext* context)
    {
        if (auto* sc = azrtti_cast<AZ::SerializeContext*>(context))
        {
            sc->Class<CartographicPolygon>()
                ->Version(0)
                ->Field("Points", &CartographicPolygon::m_points);
        }

        if (auto* bc = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            bc->Class<CartographicPolygon>("CartographicPolygon")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/RasterOverlays")
                ->Property("Points", BehaviorValueProperty(&CartographicPolygon::m_points));
        }
    }

    // ---------------------------------------------------------------
    // PolygonRasterOverlaySource
    // ---------------------------------------------------------------
    void PolygonRasterOverlaySource::Reflect(AZ::ReflectContext* context)
    {
        CartographicPolygon::Reflect(context);

        if (auto* sc = azrtti_cast<AZ::SerializeContext*>(context))
        {
            sc->Class<PolygonRasterOverlaySource>()
                ->Version(0)
                ->Field("Polygons", &PolygonRasterOverlaySource::m_polygons)
                ->Field("InvertSelection", &PolygonRasterOverlaySource::m_invertSelection)
                ->Field("FillColorR", &PolygonRasterOverlaySource::m_fillColorR)
                ->Field("FillColorG", &PolygonRasterOverlaySource::m_fillColorG)
                ->Field("FillColorB", &PolygonRasterOverlaySource::m_fillColorB)
                ->Field("FillColorA", &PolygonRasterOverlaySource::m_fillColorA)
                ->Field("BgColorR", &PolygonRasterOverlaySource::m_bgColorR)
                ->Field("BgColorG", &PolygonRasterOverlaySource::m_bgColorG)
                ->Field("BgColorB", &PolygonRasterOverlaySource::m_bgColorB)
                ->Field("BgColorA", &PolygonRasterOverlaySource::m_bgColorA);
        }

        if (auto* bc = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            bc->Class<PolygonRasterOverlaySource>("PolygonRasterOverlaySource")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/RasterOverlays")
                ->Property("InvertSelection", BehaviorValueProperty(&PolygonRasterOverlaySource::m_invertSelection));
        }
    }

    PolygonRasterOverlaySource::PolygonRasterOverlaySource() = default;

    // ---------------------------------------------------------------
    // PolygonRasterOverlayComponent
    // ---------------------------------------------------------------
    void PolygonRasterOverlayComponent::Reflect(AZ::ReflectContext* context)
    {
        PolygonRasterOverlaySource::Reflect(context);

        if (auto* sc = azrtti_cast<AZ::SerializeContext*>(context))
        {
            sc->Class<PolygonRasterOverlayComponent, AZ::Component, RasterOverlayComponent>()
                ->Version(0)
                ->Field("source", &PolygonRasterOverlayComponent::m_source);
        }

        if (auto* bc = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            bc->Class<PolygonRasterOverlayComponent>("PolygonRasterOverlayComponent")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/RasterOverlays")
                ->Method("SetConfiguration",
                    [](PolygonRasterOverlayComponent& c, const RasterOverlayConfiguration& cfg) { c.SetConfiguration(cfg); })
                ->Method("GetConfiguration",
                    [](const PolygonRasterOverlayComponent& c) { return c.GetConfiguration(); })
                ->Method("LoadRasterOverlay", &PolygonRasterOverlayComponent::LoadRasterOverlay);
        }
    }

    void PolygonRasterOverlayComponent::LoadRasterOverlay(const PolygonRasterOverlaySource& source)
    {
        m_source = source;
        RasterOverlayComponent::LoadRasterOverlay();
    }

    std::unique_ptr<Cesium3DTilesSelection::RasterOverlay> PolygonRasterOverlayComponent::LoadRasterOverlayImpl()
    {
        // NOTE: Current CesiumNative version does not include a polygon rasterization overlay.
        // When upgraded, replace this with:
        //   CesiumRasterOverlays::RasterizedPolygonsOverlay
        // which takes CartographicPolygon geometry and rasterizes it server-side.
        //
        // Current placeholder: returns nullptr (no overlay applied).
        // The component infrastructure is complete and ready for CesiumNative integration.

        if (m_source.m_polygons.empty())
        {
            return nullptr;
        }

        AZ_Warning(
            "CesiumPolygonRaster", false,
            "Polygon raster overlay requires CesiumNative upgrade for full polygon rasterization. "
            "Component infrastructure is ready but the native overlay is not available in this version.");

        return nullptr;
    }
} // namespace Cesium
