#include <Cesium/Components/UrlTemplateRasterOverlayComponent.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <Cesium3DTilesSelection/RasterOverlay.h>
#include <Cesium3DTilesSelection/TileMapServiceRasterOverlay.h>

namespace Cesium
{
    void UrlTemplateRasterOverlaySource::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<UrlTemplateRasterOverlaySource>()
                ->Version(0)
                ->Field("UrlTemplate", &UrlTemplateRasterOverlaySource::m_urlTemplate)
                ->Field("Headers", &UrlTemplateRasterOverlaySource::m_headers)
                ->Field("FileExtension", &UrlTemplateRasterOverlaySource::m_fileExtension)
                ->Field("MinimumLevel", &UrlTemplateRasterOverlaySource::m_minimumLevel)
                ->Field("MaximumLevel", &UrlTemplateRasterOverlaySource::m_maximumLevel);
        }

        if (auto* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Class<UrlTemplateRasterOverlaySource>("UrlTemplateRasterOverlaySource")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/RasterOverlays")
                ->Property("UrlTemplate", BehaviorValueProperty(&UrlTemplateRasterOverlaySource::m_urlTemplate))
                ->Property("FileExtension", BehaviorValueProperty(&UrlTemplateRasterOverlaySource::m_fileExtension))
                ->Property("MinimumLevel", BehaviorValueProperty(&UrlTemplateRasterOverlaySource::m_minimumLevel))
                ->Property("MaximumLevel", BehaviorValueProperty(&UrlTemplateRasterOverlaySource::m_maximumLevel));
        }
    }

    UrlTemplateRasterOverlaySource::UrlTemplateRasterOverlaySource()
        : m_fileExtension{ "png" }
        , m_minimumLevel{ 0 }
        , m_maximumLevel{ 25 }
    {
    }

    void UrlTemplateRasterOverlayComponent::Reflect(AZ::ReflectContext* context)
    {
        UrlTemplateRasterOverlaySource::Reflect(context);

        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<UrlTemplateRasterOverlayComponent, AZ::Component, RasterOverlayComponent>()
                ->Version(0)
                ->Field("source", &UrlTemplateRasterOverlayComponent::m_source);
        }

        if (auto* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Class<UrlTemplateRasterOverlayComponent>("UrlTemplateRasterOverlayComponent")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/RasterOverlays")
                ->Method(
                    "SetConfiguration",
                    [](UrlTemplateRasterOverlayComponent& c, const RasterOverlayConfiguration& cfg) { c.SetConfiguration(cfg); })
                ->Method(
                    "GetConfiguration",
                    [](const UrlTemplateRasterOverlayComponent& c) { return c.GetConfiguration(); })
                ->Method("LoadRasterOverlay", &UrlTemplateRasterOverlayComponent::LoadRasterOverlay);
        }
    }

    void UrlTemplateRasterOverlayComponent::LoadRasterOverlay(const UrlTemplateRasterOverlaySource& source)
    {
        m_source = source;
        RasterOverlayComponent::LoadRasterOverlay();
    }

    std::unique_ptr<Cesium3DTilesSelection::RasterOverlay> UrlTemplateRasterOverlayComponent::LoadRasterOverlayImpl()
    {
        // Parse the URL template to extract a TMS-compatible base URL.
        // Standard XYZ/TMS URL template: "https://example.com/tiles/{z}/{x}/{y}.png"
        // We extract the base URL by removing the /{z}/{x}/{y}.ext portion.
        AZStd::string url = m_source.m_urlTemplate;
        AZStd::string ext = m_source.m_fileExtension;

        // Try to extract base URL from template by finding {z} placeholder
        auto zPos = url.find("{z}");
        if (zPos != AZStd::string::npos)
        {
            // Remove everything from {z} onwards, trim trailing slash
            AZStd::string baseUrl = url.substr(0, zPos);
            while (!baseUrl.empty() && baseUrl.back() == '/')
            {
                baseUrl.pop_back();
            }
            url = baseUrl;

            // Try to extract file extension from the template
            auto dotPos = m_source.m_urlTemplate.rfind('.');
            auto lastBrace = m_source.m_urlTemplate.rfind('}');
            if (dotPos != AZStd::string::npos && dotPos > lastBrace)
            {
                // Extract extension after the last placeholder
                AZStd::string templateExt = m_source.m_urlTemplate.substr(dotPos + 1);
                // Remove any query parameters
                auto queryPos = templateExt.find('?');
                if (queryPos != AZStd::string::npos)
                {
                    templateExt = templateExt.substr(0, queryPos);
                }
                if (!templateExt.empty())
                {
                    ext = templateExt;
                }
            }
        }
        // If no {z} found, use the URL as-is (assume it's already a TMS base URL)

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
            "UrlTemplateRasterOverlay", url.c_str(), headers, options);
    }
} // namespace Cesium
