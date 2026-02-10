#include <Cesium/Components/CartographicPolygonComponent.h>
#include <Cesium/Math/GeospatialHelper.h>
#include <Cesium/Math/MathReflect.h>
#include <Cesium/EBus/OriginShiftComponentBus.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/Component/TransformBus.h>
#include <LmbrCentral/Shape/PolygonPrismShapeComponentBus.h>
#include <glm/gtc/constants.hpp>

namespace Cesium
{
    void CartographicPolygonComponent::Reflect(AZ::ReflectContext* context)
    {
        CartographicPolygonRequest::Reflect(context);

        if (auto* sc = azrtti_cast<AZ::SerializeContext*>(context))
        {
            sc->Class<CartographicPolygonComponent, AZ::Component>()
                ->Version(1)
                ->Field("InputMode", &CartographicPolygonComponent::m_inputMode)
                ->Field("LonLatPoints", &CartographicPolygonComponent::m_lonLatPoints)
                ->Field("WorldPoints", &CartographicPolygonComponent::m_worldPoints)
                ->Field("InvertSelection", &CartographicPolygonComponent::m_invertSelection);

            if (auto* ec = sc->GetEditContext())
            {
                ec->Class<CartographicPolygonComponent>(
                        "Cesium Cartographic Polygon",
                        "Defines a polygon region for tile clipping/exclusion. "
                        "Supports direct lon/lat input or O3DE world coordinate input (auto-converted).")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "Cesium")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)

                    ->DataElement(
                        AZ::Edit::UIHandlers::ComboBox, &CartographicPolygonComponent::m_inputMode,
                        "Input Mode", "How polygon points are specified")
                        ->EnumAttribute(PolygonInputMode::ShapeComponent, "Polygon Prism Shape (recommended)")
                        ->EnumAttribute(PolygonInputMode::WorldCoordinates, "World Coordinates (O3DE)")
                        ->EnumAttribute(PolygonInputMode::LonLatDegrees, "Longitude/Latitude (Degrees)")

                    ->DataElement(
                        AZ::Edit::UIHandlers::Default, &CartographicPolygonComponent::m_worldPoints,
                        "World Points", "Polygon vertices in O3DE world space (auto-converted to lon/lat via georeference)")

                    ->DataElement(
                        AZ::Edit::UIHandlers::Default, &CartographicPolygonComponent::m_lonLatPoints,
                        "Lon/Lat Points", "Polygon vertices as (longitude, latitude) in degrees")

                    ->DataElement(
                        AZ::Edit::UIHandlers::Default, &CartographicPolygonComponent::m_invertSelection,
                        "Invert Selection", "If true, exclude tiles inside the polygon; if false, exclude outside");
            }
        }

        if (auto* bc = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            bc->Enum<static_cast<int>(PolygonInputMode::LonLatDegrees)>("PolygonInputMode_LonLatDegrees")
                ->Enum<static_cast<int>(PolygonInputMode::WorldCoordinates)>("PolygonInputMode_WorldCoordinates")
                ->Enum<static_cast<int>(PolygonInputMode::ShapeComponent)>("PolygonInputMode_ShapeComponent");

            bc->Class<CartographicPolygonComponent>("CartographicPolygonComponent")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/Polygon");
        }
    }

    void CartographicPolygonComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("CesiumCartographicPolygonService"));
    }

    void CartographicPolygonComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("CesiumCartographicPolygonService"));
    }

    void CartographicPolygonComponent::Activate()
    {
        CartographicPolygonRequestBus::Handler::BusConnect(GetEntityId());
    }

    void CartographicPolygonComponent::Deactivate()
    {
        CartographicPolygonRequestBus::Handler::BusDisconnect();
    }

    AZStd::vector<glm::dvec2> CartographicPolygonComponent::GetPolygonPointsDegrees() const
    {
        switch (m_inputMode)
        {
        case PolygonInputMode::WorldCoordinates:
            return ConvertWorldPointsToLonLatDegrees();
        case PolygonInputMode::ShapeComponent:
            return ReadShapeComponentPoints();
        case PolygonInputMode::LonLatDegrees:
        default:
            return m_lonLatPoints;
        }
    }

    AZStd::vector<glm::dvec2> CartographicPolygonComponent::GetPolygonPointsRadians() const
    {
        AZStd::vector<glm::dvec2> degrees = GetPolygonPointsDegrees();
        AZStd::vector<glm::dvec2> radians;
        radians.reserve(degrees.size());
        constexpr double degToRad = glm::pi<double>() / 180.0;
        for (const auto& pt : degrees)
        {
            radians.emplace_back(pt.x * degToRad, pt.y * degToRad);
        }
        return radians;
    }

    bool CartographicPolygonComponent::GetInvertSelection() const
    {
        return m_invertSelection;
    }

    AZStd::vector<glm::dvec2> CartographicPolygonComponent::ReadShapeComponentPoints() const
    {
        AZStd::vector<glm::dvec2> result;

        // Read vertices from PolygonPrismShapeComponent on the same entity
        AZ::PolygonPrismPtr polygonPrismPtr;
        LmbrCentral::PolygonPrismShapeComponentRequestBus::EventResult(
            polygonPrismPtr, GetEntityId(),
            &LmbrCentral::PolygonPrismShapeComponentRequests::GetPolygonPrism);

        if (!polygonPrismPtr)
        {
            AZ_Warning("CesiumPolygon", false,
                "No Polygon Prism Shape component found on entity. "
                "Add a Polygon Prism Shape component or change the Input Mode.");
            return result;
        }

        const AZStd::vector<AZ::Vector2>& vertices2D = polygonPrismPtr->m_vertexContainer.GetVertices();
        if (vertices2D.size() < 3)
        {
            return result;
        }

        // Get the entity's world transform to convert local shape vertices to world space
        AZ::Transform worldTransform = AZ::Transform::CreateIdentity();
        AZ::TransformBus::EventResult(worldTransform, GetEntityId(), &AZ::TransformBus::Events::GetWorldTM);

        // Get the relative-to-absolute world transform (O3DE -> ECEF)
        glm::dmat4 relToAbsWorld{ 1.0 };
        OriginShiftRequestBus::BroadcastResult(relToAbsWorld, &OriginShiftRequestBus::Events::GetRelToAbsWorld);

        constexpr double radToDeg = 180.0 / glm::pi<double>();

        result.reserve(vertices2D.size());
        for (const auto& v2d : vertices2D)
        {
            // PolygonPrism vertices are 2D (X, Y) in the entity's local space
            // Convert to 3D local position (Z = 0)
            AZ::Vector3 localPos(v2d.GetX(), v2d.GetY(), 0.0f);

            // Transform to world space using the entity's transform
            AZ::Vector3 worldPos = worldTransform.TransformPoint(localPos);

            // Convert O3DE world position to ECEF
            glm::dvec4 relPos(
                static_cast<double>(worldPos.GetX()),
                static_cast<double>(worldPos.GetY()),
                static_cast<double>(worldPos.GetZ()),
                1.0);
            glm::dvec4 ecefPos = relToAbsWorld * relPos;
            glm::dvec3 ecef(ecefPos.x, ecefPos.y, ecefPos.z);

            // Convert ECEF to cartographic (lon/lat/height)
            auto cartographic = GeospatialHelper::ECEFCartesianToCartographic(ecef);
            if (cartographic)
            {
                result.emplace_back(
                    cartographic->m_longitude * radToDeg,
                    cartographic->m_latitude * radToDeg);
            }
        }

        return result;
    }

    AZStd::vector<glm::dvec2> CartographicPolygonComponent::ConvertWorldPointsToLonLatDegrees() const
    {
        AZStd::vector<glm::dvec2> result;

        if (m_worldPoints.empty())
        {
            return result;
        }

        // Get the relative-to-absolute world transform (O3DE -> ECEF)
        glm::dmat4 relToAbsWorld{ 1.0 };
        OriginShiftRequestBus::BroadcastResult(relToAbsWorld, &OriginShiftRequestBus::Events::GetRelToAbsWorld);

        constexpr double radToDeg = 180.0 / glm::pi<double>();

        result.reserve(m_worldPoints.size());
        for (const auto& worldPt : m_worldPoints)
        {
            // Convert O3DE world position to ECEF
            glm::dvec4 relPos(
                static_cast<double>(worldPt.GetX()),
                static_cast<double>(worldPt.GetY()),
                static_cast<double>(worldPt.GetZ()),
                1.0);
            glm::dvec4 ecefPos = relToAbsWorld * relPos;
            glm::dvec3 ecef(ecefPos.x, ecefPos.y, ecefPos.z);

            // Convert ECEF to cartographic (lon/lat/height)
            auto cartographic = GeospatialHelper::ECEFCartesianToCartographic(ecef);
            if (cartographic)
            {
                // Cartographic stores radians; convert to degrees
                result.emplace_back(
                    cartographic->m_longitude * radToDeg,
                    cartographic->m_latitude * radToDeg);
            }
        }

        return result;
    }
} // namespace Cesium
