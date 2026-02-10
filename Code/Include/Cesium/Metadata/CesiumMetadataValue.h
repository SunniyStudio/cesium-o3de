#pragma once

#include <AzCore/RTTI/RTTI.h>
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/RTTI/ReflectContext.h>
#include <AzCore/std/string/string.h>
#include <glm/glm.hpp>
#include <cstdint>

namespace Cesium
{
    enum class CesiumMetadataValueType : AZ::u8
    {
        None = 0,
        Boolean,
        Int32,
        Int64,
        Float,
        Double,
        String,
        Vec2,
        Vec3,
        Vec4
    };

    //! A generic metadata value that can hold various types of data from 3D Tiles metadata.
    //! This type is designed to be easily consumed via Script Canvas / BehaviorContext.
    struct CesiumMetadataValue final
    {
        AZ_RTTI(CesiumMetadataValue, "{A3B7C5D1-E2F4-4A6B-8C9D-0E1F2A3B4C5D}");
        AZ_CLASS_ALLOCATOR(CesiumMetadataValue, AZ::SystemAllocator, 0);

        static void Reflect(AZ::ReflectContext* context);

        CesiumMetadataValue();
        explicit CesiumMetadataValue(bool value);
        explicit CesiumMetadataValue(AZ::s32 value);
        explicit CesiumMetadataValue(AZ::s64 value);
        explicit CesiumMetadataValue(float value);
        explicit CesiumMetadataValue(double value);
        explicit CesiumMetadataValue(const AZStd::string& value);
        explicit CesiumMetadataValue(const glm::dvec2& value);
        explicit CesiumMetadataValue(const glm::dvec3& value);
        explicit CesiumMetadataValue(const glm::dvec4& value);

        CesiumMetadataValueType GetValueType() const;
        bool IsNone() const;

        bool GetBoolean(bool defaultValue = false) const;
        AZ::s32 GetInt32(AZ::s32 defaultValue = 0) const;
        AZ::s64 GetInt64(AZ::s64 defaultValue = 0) const;
        float GetFloat(float defaultValue = 0.0f) const;
        double GetDouble(double defaultValue = 0.0) const;
        AZStd::string GetString(const AZStd::string& defaultValue = {}) const;
        glm::dvec2 GetVec2(const glm::dvec2& defaultValue = glm::dvec2(0.0)) const;
        glm::dvec3 GetVec3(const glm::dvec3& defaultValue = glm::dvec3(0.0)) const;
        glm::dvec4 GetVec4(const glm::dvec4& defaultValue = glm::dvec4(0.0)) const;

        //! Convert value to a human-readable string representation.
        AZStd::string ToString() const;

        CesiumMetadataValueType m_type = CesiumMetadataValueType::None;
        bool m_boolValue = false;
        AZ::s64 m_intValue = 0;
        double m_doubleValue = 0.0;
        AZStd::string m_stringValue;
        glm::dvec4 m_vecValue = glm::dvec4(0.0);
    };
} // namespace Cesium
