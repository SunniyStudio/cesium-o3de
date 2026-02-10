#include <Cesium/Components/WMSRasterOverlayComponent.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <Cesium3DTilesSelection/RasterOverlay.h>
#include <Cesium3DTilesSelection/TileMapServiceRasterOverlay.h>

namespace Cesium
{
    void WMSRasterOverlaySource::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<WMSRasterOverlaySource>()
                ->Version(0)
                ->Field("BaseUrl", &WMSRasterOverlaySource::m_baseUrl)
                ->Field("Layers", &WMSRasterOverlaySource::m_layers)
                ->Field("Srs", &WMSRasterOverlaySource::m_srs)
                ->Field("Format", &WMSRasterOverlaySource::m_format)
                ->Field("Version", &WMSRasterOverlaySource::m_version)
                ->Field("Styles", &WMSRasterOverlaySource::m_styles)
                ->Field("Headers", &WMSRasterOverlaySource::m_headers)
                ->Field("TileWidth", &WMSRasterOverlaySource::m_tileWidth)
                ->Field("TileHeight", &WMSRasterOverlaySource::m_tileHeight)
                ->Field("MinimumLevel", &WMSRasterOverlaySource::m_minimumLevel)
                ->Field("MaximumLevel", &WMSRasterOverlaySource::m_maximumLevel);
        }

        if (auto* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Class<WMSRasterOverlaySource>("WMSRasterOverlaySource")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/RasterOverlays")
                ->Property("BaseUrl", BehaviorValueProperty(&WMSRasterOverlaySource::m_baseUrl))
                ->Property("Layers", BehaviorValueProperty(&WMSRasterOverlaySource::m_layers))
                ->Property("Srs", BehaviorValueProperty(&WMSRasterOverlaySource::m_srs))
                ->Property("Format", BehaviorValueProperty(&WMSRasterOverlaySource::m_format))
                ->Property("Version", BehaviorValueProperty(&WMSRasterOverlaySource::m_version))
                ->Property("Styles", BehaviorValueProperty(&WMSRasterOverlaySource::m_styles))
                ->Property("TileWidth", BehaviorValueProperty(&WMSRasterOverlaySource::m_tileWidth))
                ->Property("TileHeight", BehaviorValueProperty(&WMSRasterOverlaySource::m_tileHeight))
                ->Property("MinimumLevel", BehaviorValueProperty(&WMSRasterOverlaySource::m_minimumLevel))
                ->Property("MaximumLevel", BehaviorValueProperty(&WMSRasterOverlaySource::m_maximumLevel));
        }
    }

    WMSRasterOverlaySource::WMSRasterOverlaySource()
        : m_srs{ "EPSG:4326" }
        , m_format{ "image/png" }
        , m_version{ "1.1.1" }
        , m_tileWidth{ 256 }
        , m_tileHeight{ 256 }
        , m_minimumLevel{ 0 }
        , m_maximumLevel{ 25 }
    {
    }

    void WMSRasterOverlayComponent::Reflect(AZ::ReflectContext* context)
    {
        WMSRasterOverlaySource::Reflect(context);

        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<WMSRasterOverlayComponent, AZ::Component, RasterOverlayComponent>()
                ->Version(0)
                ->Field("source", &WMSRasterOverlayComponent::m_source);
        }

        if (auto* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Class<WMSRasterOverlayComponent>("WMSRasterOverlayComponent")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/RasterOverlays")
                ->Method(
                    "SetConfiguration",
                    [](WMSRasterOverlayComponent& c, const RasterOverlayConfiguration& cfg) { c.SetConfiguration(cfg); })
                ->Method(
                    "GetConfiguration",
                    [](const WMSRasterOverlayComponent& c) { return c.GetConfiguration(); })
                ->Method("LoadRasterOverlay", &WMSRasterOverlayComponent::LoadRasterOverlay);
        }
    }

    void WMSRasterOverlayComponent::LoadRasterOverlay(const WMSRasterOverlaySource& source)
    {
        m_source = source;
        RasterOverlayComponent::LoadRasterOverlay();
    }

    std::unique_ptr<Cesium3DTilesSelection::RasterOverlay> WMSRasterOverlayComponent::LoadRasterOverlayImpl()
    {
        // Build WMS GetMap URL as a TMS-compatible endpoint.
        // WMS services that support tiled access can be adapted via TMS by constructing
        // the URL to include WMS query parameters. The tile coordinates are mapped
        // to GetMap BBOX parameters by the service or a proxy.
        //
        // For direct WMS access, construct the base URL with all fixed params.
        // The TMS overlay will append /{z}/{x}/{y}.{ext} to fetch tiles.
        AZStd::string constructedUrl = m_source.m_baseUrl;

        // If the URL doesn't already have query parameters for WMS, construct them
        if (m_source.m_baseUrl.find("GetMap") == AZStd::string::npos &&
            m_source.m_baseUrl.find("getmap") == AZStd::string::npos)
        {
            char separator = (m_source.m_baseUrl.find('?') != AZStd::string::npos) ? '&' : '?';
            constructedUrl += AZStd::string::format(
                "%cSERVICE=WMS&REQUEST=GetMap&VERSION=%s&LAYERS=%s&SRS=%s&FORMAT=%s&STYLES=%s&WIDTH=%u&HEIGHT=%u",
                separator,
                m_source.m_version.c_str(),
                m_source.m_layers.c_str(),
                m_source.m_srs.c_str(),
                m_source.m_format.c_str(),
                m_source.m_styles.c_str(),
                m_source.m_tileWidth,
                m_source.m_tileHeight);
        }

        // Use TMS overlay to handle tile fetching
        Cesium3DTilesSelection::TileMapServiceRasterOverlayOptions options{};
        if (m_source.m_maximumLevel > m_source.m_minimumLevel)
        {
            options.minimumLevel = m_source.m_minimumLevel;
            options.maximumLevel = m_source.m_maximumLevel;
        }

        // Determine file extension from format
        AZStd::string ext = "png";
        if (m_source.m_format.find("jpeg") != AZStd::string::npos || m_source.m_format.find("jpg") != AZStd::string::npos)
        {
            ext = "jpg";
        }
        options.fileExtension = ext.c_str();

        std::vector<CesiumAsync::IAssetAccessor::THeader> headers;
        for (const auto& header : m_source.m_headers)
        {
            headers.emplace_back(header.first.c_str(), header.second.c_str());
        }

        return std::make_unique<Cesium3DTilesSelection::TileMapServiceRasterOverlay>(
            "WMSRasterOverlay", constructedUrl.c_str(), headers, options);
    }
} // namespace Cesium
