#include <Cesium/EBus/CartographicPolygonComponentBus.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/Script/ScriptContextAttributes.h>

namespace Cesium
{
    void CartographicPolygonRequest::Reflect(AZ::ReflectContext* context)
    {
        if (auto* bc = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            bc->EBus<CartographicPolygonRequestBus>("CartographicPolygonRequestBus")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/Polygon")
                ->Event("GetPolygonPointsDegrees", &CartographicPolygonRequestBus::Events::GetPolygonPointsDegrees)
                ->Event("GetInvertSelection", &CartographicPolygonRequestBus::Events::GetInvertSelection);
        }
    }
} // namespace Cesium
