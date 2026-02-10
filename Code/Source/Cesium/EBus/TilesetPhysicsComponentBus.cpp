#include <Cesium/EBus/TilesetPhysicsComponentBus.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/Script/ScriptContextAttributes.h>

namespace Cesium
{
    TilesetPhysicsConfiguration::TilesetPhysicsConfiguration() = default;

    void TilesetPhysicsConfiguration::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<TilesetPhysicsConfiguration>()
                ->Version(0)
                ->Field("Enabled", &TilesetPhysicsConfiguration::m_enabled)
                ->Field("DoubleSided", &TilesetPhysicsConfiguration::m_doubleSided)
                ->Field("CollisionLayerName", &TilesetPhysicsConfiguration::m_collisionLayerName)
                ->Field("MaxCooksPerFrame", &TilesetPhysicsConfiguration::m_maxCooksPerFrame);

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<TilesetPhysicsConfiguration>("TilesetPhysicsConfiguration", "Physics collision settings for 3D Tiles.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default, &TilesetPhysicsConfiguration::m_enabled,
                        "Enabled", "Enable physics collision generation for loaded tiles.")
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default, &TilesetPhysicsConfiguration::m_doubleSided,
                        "Double Sided", "Generate collision for both sides of triangles.")
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default, &TilesetPhysicsConfiguration::m_collisionLayerName,
                        "Collision Layer", "Collision layer name (empty = Default).")
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default, &TilesetPhysicsConfiguration::m_maxCooksPerFrame,
                        "Max Cooks Per Frame", "Maximum triangle meshes to cook per frame (0 = unlimited).");
            }
        }

        if (auto* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Class<TilesetPhysicsConfiguration>("TilesetPhysicsConfiguration")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/Physics")
                ->Property("Enabled", BehaviorValueProperty(&TilesetPhysicsConfiguration::m_enabled))
                ->Property("DoubleSided", BehaviorValueProperty(&TilesetPhysicsConfiguration::m_doubleSided))
                ->Property("CollisionLayerName", BehaviorValueProperty(&TilesetPhysicsConfiguration::m_collisionLayerName))
                ->Property("MaxCooksPerFrame", BehaviorValueProperty(&TilesetPhysicsConfiguration::m_maxCooksPerFrame));
        }
    }

    void TilesetPhysicsRequest::Reflect(AZ::ReflectContext* context)
    {
        if (auto* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->EBus<TilesetPhysicsRequestBus>("TilesetPhysicsRequestBus")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/Physics")
                ->Event("SetConfiguration", &TilesetPhysicsRequestBus::Events::SetConfiguration)
                ->Event("GetConfiguration", &TilesetPhysicsRequestBus::Events::GetConfiguration)
                ->Event("SetEnabled", &TilesetPhysicsRequestBus::Events::SetEnabled)
                ->Event("GetEnabled", &TilesetPhysicsRequestBus::Events::GetEnabled)
                ->Event("GetActiveBodyCount", &TilesetPhysicsRequestBus::Events::GetActiveBodyCount);
        }
    }
} // namespace Cesium
