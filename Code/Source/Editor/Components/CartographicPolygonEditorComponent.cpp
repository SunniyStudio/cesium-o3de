#include "Editor/Components/CartographicPolygonEditorComponent.h"
#include <Cesium/Math/MathReflect.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>

namespace Cesium
{
    void CartographicPolygonEditorComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto* sc = azrtti_cast<AZ::SerializeContext*>(context))
        {
            sc->Class<CartographicPolygonEditorComponent, AZ::Component>()
                ->Version(0)
                ->Field("InputMode", &CartographicPolygonEditorComponent::m_inputMode)
                ->Field("LonLatPoints", &CartographicPolygonEditorComponent::m_lonLatPoints)
                ->Field("WorldPoints", &CartographicPolygonEditorComponent::m_worldPoints)
                ->Field("InvertSelection", &CartographicPolygonEditorComponent::m_invertSelection);

            if (auto* ec = sc->GetEditContext())
            {
                ec->Class<CartographicPolygonEditorComponent>(
                        "Cesium Cartographic Polygon",
                        "Defines a polygon region for tile clipping/exclusion. "
                        "Add a Polygon Prism Shape to this entity for visual editing.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "Cesium")
                    ->Attribute(AZ::Edit::Attributes::Icon, "Editor/Icons/Components/Cesium_logo_only.svg")
                    ->Attribute(AZ::Edit::Attributes::ViewportIcon, "Editor/Icons/Components/Cesium_logo_only.svg")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game", 0x232b318c))
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(AZ::Edit::UIHandlers::ComboBox, &CartographicPolygonEditorComponent::m_inputMode,
                        "Input Mode", "How polygon points are specified")
                        ->EnumAttribute(PolygonInputMode::ShapeComponent, "Polygon Prism Shape (recommended)")
                        ->EnumAttribute(PolygonInputMode::WorldCoordinates, "World Coordinates (O3DE)")
                        ->EnumAttribute(PolygonInputMode::LonLatDegrees, "Longitude/Latitude (Degrees)")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &CartographicPolygonEditorComponent::m_worldPoints,
                        "World Points", "Polygon vertices in O3DE world space")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &CartographicPolygonEditorComponent::m_lonLatPoints,
                        "Lon/Lat Points", "Polygon vertices as (longitude, latitude) in degrees")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &CartographicPolygonEditorComponent::m_invertSelection,
                        "Invert Selection", "If true, exclude tiles inside the polygon");
            }
        }
    }

    void CartographicPolygonEditorComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    { provided.push_back(AZ_CRC_CE("CesiumCartographicPolygonService")); }

    void CartographicPolygonEditorComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    { incompatible.push_back(AZ_CRC_CE("CesiumCartographicPolygonService")); }

    void CartographicPolygonEditorComponent::BuildGameEntity(AZ::Entity* gameEntity)
    {
        auto* c = gameEntity->CreateComponent<CartographicPolygonComponent>();
        c->SetEntity(gameEntity);
    }

    void CartographicPolygonEditorComponent::Init()
    {
        AzToolsFramework::Components::EditorComponentBase::Init();
        if (!m_runtimeComponent)
        {
            m_runtimeComponent = AZStd::make_unique<CartographicPolygonComponent>();
        }
    }

    void CartographicPolygonEditorComponent::Activate()
    {
        // Activate the runtime component so it responds to CartographicPolygonRequestBus in editor
        m_runtimeComponent->SetEntity(GetEntity());
        m_runtimeComponent->Init();
        m_runtimeComponent->Activate();
    }

    void CartographicPolygonEditorComponent::Deactivate()
    {
        m_runtimeComponent->Deactivate();
        m_runtimeComponent->SetEntity(nullptr);
    }
} // namespace Cesium
