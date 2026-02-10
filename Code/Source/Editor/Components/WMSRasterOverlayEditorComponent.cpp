#include "Editor/Components/WMSRasterOverlayEditorComponent.h"
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>

namespace Cesium
{
    WMSRasterOverlayEditorComponent::WMSRasterOverlayEditorComponent() = default;

    void WMSRasterOverlayEditorComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<WMSRasterOverlayEditorComponent, AZ::Component>()
                ->Version(0)
                ->Field("Configuration", &WMSRasterOverlayEditorComponent::m_configuration)
                ->Field("Source", &WMSRasterOverlayEditorComponent::m_source);

            if (auto* editContext = serializeContext->GetEditContext())
            {
                editContext
                    ->Class<WMSRasterOverlayEditorComponent>(
                        "WMS Raster Overlay", "Drapes Web Map Service (WMS) imagery on 3D Tiles")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "Cesium")
                    ->Attribute(AZ::Edit::Attributes::Icon, "Editor/Icons/Components/Cesium_logo_only.svg")
                    ->Attribute(AZ::Edit::Attributes::ViewportIcon, "Editor/Icons/Components/Cesium_logo_only.svg")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game", 0x232b318c))
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &WMSRasterOverlayEditorComponent::m_configuration, "Configuration", "")
                    ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &WMSRasterOverlayEditorComponent::OnConfigurationChanged)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &WMSRasterOverlayEditorComponent::m_source, "Source", "")
                    ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &WMSRasterOverlayEditorComponent::OnSourceChanged);

                editContext->Class<WMSRasterOverlaySource>("Source", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->ClassElement(AZ::Edit::ClassElements::Group, "Source")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &WMSRasterOverlaySource::m_baseUrl, "Base URL", "WMS service endpoint URL")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &WMSRasterOverlaySource::m_layers, "Layers", "Comma-separated layer names")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &WMSRasterOverlaySource::m_srs, "SRS", "Spatial reference system (e.g., EPSG:4326)")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &WMSRasterOverlaySource::m_format, "Format", "Image format (e.g., image/png)")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &WMSRasterOverlaySource::m_version, "Version", "WMS version (e.g., 1.1.1)")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &WMSRasterOverlaySource::m_styles, "Styles", "Comma-separated styles (empty = default)")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &WMSRasterOverlaySource::m_headers, "Request Headers", "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &WMSRasterOverlaySource::m_tileWidth, "Tile Width", "Tile width in pixels")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &WMSRasterOverlaySource::m_tileHeight, "Tile Height", "Tile height in pixels")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &WMSRasterOverlaySource::m_minimumLevel, "Minimum Level", "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &WMSRasterOverlaySource::m_maximumLevel, "Maximum Level", "");
            }
        }
    }

    void WMSRasterOverlayEditorComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    { provided.push_back(AZ_CRC_CE("WMSRasterOverlayEditorService")); }

    void WMSRasterOverlayEditorComponent::GetIncompatibleServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& incompatible) {}

    void WMSRasterOverlayEditorComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    { required.push_back(AZ_CRC_CE("3DTilesEditorService")); }

    void WMSRasterOverlayEditorComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    { dependent.push_back(AZ_CRC_CE("3DTilesEditorService")); }

    void WMSRasterOverlayEditorComponent::BuildGameEntity(AZ::Entity* gameEntity)
    {
        auto* c = gameEntity->CreateComponent<WMSRasterOverlayComponent>();
        c->SetEntity(gameEntity); c->Init(); c->Activate();
        c->SetConfiguration(m_configuration); c->LoadRasterOverlay(m_source);
    }

    void WMSRasterOverlayEditorComponent::Init()
    {
        AzToolsFramework::Components::EditorComponentBase::Init();
        if (!m_rasterOverlayComponent) { m_rasterOverlayComponent = AZStd::make_unique<WMSRasterOverlayComponent>(); }
    }

    void WMSRasterOverlayEditorComponent::Activate()
    {
        m_rasterOverlayComponent->SetEntity(GetEntity()); m_rasterOverlayComponent->Init(); m_rasterOverlayComponent->Activate();
        m_rasterOverlayComponent->SetConfiguration(m_configuration); m_rasterOverlayComponent->LoadRasterOverlay(m_source);
        m_rasterOverlayComponent->Deactivate();
    }

    void WMSRasterOverlayEditorComponent::Deactivate()
    { m_rasterOverlayComponent->Deactivate(); m_rasterOverlayComponent->SetEntity(nullptr); }

    AZ::u32 WMSRasterOverlayEditorComponent::OnSourceChanged()
    { if (m_rasterOverlayComponent) m_rasterOverlayComponent->LoadRasterOverlay(m_source); return AZ::Edit::PropertyRefreshLevels::None; }

    AZ::u32 WMSRasterOverlayEditorComponent::OnConfigurationChanged()
    { if (m_rasterOverlayComponent) m_rasterOverlayComponent->SetConfiguration(m_configuration); return AZ::Edit::PropertyRefreshLevels::None; }
} // namespace Cesium
