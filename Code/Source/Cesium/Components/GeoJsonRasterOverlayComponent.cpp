#include <Cesium/Components/GeoJsonRasterOverlayComponent.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <Cesium3DTilesSelection/RasterOverlay.h>
#include <Cesium3DTilesSelection/TileMapServiceRasterOverlay.h>

// The current CesiumNative does not include a GeoJSON raster overlay class.
// This component provides the full O3DE infrastructure (config, editor UI, scripting).
// When CesiumNative is upgraded, replace LoadRasterOverlayImpl() with the native
// CesiumRasterOverlays::GeoJsonRasterOverlay implementation.

namespace Cesium
{
    void GeoJsonRasterOverlaySource::Reflect(AZ::ReflectContext* context)
    {
        if (auto* sc = azrtti_cast<AZ::SerializeContext*>(context))
        {
            sc->Class<GeoJsonRasterOverlaySource>()
                ->Version(0)
                ->Field("SourceType", &GeoJsonRasterOverlaySource::m_sourceType)
                ->Field("Url", &GeoJsonRasterOverlaySource::m_url)
                ->Field("InlineGeoJson", &GeoJsonRasterOverlaySource::m_inlineGeoJson)
                ->Field("StrokeR", &GeoJsonRasterOverlaySource::m_strokeR)
                ->Field("StrokeG", &GeoJsonRasterOverlaySource::m_strokeG)
                ->Field("StrokeB", &GeoJsonRasterOverlaySource::m_strokeB)
                ->Field("StrokeA", &GeoJsonRasterOverlaySource::m_strokeA)
                ->Field("FillR", &GeoJsonRasterOverlaySource::m_fillR)
                ->Field("FillG", &GeoJsonRasterOverlaySource::m_fillG)
                ->Field("FillB", &GeoJsonRasterOverlaySource::m_fillB)
                ->Field("FillA", &GeoJsonRasterOverlaySource::m_fillA)
                ->Field("StrokeWidth", &GeoJsonRasterOverlaySource::m_strokeWidth)
                ->Field("MinimumLevel", &GeoJsonRasterOverlaySource::m_minimumLevel)
                ->Field("MaximumLevel", &GeoJsonRasterOverlaySource::m_maximumLevel);
        }

        if (auto* bc = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            bc->Enum<static_cast<int>(GeoJsonSourceType::Url)>("GeoJsonSourceType_Url")
                ->Enum<static_cast<int>(GeoJsonSourceType::Inline)>("GeoJsonSourceType_Inline");

            bc->Class<GeoJsonRasterOverlaySource>("GeoJsonRasterOverlaySource")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/RasterOverlays")
                ->Property("Url", BehaviorValueProperty(&GeoJsonRasterOverlaySource::m_url))
                ->Property("InlineGeoJson", BehaviorValueProperty(&GeoJsonRasterOverlaySource::m_inlineGeoJson))
                ->Property("StrokeWidth", BehaviorValueProperty(&GeoJsonRasterOverlaySource::m_strokeWidth))
                ->Property("MinimumLevel", BehaviorValueProperty(&GeoJsonRasterOverlaySource::m_minimumLevel))
                ->Property("MaximumLevel", BehaviorValueProperty(&GeoJsonRasterOverlaySource::m_maximumLevel));
        }
    }

    GeoJsonRasterOverlaySource::GeoJsonRasterOverlaySource() = default;

    void GeoJsonRasterOverlayComponent::Reflect(AZ::ReflectContext* context)
    {
        GeoJsonRasterOverlaySource::Reflect(context);

        if (auto* sc = azrtti_cast<AZ::SerializeContext*>(context))
        {
            sc->Class<GeoJsonRasterOverlayComponent, AZ::Component, RasterOverlayComponent>()
                ->Version(0)
                ->Field("source", &GeoJsonRasterOverlayComponent::m_source);
        }

        if (auto* bc = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            bc->Class<GeoJsonRasterOverlayComponent>("GeoJsonRasterOverlayComponent")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/RasterOverlays")
                ->Method("SetConfiguration",
                    [](GeoJsonRasterOverlayComponent& c, const RasterOverlayConfiguration& cfg) { c.SetConfiguration(cfg); })
                ->Method("GetConfiguration",
                    [](const GeoJsonRasterOverlayComponent& c) { return c.GetConfiguration(); })
                ->Method("LoadRasterOverlay", &GeoJsonRasterOverlayComponent::LoadRasterOverlay);
        }
    }

    void GeoJsonRasterOverlayComponent::LoadRasterOverlay(const GeoJsonRasterOverlaySource& source)
    {
        m_source = source;
        RasterOverlayComponent::LoadRasterOverlay();
    }

    std::unique_ptr<Cesium3DTilesSelection::RasterOverlay> GeoJsonRasterOverlayComponent::LoadRasterOverlayImpl()
    {
        // NOTE: Current CesiumNative version does not include a GeoJSON raster overlay.
        // When upgraded, replace with native CesiumRasterOverlays::GeoJsonDocumentRasterOverlay.
        //
        // The component infrastructure (config, editor UI, Script Canvas) is complete.

        bool hasSource = (m_source.m_sourceType == GeoJsonSourceType::Url && !m_source.m_url.empty()) ||
                         (m_source.m_sourceType == GeoJsonSourceType::Inline && !m_source.m_inlineGeoJson.empty());

        if (!hasSource)
        {
            return nullptr;
        }

        AZ_Warning(
            "CesiumGeoJson", false,
            "GeoJSON raster overlay requires CesiumNative upgrade for native rendering. "
            "Component infrastructure is ready but the native overlay is not available in this version.");

        return nullptr;
    }
} // namespace Cesium
