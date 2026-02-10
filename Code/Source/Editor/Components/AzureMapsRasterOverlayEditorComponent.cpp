#include "Editor/Components/AzureMapsRasterOverlayEditorComponent.h"
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>

namespace Cesium
{
    AzureMapsRasterOverlayEditorComponent::AzureMapsRasterOverlayEditorComponent() = default;

    void AzureMapsRasterOverlayEditorComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<AzureMapsRasterOverlayEditorComponent, AZ::Component>()
                ->Version(0)
                ->Field("Configuration", &AzureMapsRasterOverlayEditorComponent::m_configuration)
                ->Field("Source", &AzureMapsRasterOverlayEditorComponent::m_source);

            if (auto* editContext = serializeContext->GetEditContext())
            {
                editContext
                    ->Class<AzureMapsRasterOverlayEditorComponent>(
                        "Azure Maps Raster Overlay", "Drapes Azure Maps tile imagery on 3D Tiles")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "Cesium")
                    ->Attribute(AZ::Edit::Attributes::Icon, "Editor/Icons/Components/Cesium_logo_only.svg")
                    ->Attribute(AZ::Edit::Attributes::ViewportIcon, "Editor/Icons/Components/Cesium_logo_only.svg")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game", 0x232b318c))
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &AzureMapsRasterOverlayEditorComponent::m_configuration, "Configuration", "")
                    ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &AzureMapsRasterOverlayEditorComponent::OnConfigurationChanged)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &AzureMapsRasterOverlayEditorComponent::m_source, "Source", "")
                    ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &AzureMapsRasterOverlayEditorComponent::OnSourceChanged);

                editContext->Class<AzureMapsRasterOverlaySource>("Source", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->ClassElement(AZ::Edit::ClassElements::Group, "Source")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &AzureMapsRasterOverlaySource::m_subscriptionKey,
                        "Subscription Key", "Azure Maps subscription key")
                    ->DataElement(AZ::Edit::UIHandlers::ComboBox, &AzureMapsRasterOverlaySource::m_mapStyle, "Map Style", "Map imagery style")
                        ->EnumAttribute(AzureMapsMapStyle::RoadLight, "Road (Light)")
                        ->EnumAttribute(AzureMapsMapStyle::RoadDark, "Road (Dark)")
                        ->EnumAttribute(AzureMapsMapStyle::Satellite, "Satellite")
                        ->EnumAttribute(AzureMapsMapStyle::SatelliteRoadLabels, "Satellite + Road Labels")
                        ->EnumAttribute(AzureMapsMapStyle::GrayscaleLight, "Grayscale (Light)")
                        ->EnumAttribute(AzureMapsMapStyle::GrayscaleDark, "Grayscale (Dark)")
                        ->EnumAttribute(AzureMapsMapStyle::NightView, "Night View")
                        ->EnumAttribute(AzureMapsMapStyle::HighContrastLight, "High Contrast (Light)")
                        ->EnumAttribute(AzureMapsMapStyle::HighContrastDark, "High Contrast (Dark)")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &AzureMapsRasterOverlaySource::m_language,
                        "Language", "e.g., en-US, zh-Hans-CN")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &AzureMapsRasterOverlaySource::m_tileSize,
                        "Tile Size", "256 or 512")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &AzureMapsRasterOverlaySource::m_minimumLevel, "Minimum Level", "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &AzureMapsRasterOverlaySource::m_maximumLevel, "Maximum Level", "");
            }
        }
    }

    void AzureMapsRasterOverlayEditorComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    { provided.push_back(AZ_CRC_CE("AzureMapsRasterOverlayEditorService")); }

    void AzureMapsRasterOverlayEditorComponent::GetIncompatibleServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& incompatible) {}

    void AzureMapsRasterOverlayEditorComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    { required.push_back(AZ_CRC_CE("3DTilesEditorService")); }

    void AzureMapsRasterOverlayEditorComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    { dependent.push_back(AZ_CRC_CE("3DTilesEditorService")); }

    void AzureMapsRasterOverlayEditorComponent::BuildGameEntity(AZ::Entity* gameEntity)
    {
        auto* c = gameEntity->CreateComponent<AzureMapsRasterOverlayComponent>();
        c->SetEntity(gameEntity); c->Init(); c->Activate();
        c->SetConfiguration(m_configuration); c->LoadRasterOverlay(m_source);
    }

    void AzureMapsRasterOverlayEditorComponent::Init()
    {
        AzToolsFramework::Components::EditorComponentBase::Init();
        if (!m_rasterOverlayComponent) { m_rasterOverlayComponent = AZStd::make_unique<AzureMapsRasterOverlayComponent>(); }
    }

    void AzureMapsRasterOverlayEditorComponent::Activate()
    {
        m_rasterOverlayComponent->SetEntity(GetEntity()); m_rasterOverlayComponent->Init(); m_rasterOverlayComponent->Activate();
        m_rasterOverlayComponent->SetConfiguration(m_configuration); m_rasterOverlayComponent->LoadRasterOverlay(m_source);
        m_rasterOverlayComponent->Deactivate();
    }

    void AzureMapsRasterOverlayEditorComponent::Deactivate()
    { m_rasterOverlayComponent->Deactivate(); m_rasterOverlayComponent->SetEntity(nullptr); }

    AZ::u32 AzureMapsRasterOverlayEditorComponent::OnSourceChanged()
    { if (m_rasterOverlayComponent) m_rasterOverlayComponent->LoadRasterOverlay(m_source); return AZ::Edit::PropertyRefreshLevels::None; }

    AZ::u32 AzureMapsRasterOverlayEditorComponent::OnConfigurationChanged()
    { if (m_rasterOverlayComponent) m_rasterOverlayComponent->SetConfiguration(m_configuration); return AZ::Edit::PropertyRefreshLevels::None; }
} // namespace Cesium
