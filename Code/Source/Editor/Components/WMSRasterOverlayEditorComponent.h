#pragma once

#include <Cesium/Components/WMSRasterOverlayComponent.h>
#include <AzToolsFramework/ToolsComponents/EditorComponentBase.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>

namespace Cesium
{
    class WMSRasterOverlayEditorComponent : public AzToolsFramework::Components::EditorComponentBase
    {
    public:
        AZ_EDITOR_COMPONENT(WMSRasterOverlayEditorComponent, "{A1B2C3D4-E5F6-7A8B-9C0D-E1F2A3B4C5D6}");

        static void Reflect(AZ::ReflectContext* context);
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        WMSRasterOverlayEditorComponent();
        void BuildGameEntity(AZ::Entity* gameEntity) override;

    private:
        void Init() override;
        void Activate() override;
        void Deactivate() override;
        AZ::u32 OnSourceChanged();
        AZ::u32 OnConfigurationChanged();

        AZStd::unique_ptr<WMSRasterOverlayComponent> m_rasterOverlayComponent;
        RasterOverlayConfiguration m_configuration;
        WMSRasterOverlaySource m_source;
    };
} // namespace Cesium
