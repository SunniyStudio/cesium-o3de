#include "Editor/Components/WMTSRasterOverlayEditorComponent.h"
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>

namespace Cesium
{
    WMTSRasterOverlayEditorComponent::WMTSRasterOverlayEditorComponent() = default;

    void WMTSRasterOverlayEditorComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<WMTSRasterOverlayEditorComponent, AZ::Component>()
                ->Version(0)
                ->Field("Configuration", &WMTSRasterOverlayEditorComponent::m_configuration)
                ->Field("Source", &WMTSRasterOverlayEditorComponent::m_source);

            if (auto* editContext = serializeContext->GetEditContext())
            {
                editContext
                    ->Class<WMTSRasterOverlayEditorComponent>(
                        "WMTS Raster Overlay", "Drapes Web Map Tile Service (WMTS) imagery on 3D Tiles")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "Cesium")
                    ->Attribute(AZ::Edit::Attributes::Icon, "Editor/Icons/Components/Cesium_logo_only.svg")
                    ->Attribute(AZ::Edit::Attributes::ViewportIcon, "Editor/Icons/Components/Cesium_logo_only.svg")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game", 0x232b318c))
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &WMTSRasterOverlayEditorComponent::m_configuration, "Configuration", "")
                    ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &WMTSRasterOverlayEditorComponent::OnConfigurationChanged)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &WMTSRasterOverlayEditorComponent::m_source, "Source", "")
                    ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &WMTSRasterOverlayEditorComponent::OnSourceChanged);

                editContext->Class<WMTSRasterOverlaySource>("Source", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->ClassElement(AZ::Edit::ClassElements::Group, "Source")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &WMTSRasterOverlaySource::m_baseUrl, "Base URL", "WMTS service endpoint URL")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &WMTSRasterOverlaySource::m_layer, "Layer", "Layer identifier")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &WMTSRasterOverlaySource::m_style, "Style", "Style identifier")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &WMTSRasterOverlaySource::m_tileMatrixSet, "Tile Matrix Set", "e.g., GoogleMapsCompatible")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &WMTSRasterOverlaySource::m_format, "Format", "Image format (e.g., image/png)")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &WMTSRasterOverlaySource::m_headers, "Request Headers", "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &WMTSRasterOverlaySource::m_minimumLevel, "Minimum Level", "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &WMTSRasterOverlaySource::m_maximumLevel, "Maximum Level", "");
            }
        }
    }

    void WMTSRasterOverlayEditorComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    { provided.push_back(AZ_CRC_CE("WMTSRasterOverlayEditorService")); }

    void WMTSRasterOverlayEditorComponent::GetIncompatibleServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& incompatible) {}

    void WMTSRasterOverlayEditorComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    { required.push_back(AZ_CRC_CE("3DTilesEditorService")); }

    void WMTSRasterOverlayEditorComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    { dependent.push_back(AZ_CRC_CE("3DTilesEditorService")); }

    void WMTSRasterOverlayEditorComponent::BuildGameEntity(AZ::Entity* gameEntity)
    {
        auto* c = gameEntity->CreateComponent<WMTSRasterOverlayComponent>();
        c->SetEntity(gameEntity); c->Init(); c->Activate();
        c->SetConfiguration(m_configuration); c->LoadRasterOverlay(m_source);
    }

    void WMTSRasterOverlayEditorComponent::Init()
    {
        AzToolsFramework::Components::EditorComponentBase::Init();
        if (!m_rasterOverlayComponent) { m_rasterOverlayComponent = AZStd::make_unique<WMTSRasterOverlayComponent>(); }
    }

    void WMTSRasterOverlayEditorComponent::Activate()
    {
        m_rasterOverlayComponent->SetEntity(GetEntity()); m_rasterOverlayComponent->Init(); m_rasterOverlayComponent->Activate();
        m_rasterOverlayComponent->SetConfiguration(m_configuration); m_rasterOverlayComponent->LoadRasterOverlay(m_source);
        m_rasterOverlayComponent->Deactivate();
    }

    void WMTSRasterOverlayEditorComponent::Deactivate()
    { m_rasterOverlayComponent->Deactivate(); m_rasterOverlayComponent->SetEntity(nullptr); }

    AZ::u32 WMTSRasterOverlayEditorComponent::OnSourceChanged()
    { if (m_rasterOverlayComponent) m_rasterOverlayComponent->LoadRasterOverlay(m_source); return AZ::Edit::PropertyRefreshLevels::None; }

    AZ::u32 WMTSRasterOverlayEditorComponent::OnConfigurationChanged()
    { if (m_rasterOverlayComponent) m_rasterOverlayComponent->SetConfiguration(m_configuration); return AZ::Edit::PropertyRefreshLevels::None; }
} // namespace Cesium
