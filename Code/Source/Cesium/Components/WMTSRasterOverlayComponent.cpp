#include <Cesium/Components/WMTSRasterOverlayComponent.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <Cesium3DTilesSelection/RasterOverlay.h>
#include <Cesium3DTilesSelection/TileMapServiceRasterOverlay.h>

namespace Cesium
{
    void WMTSRasterOverlaySource::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<WMTSRasterOverlaySource>()
                ->Version(0)
                ->Field("BaseUrl", &WMTSRasterOverlaySource::m_baseUrl)
                ->Field("Layer", &WMTSRasterOverlaySource::m_layer)
                ->Field("Style", &WMTSRasterOverlaySource::m_style)
                ->Field("TileMatrixSet", &WMTSRasterOverlaySource::m_tileMatrixSet)
                ->Field("Format", &WMTSRasterOverlaySource::m_format)
                ->Field("Headers", &WMTSRasterOverlaySource::m_headers)
                ->Field("MinimumLevel", &WMTSRasterOverlaySource::m_minimumLevel)
                ->Field("MaximumLevel", &WMTSRasterOverlaySource::m_maximumLevel);
        }

        if (auto* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Class<WMTSRasterOverlaySource>("WMTSRasterOverlaySource")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/RasterOverlays")
                ->Property("BaseUrl", BehaviorValueProperty(&WMTSRasterOverlaySource::m_baseUrl))
                ->Property("Layer", BehaviorValueProperty(&WMTSRasterOverlaySource::m_layer))
                ->Property("Style", BehaviorValueProperty(&WMTSRasterOverlaySource::m_style))
                ->Property("TileMatrixSet", BehaviorValueProperty(&WMTSRasterOverlaySource::m_tileMatrixSet))
                ->Property("Format", BehaviorValueProperty(&WMTSRasterOverlaySource::m_format))
                ->Property("MinimumLevel", BehaviorValueProperty(&WMTSRasterOverlaySource::m_minimumLevel))
                ->Property("MaximumLevel", BehaviorValueProperty(&WMTSRasterOverlaySource::m_maximumLevel));
        }
    }

    WMTSRasterOverlaySource::WMTSRasterOverlaySource()
        : m_style{ "default" }
        , m_tileMatrixSet{ "GoogleMapsCompatible" }
        , m_format{ "image/png" }
        , m_minimumLevel{ 0 }
        , m_maximumLevel{ 25 }
    {
    }

    void WMTSRasterOverlayComponent::Reflect(AZ::ReflectContext* context)
    {
        WMTSRasterOverlaySource::Reflect(context);

        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<WMTSRasterOverlayComponent, AZ::Component, RasterOverlayComponent>()
                ->Version(0)
                ->Field("source", &WMTSRasterOverlayComponent::m_source);
        }

        if (auto* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Class<WMTSRasterOverlayComponent>("WMTSRasterOverlayComponent")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/RasterOverlays")
                ->Method(
                    "SetConfiguration",
                    [](WMTSRasterOverlayComponent& c, const RasterOverlayConfiguration& cfg) { c.SetConfiguration(cfg); })
                ->Method(
                    "GetConfiguration",
                    [](const WMTSRasterOverlayComponent& c) { return c.GetConfiguration(); })
                ->Method("LoadRasterOverlay", &WMTSRasterOverlayComponent::LoadRasterOverlay);
        }
    }

    void WMTSRasterOverlayComponent::LoadRasterOverlay(const WMTSRasterOverlaySource& source)
    {
        m_source = source;
        RasterOverlayComponent::LoadRasterOverlay();
    }

    std::unique_ptr<Cesium3DTilesSelection::RasterOverlay> WMTSRasterOverlayComponent::LoadRasterOverlayImpl()
    {
        // Construct WMTS RESTful URL in TMS-compatible format.
        // Standard WMTS RESTful pattern:
        //   {baseUrl}/{layer}/{style}/{tileMatrixSet}/{z}/{y}/{x}.{format}
        // TMS expects the base URL to have the tile path appended as /{z}/{x}/{y}.{ext}
        AZStd::string constructedUrl = m_source.m_baseUrl;

        // Ensure trailing slash
        if (!constructedUrl.empty() && constructedUrl.back() != '/')
        {
            constructedUrl += '/';
        }

        // Build the WMTS RESTful path prefix
        if (!m_source.m_layer.empty())
        {
            constructedUrl += m_source.m_layer + '/';
        }
        if (!m_source.m_style.empty())
        {
            constructedUrl += m_source.m_style + '/';
        }
        if (!m_source.m_tileMatrixSet.empty())
        {
            constructedUrl += m_source.m_tileMatrixSet + '/';
        }

        // Determine file extension from format
        AZStd::string ext = "png";
        if (m_source.m_format.find("jpeg") != AZStd::string::npos || m_source.m_format.find("jpg") != AZStd::string::npos)
        {
            ext = "jpg";
        }
        else if (m_source.m_format.find("png") != AZStd::string::npos)
        {
            ext = "png";
        }

        Cesium3DTilesSelection::TileMapServiceRasterOverlayOptions options{};
        if (m_source.m_maximumLevel > m_source.m_minimumLevel)
        {
            options.minimumLevel = m_source.m_minimumLevel;
            options.maximumLevel = m_source.m_maximumLevel;
        }
        options.fileExtension = ext.c_str();

        std::vector<CesiumAsync::IAssetAccessor::THeader> headers;
        for (const auto& header : m_source.m_headers)
        {
            headers.emplace_back(header.first.c_str(), header.second.c_str());
        }

        return std::make_unique<Cesium3DTilesSelection::TileMapServiceRasterOverlay>(
            "WMTSRasterOverlay", constructedUrl.c_str(), headers, options);
    }
} // namespace Cesium
