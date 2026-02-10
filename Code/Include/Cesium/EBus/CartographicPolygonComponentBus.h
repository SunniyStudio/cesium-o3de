#pragma once

#include <AzCore/Component/ComponentBus.h>
#include <AzCore/RTTI/ReflectContext.h>
#include <AzCore/std/containers/vector.h>
#include <glm/glm.hpp>

namespace Cesium
{
    //! EBus for querying cartographic polygon data from an entity.
    class CartographicPolygonRequest : public AZ::ComponentBus
    {
    public:
        static void Reflect(AZ::ReflectContext* context);

        //! Returns polygon vertices as (longitude, latitude) in degrees.
        virtual AZStd::vector<glm::dvec2> GetPolygonPointsDegrees() const = 0;

        //! Returns polygon vertices as (longitude, latitude) in radians (CesiumNative convention).
        virtual AZStd::vector<glm::dvec2> GetPolygonPointsRadians() const = 0;

        //! Returns true if this polygon should invert the selection (exclude inside vs outside).
        virtual bool GetInvertSelection() const = 0;
    };

    using CartographicPolygonRequestBus = AZ::EBus<CartographicPolygonRequest>;
} // namespace Cesium
