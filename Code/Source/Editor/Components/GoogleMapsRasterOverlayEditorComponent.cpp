#include "Editor/Components/GoogleMapsRasterOverlayEditorComponent.h"
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>

namespace Cesium
{
    GoogleMapsRasterOverlayEditorComponent::GoogleMapsRasterOverlayEditorComponent() = default;

    void GoogleMapsRasterOverlayEditorComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<GoogleMapsRasterOverlayEditorComponent, AZ::Component>()
                ->Version(0)
                ->Field("Configuration", &GoogleMapsRasterOverlayEditorComponent::m_configuration)
                ->Field("Source", &GoogleMapsRasterOverlayEditorComponent::m_source);

            if (auto* editContext = serializeContext->GetEditContext())
            {
                editContext
                    ->Class<GoogleMapsRasterOverlayEditorComponent>(
                        "Google Maps Raster Overlay", "Drapes Google Maps tile imagery on 3D Tiles")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "Cesium")
                    ->Attribute(AZ::Edit::Attributes::Icon, "Editor/Icons/Components/Cesium_logo_only.svg")
                    ->Attribute(AZ::Edit::Attributes::ViewportIcon, "Editor/Icons/Components/Cesium_logo_only.svg")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game", 0x232b318c))
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &GoogleMapsRasterOverlayEditorComponent::m_configuration, "Configuration", "")
                    ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &GoogleMapsRasterOverlayEditorComponent::OnConfigurationChanged)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &GoogleMapsRasterOverlayEditorComponent::m_source, "Source", "")
                    ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &GoogleMapsRasterOverlayEditorComponent::OnSourceChanged);

                editContext->Class<GoogleMapsRasterOverlaySource>("Source", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->ClassElement(AZ::Edit::ClassElements::Group, "Source")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &GoogleMapsRasterOverlaySource::m_apiKey, "API Key", "Google Maps API key")
                    ->DataElement(AZ::Edit::UIHandlers::ComboBox, &GoogleMapsRasterOverlaySource::m_mapType, "Map Type", "Map imagery style")
                        ->EnumAttribute(GoogleMapsMapType::Satellite, "Satellite")
                        ->EnumAttribute(GoogleMapsMapType::Roadmap, "Roadmap")
                        ->EnumAttribute(GoogleMapsMapType::Terrain, "Terrain")
                        ->EnumAttribute(GoogleMapsMapType::Hybrid, "Hybrid")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &GoogleMapsRasterOverlaySource::m_language, "Language", "e.g., en, zh-CN")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &GoogleMapsRasterOverlaySource::m_region, "Region", "e.g., us, cn")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &GoogleMapsRasterOverlaySource::m_minimumLevel, "Minimum Level", "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &GoogleMapsRasterOverlaySource::m_maximumLevel, "Maximum Level", "");
            }
        }
    }

    void GoogleMapsRasterOverlayEditorComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    { provided.push_back(AZ_CRC_CE("GoogleMapsRasterOverlayEditorService")); }

    void GoogleMapsRasterOverlayEditorComponent::GetIncompatibleServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& incompatible) {}

    void GoogleMapsRasterOverlayEditorComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    { required.push_back(AZ_CRC_CE("3DTilesEditorService")); }

    void GoogleMapsRasterOverlayEditorComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    { dependent.push_back(AZ_CRC_CE("3DTilesEditorService")); }

    void GoogleMapsRasterOverlayEditorComponent::BuildGameEntity(AZ::Entity* gameEntity)
    {
        auto* c = gameEntity->CreateComponent<GoogleMapsRasterOverlayComponent>();
        c->SetEntity(gameEntity); c->Init(); c->Activate();
        c->SetConfiguration(m_configuration); c->LoadRasterOverlay(m_source);
    }

    void GoogleMapsRasterOverlayEditorComponent::Init()
    {
        AzToolsFramework::Components::EditorComponentBase::Init();
        if (!m_rasterOverlayComponent) { m_rasterOverlayComponent = AZStd::make_unique<GoogleMapsRasterOverlayComponent>(); }
    }

    void GoogleMapsRasterOverlayEditorComponent::Activate()
    {
        m_rasterOverlayComponent->SetEntity(GetEntity()); m_rasterOverlayComponent->Init(); m_rasterOverlayComponent->Activate();
        m_rasterOverlayComponent->SetConfiguration(m_configuration); m_rasterOverlayComponent->LoadRasterOverlay(m_source);
        m_rasterOverlayComponent->Deactivate();
    }

    void GoogleMapsRasterOverlayEditorComponent::Deactivate()
    { m_rasterOverlayComponent->Deactivate(); m_rasterOverlayComponent->SetEntity(nullptr); }

    AZ::u32 GoogleMapsRasterOverlayEditorComponent::OnSourceChanged()
    { if (m_rasterOverlayComponent) m_rasterOverlayComponent->LoadRasterOverlay(m_source); return AZ::Edit::PropertyRefreshLevels::None; }

    AZ::u32 GoogleMapsRasterOverlayEditorComponent::OnConfigurationChanged()
    { if (m_rasterOverlayComponent) m_rasterOverlayComponent->SetConfiguration(m_configuration); return AZ::Edit::PropertyRefreshLevels::None; }
} // namespace Cesium
