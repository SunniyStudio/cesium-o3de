#pragma once

#include <Cesium/EBus/CartographicPolygonComponentBus.h>
#include <AzCore/Component/Component.h>
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/std/containers/vector.h>
#include <AzCore/Math/Vector3.h>
#include <glm/glm.hpp>

namespace Cesium
{
    enum class PolygonInputMode : AZ::u8
    {
        LonLatDegrees = 0,  //!< Input as (longitude, latitude) in degrees
        WorldCoordinates,   //!< Input as O3DE world-space positions (auto-converted via georeference)
        ShapeComponent      //!< Read vertices from a Polygon Prism Shape on the same entity
    };

    //! Defines a cartographic polygon region for tile exclusion/clipping.
    //! Place on any entity. Reference this entity from a CesiumTileExcluderComponent
    //! on your tileset entity to clip or exclude tiles within the polygon.
    //!
    //! Supports two input modes:
    //! - LonLatDegrees: directly input longitude/latitude in degrees
    //! - WorldCoordinates: input O3DE world positions, automatically converted via origin shift
    class CartographicPolygonComponent
        : public AZ::Component
        , public CartographicPolygonRequestBus::Handler
    {
    public:
        AZ_COMPONENT(CartographicPolygonComponent, "{7A8B9C0D-1E2F-3A4B-5C6D-7E8F9A0B1C2D}", AZ::Component);

        static void Reflect(AZ::ReflectContext* context);
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);

        CartographicPolygonComponent() = default;

        using AZ::Component::SetEntity;

        void Init() override {}
        void Activate() override;
        void Deactivate() override;

        // CartographicPolygonRequestBus
        AZStd::vector<glm::dvec2> GetPolygonPointsDegrees() const override;
        AZStd::vector<glm::dvec2> GetPolygonPointsRadians() const override;
        bool GetInvertSelection() const override;

    private:
        //! Convert O3DE world coordinate points to lon/lat degrees via georeference.
        AZStd::vector<glm::dvec2> ConvertWorldPointsToLonLatDegrees() const;

        //! Read vertices from a PolygonPrismShapeComponent on the same entity,
        //! transform to world space, then convert to lon/lat degrees.
        AZStd::vector<glm::dvec2> ReadShapeComponentPoints() const;

        PolygonInputMode m_inputMode = PolygonInputMode::ShapeComponent;

        //! Polygon vertices as (longitude, latitude) in degrees. Used when m_inputMode == LonLatDegrees.
        AZStd::vector<glm::dvec2> m_lonLatPoints;

        //! Polygon vertices as O3DE world-space positions. Used when m_inputMode == WorldCoordinates.
        AZStd::vector<AZ::Vector3> m_worldPoints;

        bool m_invertSelection = false;
    };
} // namespace Cesium
