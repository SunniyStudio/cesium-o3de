#pragma once

#include <Cesium/Components/CartographicPolygonComponent.h>
#include <AzToolsFramework/ToolsComponents/EditorComponentBase.h>

namespace Cesium
{
    class CartographicPolygonEditorComponent : public AzToolsFramework::Components::EditorComponentBase
    {
    public:
        AZ_EDITOR_COMPONENT(CartographicPolygonEditorComponent, "{B8C9D0E1-F2A3-4B5C-6D7E-8F9A0B1C2D3E}");

        static void Reflect(AZ::ReflectContext* context);
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);

        CartographicPolygonEditorComponent() = default;
        void BuildGameEntity(AZ::Entity* gameEntity) override;

    private:
        void Init() override;
        void Activate() override;
        void Deactivate() override;

        AZStd::unique_ptr<CartographicPolygonComponent> m_runtimeComponent;
        PolygonInputMode m_inputMode = PolygonInputMode::ShapeComponent;
        AZStd::vector<glm::dvec2> m_lonLatPoints;
        AZStd::vector<AZ::Vector3> m_worldPoints;
        bool m_invertSelection = false;
    };
} // namespace Cesium
