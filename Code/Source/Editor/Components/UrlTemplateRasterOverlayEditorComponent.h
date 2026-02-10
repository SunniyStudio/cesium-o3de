#pragma once

#include <Cesium/Components/UrlTemplateRasterOverlayComponent.h>
#include <AzToolsFramework/ToolsComponents/EditorComponentBase.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>

namespace Cesium
{
    class UrlTemplateRasterOverlayEditorComponent : public AzToolsFramework::Components::EditorComponentBase
    {
    public:
        AZ_EDITOR_COMPONENT(UrlTemplateRasterOverlayEditorComponent, "{C3D4E5F6-A7B8-9C0D-1E2F-A3B4C5D6E7F8}");

        static void Reflect(AZ::ReflectContext* context);
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        UrlTemplateRasterOverlayEditorComponent();
        void BuildGameEntity(AZ::Entity* gameEntity) override;

    private:
        void Init() override;
        void Activate() override;
        void Deactivate() override;
        AZ::u32 OnSourceChanged();
        AZ::u32 OnConfigurationChanged();

        AZStd::unique_ptr<UrlTemplateRasterOverlayComponent> m_rasterOverlayComponent;
        RasterOverlayConfiguration m_configuration;
        UrlTemplateRasterOverlaySource m_source;
    };
} // namespace Cesium
