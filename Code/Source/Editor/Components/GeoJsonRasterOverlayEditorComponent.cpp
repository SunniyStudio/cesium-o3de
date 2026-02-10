#include "Editor/Components/GeoJsonRasterOverlayEditorComponent.h"
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>

namespace Cesium
{
    GeoJsonRasterOverlayEditorComponent::GeoJsonRasterOverlayEditorComponent() = default;

    void GeoJsonRasterOverlayEditorComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<GeoJsonRasterOverlayEditorComponent, AZ::Component>()
                ->Version(0)
                ->Field("Configuration", &GeoJsonRasterOverlayEditorComponent::m_configuration)
                ->Field("Source", &GeoJsonRasterOverlayEditorComponent::m_source);

            if (auto* editContext = serializeContext->GetEditContext())
            {
                editContext
                    ->Class<GeoJsonRasterOverlayEditorComponent>(
                        "GeoJSON Raster Overlay",
                        "Rasterizes GeoJSON features (points, lines, polygons) onto 3D Tiles.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "Cesium")
                    ->Attribute(AZ::Edit::Attributes::Icon, "Editor/Icons/Components/Cesium_logo_only.svg")
                    ->Attribute(AZ::Edit::Attributes::ViewportIcon, "Editor/Icons/Components/Cesium_logo_only.svg")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game", 0x232b318c))
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &GeoJsonRasterOverlayEditorComponent::m_configuration, "Configuration", "")
                    ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &GeoJsonRasterOverlayEditorComponent::OnConfigurationChanged)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &GeoJsonRasterOverlayEditorComponent::m_source, "Source", "")
                    ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &GeoJsonRasterOverlayEditorComponent::OnSourceChanged);

                editContext->Class<GeoJsonRasterOverlaySource>("Source", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->ClassElement(AZ::Edit::ClassElements::Group, "Source")
                    ->DataElement(AZ::Edit::UIHandlers::ComboBox, &GeoJsonRasterOverlaySource::m_sourceType, "Source Type", "")
                        ->EnumAttribute(GeoJsonSourceType::Url, "URL")
                        ->EnumAttribute(GeoJsonSourceType::Inline, "Inline GeoJSON")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &GeoJsonRasterOverlaySource::m_url,
                        "URL", "URL to a .geojson or .json file")
                    ->DataElement(AZ::Edit::UIHandlers::MultiLineEdit, &GeoJsonRasterOverlaySource::m_inlineGeoJson,
                        "Inline GeoJSON", "Paste GeoJSON content directly")
                    ->ClassElement(AZ::Edit::ClassElements::Group, "Stroke Color")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &GeoJsonRasterOverlaySource::m_strokeR, "R", "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &GeoJsonRasterOverlaySource::m_strokeG, "G", "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &GeoJsonRasterOverlaySource::m_strokeB, "B", "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &GeoJsonRasterOverlaySource::m_strokeA, "A", "")
                    ->ClassElement(AZ::Edit::ClassElements::Group, "Fill Color")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &GeoJsonRasterOverlaySource::m_fillR, "R", "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &GeoJsonRasterOverlaySource::m_fillG, "G", "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &GeoJsonRasterOverlaySource::m_fillB, "B", "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &GeoJsonRasterOverlaySource::m_fillA, "A", "")
                    ->ClassElement(AZ::Edit::ClassElements::Group, "Options")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &GeoJsonRasterOverlaySource::m_strokeWidth, "Stroke Width", "Line width in pixels")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &GeoJsonRasterOverlaySource::m_minimumLevel, "Minimum Level", "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &GeoJsonRasterOverlaySource::m_maximumLevel, "Maximum Level", "");
            }
        }
    }

    void GeoJsonRasterOverlayEditorComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    { provided.push_back(AZ_CRC_CE("GeoJsonRasterOverlayEditorService")); }

    void GeoJsonRasterOverlayEditorComponent::GetIncompatibleServices(
        [[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& incompatible) {}

    void GeoJsonRasterOverlayEditorComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    { required.push_back(AZ_CRC_CE("3DTilesEditorService")); }

    void GeoJsonRasterOverlayEditorComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    { dependent.push_back(AZ_CRC_CE("3DTilesEditorService")); }

    void GeoJsonRasterOverlayEditorComponent::BuildGameEntity(AZ::Entity* gameEntity)
    {
        auto* c = gameEntity->CreateComponent<GeoJsonRasterOverlayComponent>();
        c->SetEntity(gameEntity); c->Init(); c->Activate();
        c->SetConfiguration(m_configuration); c->LoadRasterOverlay(m_source);
    }

    void GeoJsonRasterOverlayEditorComponent::Init()
    {
        AzToolsFramework::Components::EditorComponentBase::Init();
        if (!m_rasterOverlayComponent) m_rasterOverlayComponent = AZStd::make_unique<GeoJsonRasterOverlayComponent>();
    }

    void GeoJsonRasterOverlayEditorComponent::Activate()
    {
        m_rasterOverlayComponent->SetEntity(GetEntity()); m_rasterOverlayComponent->Init(); m_rasterOverlayComponent->Activate();
        m_rasterOverlayComponent->SetConfiguration(m_configuration); m_rasterOverlayComponent->LoadRasterOverlay(m_source);
        m_rasterOverlayComponent->Deactivate();
    }

    void GeoJsonRasterOverlayEditorComponent::Deactivate()
    { m_rasterOverlayComponent->Deactivate(); m_rasterOverlayComponent->SetEntity(nullptr); }

    AZ::u32 GeoJsonRasterOverlayEditorComponent::OnSourceChanged()
    { if (m_rasterOverlayComponent) m_rasterOverlayComponent->LoadRasterOverlay(m_source); return AZ::Edit::PropertyRefreshLevels::None; }

    AZ::u32 GeoJsonRasterOverlayEditorComponent::OnConfigurationChanged()
    { if (m_rasterOverlayComponent) m_rasterOverlayComponent->SetConfiguration(m_configuration); return AZ::Edit::PropertyRefreshLevels::None; }
} // namespace Cesium
