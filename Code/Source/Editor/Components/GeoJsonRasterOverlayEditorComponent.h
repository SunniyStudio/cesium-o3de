#pragma once

#include <Cesium/Components/GeoJsonRasterOverlayComponent.h>
#include <AzToolsFramework/ToolsComponents/EditorComponentBase.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>

namespace Cesium
{
    class GeoJsonRasterOverlayEditorComponent : public AzToolsFramework::Components::EditorComponentBase
    {
    public:
        AZ_EDITOR_COMPONENT(GeoJsonRasterOverlayEditorComponent, "{A7B8C9D0-E1F2-3A4B-5C6D-E7F8A9B0C1D2}");

        static void Reflect(AZ::ReflectContext* context);
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        GeoJsonRasterOverlayEditorComponent();
        void BuildGameEntity(AZ::Entity* gameEntity) override;

    private:
        void Init() override;
        void Activate() override;
        void Deactivate() override;
        AZ::u32 OnSourceChanged();
        AZ::u32 OnConfigurationChanged();

        AZStd::unique_ptr<GeoJsonRasterOverlayComponent> m_rasterOverlayComponent;
        RasterOverlayConfiguration m_configuration;
        GeoJsonRasterOverlaySource m_source;
    };
} // namespace Cesium
