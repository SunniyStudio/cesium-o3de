#include <Cesium/Components/AzureMapsRasterOverlayComponent.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <Cesium3DTilesSelection/RasterOverlay.h>
#include <Cesium3DTilesSelection/TileMapServiceRasterOverlay.h>

namespace Cesium
{
    namespace
    {
        const char* AzureMapsStyleToTilesetId(AzureMapsMapStyle style)
        {
            switch (style)
            {
            case AzureMapsMapStyle::RoadLight:           return "microsoft.base.road";
            case AzureMapsMapStyle::RoadDark:            return "microsoft.base.darkgrey";
            case AzureMapsMapStyle::Satellite:           return "microsoft.imagery";
            case AzureMapsMapStyle::SatelliteRoadLabels: return "microsoft.imagery";
            case AzureMapsMapStyle::GrayscaleLight:      return "microsoft.base.road";
            case AzureMapsMapStyle::GrayscaleDark:       return "microsoft.base.darkgrey";
            case AzureMapsMapStyle::NightView:           return "microsoft.base.darkgrey";
            case AzureMapsMapStyle::HighContrastLight:   return "microsoft.base.road";
            case AzureMapsMapStyle::HighContrastDark:    return "microsoft.base.darkgrey";
            default: return "microsoft.imagery";
            }
        }
    }

    void AzureMapsRasterOverlaySource::Reflect(AZ::ReflectContext* context)
    {
        if (auto* sc = azrtti_cast<AZ::SerializeContext*>(context))
        {
            sc->Class<AzureMapsRasterOverlaySource>()
                ->Version(0)
                ->Field("SubscriptionKey", &AzureMapsRasterOverlaySource::m_subscriptionKey)
                ->Field("MapStyle", &AzureMapsRasterOverlaySource::m_mapStyle)
                ->Field("Language", &AzureMapsRasterOverlaySource::m_language)
                ->Field("TileSize", &AzureMapsRasterOverlaySource::m_tileSize)
                ->Field("MinimumLevel", &AzureMapsRasterOverlaySource::m_minimumLevel)
                ->Field("MaximumLevel", &AzureMapsRasterOverlaySource::m_maximumLevel);
        }

        if (auto* bc = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            bc->Enum<static_cast<int>(AzureMapsMapStyle::RoadLight)>("AzureMapsMapStyle_RoadLight")
                ->Enum<static_cast<int>(AzureMapsMapStyle::RoadDark)>("AzureMapsMapStyle_RoadDark")
                ->Enum<static_cast<int>(AzureMapsMapStyle::Satellite)>("AzureMapsMapStyle_Satellite")
                ->Enum<static_cast<int>(AzureMapsMapStyle::SatelliteRoadLabels)>("AzureMapsMapStyle_SatelliteRoadLabels")
                ->Enum<static_cast<int>(AzureMapsMapStyle::GrayscaleLight)>("AzureMapsMapStyle_GrayscaleLight")
                ->Enum<static_cast<int>(AzureMapsMapStyle::GrayscaleDark)>("AzureMapsMapStyle_GrayscaleDark")
                ->Enum<static_cast<int>(AzureMapsMapStyle::NightView)>("AzureMapsMapStyle_NightView")
                ->Enum<static_cast<int>(AzureMapsMapStyle::HighContrastLight)>("AzureMapsMapStyle_HighContrastLight")
                ->Enum<static_cast<int>(AzureMapsMapStyle::HighContrastDark)>("AzureMapsMapStyle_HighContrastDark");

            bc->Class<AzureMapsRasterOverlaySource>("AzureMapsRasterOverlaySource")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/RasterOverlays")
                ->Property("SubscriptionKey", BehaviorValueProperty(&AzureMapsRasterOverlaySource::m_subscriptionKey))
                ->Property("Language", BehaviorValueProperty(&AzureMapsRasterOverlaySource::m_language))
                ->Property("TileSize", BehaviorValueProperty(&AzureMapsRasterOverlaySource::m_tileSize))
                ->Property("MinimumLevel", BehaviorValueProperty(&AzureMapsRasterOverlaySource::m_minimumLevel))
                ->Property("MaximumLevel", BehaviorValueProperty(&AzureMapsRasterOverlaySource::m_maximumLevel));
        }
    }

    AzureMapsRasterOverlaySource::AzureMapsRasterOverlaySource() = default;

    void AzureMapsRasterOverlayComponent::Reflect(AZ::ReflectContext* context)
    {
        AzureMapsRasterOverlaySource::Reflect(context);

        if (auto* sc = azrtti_cast<AZ::SerializeContext*>(context))
        {
            sc->Class<AzureMapsRasterOverlayComponent, AZ::Component, RasterOverlayComponent>()
                ->Version(0)
                ->Field("source", &AzureMapsRasterOverlayComponent::m_source);
        }

        if (auto* bc = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            bc->Class<AzureMapsRasterOverlayComponent>("AzureMapsRasterOverlayComponent")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/RasterOverlays")
                ->Method("SetConfiguration",
                    [](AzureMapsRasterOverlayComponent& c, const RasterOverlayConfiguration& cfg) { c.SetConfiguration(cfg); })
                ->Method("GetConfiguration",
                    [](const AzureMapsRasterOverlayComponent& c) { return c.GetConfiguration(); })
                ->Method("LoadRasterOverlay", &AzureMapsRasterOverlayComponent::LoadRasterOverlay);
        }
    }

    void AzureMapsRasterOverlayComponent::LoadRasterOverlay(const AzureMapsRasterOverlaySource& source)
    {
        m_source = source;
        RasterOverlayComponent::LoadRasterOverlay();
    }

    std::unique_ptr<Cesium3DTilesSelection::RasterOverlay> AzureMapsRasterOverlayComponent::LoadRasterOverlayImpl()
    {
        // Azure Maps Render V2 tile endpoint:
        // https://atlas.microsoft.com/map/tile?api-version=2.1&tilesetId={tilesetId}&zoom={z}&x={x}&y={y}
        //   &tileSize={tileSize}&language={language}&subscription-key={key}
        // Adapted to TMS by constructing a base URL.
        const char* tilesetId = AzureMapsStyleToTilesetId(m_source.m_mapStyle);

        AZStd::string url = AZStd::string::format(
            "https://atlas.microsoft.com/map/tile?api-version=2.1&tilesetId=%s&tileSize=%u",
            tilesetId,
            m_source.m_tileSize);

        if (!m_source.m_language.empty())
        {
            url += AZStd::string::format("&language=%s", m_source.m_language.c_str());
        }

        Cesium3DTilesSelection::TileMapServiceRasterOverlayOptions options{};
        if (m_source.m_maximumLevel > m_source.m_minimumLevel)
        {
            options.minimumLevel = m_source.m_minimumLevel;
            options.maximumLevel = m_source.m_maximumLevel;
        }
        options.fileExtension = "png";

        // Pass subscription key via header for security
        std::vector<CesiumAsync::IAssetAccessor::THeader> headers;
        if (!m_source.m_subscriptionKey.empty())
        {
            headers.emplace_back("subscription-key", m_source.m_subscriptionKey.c_str());
        }

        return std::make_unique<Cesium3DTilesSelection::TileMapServiceRasterOverlay>(
            "AzureMapsRasterOverlay", url.c_str(), headers, options);
    }
} // namespace Cesium
