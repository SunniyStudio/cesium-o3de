#pragma once

#include <AzCore/RTTI/RTTI.h>
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/RTTI/ReflectContext.h>
#include <AzCore/std/string/string.h>
#include <cstdint>

namespace Cesium
{
    enum class CesiumFeatureIdSetType : AZ::u8
    {
        None = 0,
        Attribute,  //!< Feature IDs stored as a vertex attribute (e.g. _FEATURE_ID_0)
        Texture,    //!< Feature IDs stored in a texture
        Implicit    //!< Feature IDs are implicitly derived from vertex/instance index
    };

    //! Describes a single feature ID set on a glTF primitive.
    //! A primitive may have multiple feature ID sets, each mapping vertices to a different feature table.
    struct CesiumFeatureIdSet final
    {
        AZ_RTTI(CesiumFeatureIdSet, "{B4C8D6E2-F3A5-4B7C-9D0E-1F2A3B4C5D6E}");
        AZ_CLASS_ALLOCATOR(CesiumFeatureIdSet, AZ::SystemAllocator, 0);

        static void Reflect(AZ::ReflectContext* context);

        CesiumFeatureIdSet();

        CesiumFeatureIdSetType m_type = CesiumFeatureIdSetType::None;

        //!< Name of the feature table this set maps to (key into model extension's featureTables map)
        AZStd::string m_featureTableName;

        //!< Name of the vertex attribute containing feature IDs (e.g. "_FEATURE_ID_0")
        AZStd::string m_attributeName;

        //!< Accessor index in the glTF model for attribute-based feature IDs (-1 if N/A)
        AZ::s32 m_accessorIndex = -1;

        //!< Total number of features in the referenced feature table
        AZ::s64 m_featureCount = 0;
    };

    //! Per-primitive metadata describing which feature ID sets and feature tables are available.
    struct CesiumPrimitiveFeatureIds final
    {
        AZ_RTTI(CesiumPrimitiveFeatureIds, "{E8A1B2C3-D4E5-4F6A-7B8C-9D0E1F2A3B4C}");
        AZ_CLASS_ALLOCATOR(CesiumPrimitiveFeatureIds, AZ::SystemAllocator, 0);

        static void Reflect(AZ::ReflectContext* context);

        //!< Indices into the source glTF model for resolving data at query time
        AZ::s32 m_gltfMeshIndex = -1;
        AZ::s32 m_gltfPrimitiveIndex = -1;

        //!< All feature ID sets available on this primitive
        AZStd::vector<CesiumFeatureIdSet> m_featureIdSets;

        bool HasFeatureIds() const;
    };
} // namespace Cesium
