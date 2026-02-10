#pragma once

#include <Cesium/Components/WMTSRasterOverlayComponent.h>
#include <AzToolsFramework/ToolsComponents/EditorComponentBase.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>

namespace Cesium
{
    class WMTSRasterOverlayEditorComponent : public AzToolsFramework::Components::EditorComponentBase
    {
    public:
        AZ_EDITOR_COMPONENT(WMTSRasterOverlayEditorComponent, "{B2C3D4E5-F6A7-8B9C-0D1E-F2A3B4C5D6E7}");

        static void Reflect(AZ::ReflectContext* context);
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        WMTSRasterOverlayEditorComponent();
        void BuildGameEntity(AZ::Entity* gameEntity) override;

    private:
        void Init() override;
        void Activate() override;
        void Deactivate() override;
        AZ::u32 OnSourceChanged();
        AZ::u32 OnConfigurationChanged();

        AZStd::unique_ptr<WMTSRasterOverlayComponent> m_rasterOverlayComponent;
        RasterOverlayConfiguration m_configuration;
        WMTSRasterOverlaySource m_source;
    };
} // namespace Cesium
