#pragma once

#include <Cesium/Components/CesiumTileExcluderComponent.h>
#include <AzToolsFramework/ToolsComponents/EditorComponentBase.h>

namespace Cesium
{
    class CesiumTileExcluderEditorComponent : public AzToolsFramework::Components::EditorComponentBase
    {
    public:
        AZ_EDITOR_COMPONENT(CesiumTileExcluderEditorComponent, "{C9D0E1F2-A3B4-5C6D-7E8F-9A0B1C2D3E4F}");

        static void Reflect(AZ::ReflectContext* context);
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        CesiumTileExcluderEditorComponent() = default;
        void BuildGameEntity(AZ::Entity* gameEntity) override;

    private:
        void Init() override;
        void Activate() override;
        void Deactivate() override;
        AZ::u32 OnConfigChanged();

        AZStd::unique_ptr<CesiumTileExcluderComponent> m_runtimeComponent;
        AZStd::vector<AZ::EntityId> m_polygonEntityIds;
        bool m_enabled = true;
    };
} // namespace Cesium
