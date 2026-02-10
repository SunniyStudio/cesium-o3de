#include <Cesium/EBus/CesiumMetadataComponentBus.h>
#include <Cesium/Metadata/CesiumPropertyTable.h>
#include <Cesium/Math/MathReflect.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/Script/ScriptContextAttributes.h>

namespace Cesium
{
    void CesiumMetadataRequest::Reflect(AZ::ReflectContext* context)
    {
        if (auto* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->EBus<CesiumMetadataRequestBus>("CesiumMetadataRequestBus")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/Metadata")
                ->Event("GetFeatureTableNames", &CesiumMetadataRequestBus::Events::GetFeatureTableNames)
                ->Event("GetFeatureCount", &CesiumMetadataRequestBus::Events::GetFeatureCount)
                ->Event("GetPropertyNames", &CesiumMetadataRequestBus::Events::GetPropertyNames)
                ->Event("GetPropertyValue", &CesiumMetadataRequestBus::Events::GetPropertyValue)
                ->Event("GetAllPropertiesForFeature", &CesiumMetadataRequestBus::Events::GetAllPropertiesForFeature)
                ->Event("GetFeatureIdFromRaycast", &CesiumMetadataRequestBus::Events::GetFeatureIdFromRaycast)
                ->Event("GetPropertiesFromRaycast", &CesiumMetadataRequestBus::Events::GetPropertiesFromRaycast);
        }
    }
} // namespace Cesium
