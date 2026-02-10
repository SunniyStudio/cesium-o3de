#include <Cesium/Metadata/CesiumFeatureIdSet.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/Script/ScriptContextAttributes.h>

namespace Cesium
{
    void CesiumFeatureIdSet::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<CesiumFeatureIdSet>()
                ->Version(0)
                ->Field("Type", &CesiumFeatureIdSet::m_type)
                ->Field("FeatureTableName", &CesiumFeatureIdSet::m_featureTableName)
                ->Field("AttributeName", &CesiumFeatureIdSet::m_attributeName)
                ->Field("AccessorIndex", &CesiumFeatureIdSet::m_accessorIndex)
                ->Field("FeatureCount", &CesiumFeatureIdSet::m_featureCount);
        }

        if (auto* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Enum<static_cast<int>(CesiumFeatureIdSetType::None)>("CesiumFeatureIdSetType_None")
                ->Enum<static_cast<int>(CesiumFeatureIdSetType::Attribute)>("CesiumFeatureIdSetType_Attribute")
                ->Enum<static_cast<int>(CesiumFeatureIdSetType::Texture)>("CesiumFeatureIdSetType_Texture")
                ->Enum<static_cast<int>(CesiumFeatureIdSetType::Implicit)>("CesiumFeatureIdSetType_Implicit");

            auto getType = [](CesiumFeatureIdSet* self) -> int
            {
                return static_cast<int>(self->m_type);
            };

            behaviorContext->Class<CesiumFeatureIdSet>("CesiumFeatureIdSet")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/Metadata")
                ->Property("Type", getType, nullptr)
                ->Property("FeatureTableName", BehaviorValueProperty(&CesiumFeatureIdSet::m_featureTableName))
                ->Property("AttributeName", BehaviorValueProperty(&CesiumFeatureIdSet::m_attributeName))
                ->Property("FeatureCount", BehaviorValueProperty(&CesiumFeatureIdSet::m_featureCount));
        }
    }

    CesiumFeatureIdSet::CesiumFeatureIdSet() = default;

    void CesiumPrimitiveFeatureIds::Reflect(AZ::ReflectContext* context)
    {
        CesiumFeatureIdSet::Reflect(context);

        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<CesiumPrimitiveFeatureIds>()
                ->Version(0)
                ->Field("GltfMeshIndex", &CesiumPrimitiveFeatureIds::m_gltfMeshIndex)
                ->Field("GltfPrimitiveIndex", &CesiumPrimitiveFeatureIds::m_gltfPrimitiveIndex)
                ->Field("FeatureIdSets", &CesiumPrimitiveFeatureIds::m_featureIdSets);
        }
    }

    bool CesiumPrimitiveFeatureIds::HasFeatureIds() const
    {
        return !m_featureIdSets.empty();
    }
} // namespace Cesium
