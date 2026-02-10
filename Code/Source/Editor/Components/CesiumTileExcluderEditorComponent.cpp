#include "Editor/Components/CesiumTileExcluderEditorComponent.h"
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>

namespace Cesium
{
    void CesiumTileExcluderEditorComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto* sc = azrtti_cast<AZ::SerializeContext*>(context))
        {
            sc->Class<CesiumTileExcluderEditorComponent, AZ::Component>()
                ->Version(0)
                ->Field("PolygonEntityIds", &CesiumTileExcluderEditorComponent::m_polygonEntityIds)
                ->Field("Enabled", &CesiumTileExcluderEditorComponent::m_enabled);

            if (auto* ec = sc->GetEditContext())
            {
                ec->Class<CesiumTileExcluderEditorComponent>(
                        "Cesium Tile Excluder",
                        "Excludes (clips) tiles from a tileset based on cartographic polygon regions. "
                        "Use for 'dig hole' scenarios to place custom buildings inside 3D Tiles terrain.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "Cesium")
                    ->Attribute(AZ::Edit::Attributes::Icon, "Editor/Icons/Components/Cesium_logo_only.svg")
                    ->Attribute(AZ::Edit::Attributes::ViewportIcon, "Editor/Icons/Components/Cesium_logo_only.svg")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game", 0x232b318c))
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &CesiumTileExcluderEditorComponent::m_enabled,
                        "Enabled", "Enable tile exclusion")
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &CesiumTileExcluderEditorComponent::OnConfigChanged)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &CesiumTileExcluderEditorComponent::m_polygonEntityIds,
                        "Polygon Entities", "Entities with Cesium Cartographic Polygon components")
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &CesiumTileExcluderEditorComponent::OnConfigChanged);
            }
        }
    }

    void CesiumTileExcluderEditorComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    { provided.push_back(AZ_CRC_CE("CesiumTileExcluderService")); }

    void CesiumTileExcluderEditorComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    { incompatible.push_back(AZ_CRC_CE("CesiumTileExcluderService")); }

    void CesiumTileExcluderEditorComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    { required.push_back(AZ_CRC_CE("3DTilesEditorService")); }

    void CesiumTileExcluderEditorComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    { dependent.push_back(AZ_CRC_CE("3DTilesEditorService")); }

    void CesiumTileExcluderEditorComponent::BuildGameEntity(AZ::Entity* gameEntity)
    {
        auto* c = gameEntity->CreateComponent<CesiumTileExcluderComponent>();
        c->SetEntity(gameEntity);
    }

    void CesiumTileExcluderEditorComponent::Init()
    {
        AzToolsFramework::Components::EditorComponentBase::Init();
        if (!m_runtimeComponent)
        {
            m_runtimeComponent = AZStd::make_unique<CesiumTileExcluderComponent>();
        }
    }

    void CesiumTileExcluderEditorComponent::Activate()
    {
        // Activate the runtime component so excluder works in editor
        m_runtimeComponent->SetEntity(GetEntity());
        m_runtimeComponent->Init();
        m_runtimeComponent->SetPolygonEntityIds(m_polygonEntityIds);
        m_runtimeComponent->SetEnabled(m_enabled);
        m_runtimeComponent->Activate();
    }

    void CesiumTileExcluderEditorComponent::Deactivate()
    {
        m_runtimeComponent->Deactivate();
        m_runtimeComponent->SetEntity(nullptr);
    }

    AZ::u32 CesiumTileExcluderEditorComponent::OnConfigChanged()
    {
        // Update runtime component with new config
        if (m_runtimeComponent)
        {
            m_runtimeComponent->SetPolygonEntityIds(m_polygonEntityIds);
            m_runtimeComponent->SetEnabled(m_enabled);
        }
        return AZ::Edit::PropertyRefreshLevels::None;
    }
} // namespace Cesium
