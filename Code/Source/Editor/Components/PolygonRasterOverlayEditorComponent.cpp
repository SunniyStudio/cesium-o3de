#include "Editor/Components/PolygonRasterOverlayEditorComponent.h"
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>

namespace Cesium
{
    PolygonRasterOverlayEditorComponent::PolygonRasterOverlayEditorComponent() = default;

    void PolygonRasterOverlayEditorComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<PolygonRasterOverlayEditorComponent, AZ::Component>()
                ->Version(0)
                ->Field("Configuration", &PolygonRasterOverlayEditorComponent::m_configuration)
                ->Field("Source", &PolygonRasterOverlayEditorComponent::m_source);

            if (auto* editContext = serializeContext->GetEditContext())
            {
                editContext
                    ->Class<PolygonRasterOverlayEditorComponent>(
                        "Polygon Raster Overlay",
                        "Rasterizes cartographic polygons onto 3D Tiles for masking or highlighting regions.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "Cesium")
                    ->Attribute(AZ::Edit::Attributes::Icon, "Editor/Icons/Components/Cesium_logo_only.svg")
                    ->Attribute(AZ::Edit::Attributes::ViewportIcon, "Editor/Icons/Components/Cesium_logo_only.svg")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game", 0x232b318c))
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default, &PolygonRasterOverlayEditorComponent::m_configuration, "Configuration", "")
                    ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &PolygonRasterOverlayEditorComponent::OnConfigurationChanged)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &PolygonRasterOverlayEditorComponent::m_source, "Source", "")
                    ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &PolygonRasterOverlayEditorComponent::OnSourceChanged);

                editContext->Class<CartographicPolygon>("CartographicPolygon", "A polygon defined by lon/lat points (degrees)")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default, &CartographicPolygon::m_points,
                        "Points", "Polygon vertices as (longitude, latitude) in degrees");

                editContext->Class<PolygonRasterOverlaySource>("Source", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->ClassElement(AZ::Edit::ClassElements::Group, "Source")
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default, &PolygonRasterOverlaySource::m_polygons,
                        "Polygons", "List of cartographic polygons to rasterize")
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default, &PolygonRasterOverlaySource::m_invertSelection,
                        "Invert Selection", "If true, mask inside polygons; if false, mask outside")
                    ->ClassElement(AZ::Edit::ClassElements::Group, "Fill Color")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &PolygonRasterOverlaySource::m_fillColorR, "R", "Red (0-255)")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &PolygonRasterOverlaySource::m_fillColorG, "G", "Green (0-255)")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &PolygonRasterOverlaySource::m_fillColorB, "B", "Blue (0-255)")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &PolygonRasterOverlaySource::m_fillColorA, "A", "Alpha (0-255)")
                    ->ClassElement(AZ::Edit::ClassElements::Group, "Background Color")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &PolygonRasterOverlaySource::m_bgColorR, "R", "Red (0-255)")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &PolygonRasterOverlaySource::m_bgColorG, "G", "Green (0-255)")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &PolygonRasterOverlaySource::m_bgColorB, "B", "Blue (0-255)")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &PolygonRasterOverlaySource::m_bgColorA, "A", "Alpha (0-255)");
            }
        }
    }

    void PolygonRasterOverlayEditorComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    { provided.push_back(AZ_CRC_CE("PolygonRasterOverlayEditorService")); }

    void PolygonRasterOverlayEditorComponent::GetIncompatibleServices(
        [[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& incompatible) {}

    void PolygonRasterOverlayEditorComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    { required.push_back(AZ_CRC_CE("3DTilesEditorService")); }

    void PolygonRasterOverlayEditorComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    { dependent.push_back(AZ_CRC_CE("3DTilesEditorService")); }

    void PolygonRasterOverlayEditorComponent::BuildGameEntity(AZ::Entity* gameEntity)
    {
        auto* c = gameEntity->CreateComponent<PolygonRasterOverlayComponent>();
        c->SetEntity(gameEntity); c->Init(); c->Activate();
        c->SetConfiguration(m_configuration); c->LoadRasterOverlay(m_source);
    }

    void PolygonRasterOverlayEditorComponent::Init()
    {
        AzToolsFramework::Components::EditorComponentBase::Init();
        if (!m_rasterOverlayComponent) m_rasterOverlayComponent = AZStd::make_unique<PolygonRasterOverlayComponent>();
    }

    void PolygonRasterOverlayEditorComponent::Activate()
    {
        m_rasterOverlayComponent->SetEntity(GetEntity()); m_rasterOverlayComponent->Init(); m_rasterOverlayComponent->Activate();
        m_rasterOverlayComponent->SetConfiguration(m_configuration); m_rasterOverlayComponent->LoadRasterOverlay(m_source);
        m_rasterOverlayComponent->Deactivate();
    }

    void PolygonRasterOverlayEditorComponent::Deactivate()
    { m_rasterOverlayComponent->Deactivate(); m_rasterOverlayComponent->SetEntity(nullptr); }

    AZ::u32 PolygonRasterOverlayEditorComponent::OnSourceChanged()
    { if (m_rasterOverlayComponent) m_rasterOverlayComponent->LoadRasterOverlay(m_source); return AZ::Edit::PropertyRefreshLevels::None; }

    AZ::u32 PolygonRasterOverlayEditorComponent::OnConfigurationChanged()
    { if (m_rasterOverlayComponent) m_rasterOverlayComponent->SetConfiguration(m_configuration); return AZ::Edit::PropertyRefreshLevels::None; }
} // namespace Cesium
