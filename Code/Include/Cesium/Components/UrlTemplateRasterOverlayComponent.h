#pragma once

#include <Cesium/Components/RasterOverlayComponent.h>
#include <AzCore/std/string/string.h>
#include <AzCore/std/containers/map.h>
#include <cstdint>
#include <memory>

namespace Cesium
{
    struct UrlTemplateRasterOverlaySource final
    {
        AZ_RTTI(UrlTemplateRasterOverlaySource, "{5F7A8B9C-3D4E-6F8A-0B1C-5D6E7F8A9B0C}");
        AZ_CLASS_ALLOCATOR(UrlTemplateRasterOverlaySource, AZ::SystemAllocator, 0);

        static void Reflect(AZ::ReflectContext* context);

        UrlTemplateRasterOverlaySource();

        //!< URL template with placeholders: {x}, {y}, {z}, {reverseY}
        //!< Example: "https://example.com/tiles/{z}/{x}/{y}.png"
        AZStd::string m_urlTemplate;
        AZStd::unordered_map<AZStd::string, AZStd::string> m_headers;
        AZStd::string m_fileExtension;
        std::uint32_t m_minimumLevel;
        std::uint32_t m_maximumLevel;
    };

    class UrlTemplateRasterOverlayComponent : public RasterOverlayComponent
    {
    public:
        AZ_COMPONENT(UrlTemplateRasterOverlayComponent, "{6A8B9C0D-4E5F-7A8B-1C2D-6E7F8A9B0C1D}", RasterOverlayComponent)

        static void Reflect(AZ::ReflectContext* context);

        void LoadRasterOverlay(const UrlTemplateRasterOverlaySource& source);

    private:
        std::unique_ptr<Cesium3DTilesSelection::RasterOverlay> LoadRasterOverlayImpl() override;

        UrlTemplateRasterOverlaySource m_source;
    };
} // namespace Cesium
