#include <Cesium/Components/GoogleMapsRasterOverlayComponent.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <Cesium3DTilesSelection/RasterOverlay.h>
#include <Cesium3DTilesSelection/TileMapServiceRasterOverlay.h>

namespace Cesium
{
    namespace
    {
        const char* GoogleMapsMapTypeToString(GoogleMapsMapType type)
        {
            switch (type)
            {
            case GoogleMapsMapType::Satellite: return "s";
            case GoogleMapsMapType::Roadmap:   return "m";
            case GoogleMapsMapType::Terrain:   return "t";
            case GoogleMapsMapType::Hybrid:    return "y";
            default: return "s";
            }
        }
    }

    void GoogleMapsRasterOverlaySource::Reflect(AZ::ReflectContext* context)
    {
        if (auto* sc = azrtti_cast<AZ::SerializeContext*>(context))
        {
            sc->Class<GoogleMapsRasterOverlaySource>()
                ->Version(0)
                ->Field("ApiKey", &GoogleMapsRasterOverlaySource::m_apiKey)
                ->Field("MapType", &GoogleMapsRasterOverlaySource::m_mapType)
                ->Field("Language", &GoogleMapsRasterOverlaySource::m_language)
                ->Field("Region", &GoogleMapsRasterOverlaySource::m_region)
                ->Field("MinimumLevel", &GoogleMapsRasterOverlaySource::m_minimumLevel)
                ->Field("MaximumLevel", &GoogleMapsRasterOverlaySource::m_maximumLevel);
        }

        if (auto* bc = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            bc->Enum<static_cast<int>(GoogleMapsMapType::Satellite)>("GoogleMapsMapType_Satellite")
                ->Enum<static_cast<int>(GoogleMapsMapType::Roadmap)>("GoogleMapsMapType_Roadmap")
                ->Enum<static_cast<int>(GoogleMapsMapType::Terrain)>("GoogleMapsMapType_Terrain")
                ->Enum<static_cast<int>(GoogleMapsMapType::Hybrid)>("GoogleMapsMapType_Hybrid");

            bc->Class<GoogleMapsRasterOverlaySource>("GoogleMapsRasterOverlaySource")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/RasterOverlays")
                ->Property("ApiKey", BehaviorValueProperty(&GoogleMapsRasterOverlaySource::m_apiKey))
                ->Property("Language", BehaviorValueProperty(&GoogleMapsRasterOverlaySource::m_language))
                ->Property("Region", BehaviorValueProperty(&GoogleMapsRasterOverlaySource::m_region))
                ->Property("MinimumLevel", BehaviorValueProperty(&GoogleMapsRasterOverlaySource::m_minimumLevel))
                ->Property("MaximumLevel", BehaviorValueProperty(&GoogleMapsRasterOverlaySource::m_maximumLevel));
        }
    }

    GoogleMapsRasterOverlaySource::GoogleMapsRasterOverlaySource() = default;

    void GoogleMapsRasterOverlayComponent::Reflect(AZ::ReflectContext* context)
    {
        GoogleMapsRasterOverlaySource::Reflect(context);

        if (auto* sc = azrtti_cast<AZ::SerializeContext*>(context))
        {
            sc->Class<GoogleMapsRasterOverlayComponent, AZ::Component, RasterOverlayComponent>()
                ->Version(0)
                ->Field("source", &GoogleMapsRasterOverlayComponent::m_source);
        }

        if (auto* bc = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            bc->Class<GoogleMapsRasterOverlayComponent>("GoogleMapsRasterOverlayComponent")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/RasterOverlays")
                ->Method("SetConfiguration",
                    [](GoogleMapsRasterOverlayComponent& c, const RasterOverlayConfiguration& cfg) { c.SetConfiguration(cfg); })
                ->Method("GetConfiguration",
                    [](const GoogleMapsRasterOverlayComponent& c) { return c.GetConfiguration(); })
                ->Method("LoadRasterOverlay", &GoogleMapsRasterOverlayComponent::LoadRasterOverlay);
        }
    }

    void GoogleMapsRasterOverlayComponent::LoadRasterOverlay(const GoogleMapsRasterOverlaySource& source)
    {
        m_source = source;
        RasterOverlayComponent::LoadRasterOverlay();
    }

    std::unique_ptr<Cesium3DTilesSelection::RasterOverlay> GoogleMapsRasterOverlayComponent::LoadRasterOverlayImpl()
    {
        // Google Maps Tile API URL pattern:
        // https://maps.googleapis.com/maps/vt?lyrs={mapType}&x={x}&y={y}&z={z}&key={apiKey}
        // Adapted to TMS format by constructing a base URL that the TMS overlay appends /{z}/{x}/{y}.png to.
        //
        // Alternative: Use the Google Map Tiles API endpoint which follows XYZ pattern:
        // https://tile.googleapis.com/v1/2dtiles/{z}/{x}/{y}?session={session}&key={key}
        //
        // For simplicity, we use the public mt1.google.com endpoint with TMS:
        AZStd::string mapType = GoogleMapsMapTypeToString(m_source.m_mapType);

        AZStd::string url = AZStd::string::format(
            "https://mt1.google.com/vt/lyrs=%s", mapType.c_str());

        if (!m_source.m_language.empty())
        {
            url += AZStd::string::format("&hl=%s", m_source.m_language.c_str());
        }
        if (!m_source.m_region.empty())
        {
            url += AZStd::string::format("&gl=%s", m_source.m_region.c_str());
        }

        Cesium3DTilesSelection::TileMapServiceRasterOverlayOptions options{};
        if (m_source.m_maximumLevel > m_source.m_minimumLevel)
        {
            options.minimumLevel = m_source.m_minimumLevel;
            options.maximumLevel = m_source.m_maximumLevel;
        }
        options.fileExtension = "png";

        std::vector<CesiumAsync::IAssetAccessor::THeader> headers;

        return std::make_unique<Cesium3DTilesSelection::TileMapServiceRasterOverlay>(
            "GoogleMapsRasterOverlay", url.c_str(), headers, options);
    }
} // namespace Cesium
