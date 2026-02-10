#include <Cesium/Metadata/CesiumMetadataValue.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/Script/ScriptContextAttributes.h>
#include <sstream>

namespace Cesium
{
    void CesiumMetadataValue::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<CesiumMetadataValue>()
                ->Version(0)
                ->Field("Type", &CesiumMetadataValue::m_type)
                ->Field("BoolValue", &CesiumMetadataValue::m_boolValue)
                ->Field("IntValue", &CesiumMetadataValue::m_intValue)
                ->Field("DoubleValue", &CesiumMetadataValue::m_doubleValue)
                ->Field("StringValue", &CesiumMetadataValue::m_stringValue)
                ->Field("VecValue", &CesiumMetadataValue::m_vecValue);
        }

        if (auto* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Enum<static_cast<int>(CesiumMetadataValueType::None)>("CesiumMetadataValueType_None")
                ->Enum<static_cast<int>(CesiumMetadataValueType::Boolean)>("CesiumMetadataValueType_Boolean")
                ->Enum<static_cast<int>(CesiumMetadataValueType::Int32)>("CesiumMetadataValueType_Int32")
                ->Enum<static_cast<int>(CesiumMetadataValueType::Int64)>("CesiumMetadataValueType_Int64")
                ->Enum<static_cast<int>(CesiumMetadataValueType::Float)>("CesiumMetadataValueType_Float")
                ->Enum<static_cast<int>(CesiumMetadataValueType::Double)>("CesiumMetadataValueType_Double")
                ->Enum<static_cast<int>(CesiumMetadataValueType::String)>("CesiumMetadataValueType_String")
                ->Enum<static_cast<int>(CesiumMetadataValueType::Vec2)>("CesiumMetadataValueType_Vec2")
                ->Enum<static_cast<int>(CesiumMetadataValueType::Vec3)>("CesiumMetadataValueType_Vec3")
                ->Enum<static_cast<int>(CesiumMetadataValueType::Vec4)>("CesiumMetadataValueType_Vec4");

            auto getType = [](CesiumMetadataValue* self) -> int
            {
                return static_cast<int>(self->GetValueType());
            };

            behaviorContext->Class<CesiumMetadataValue>("CesiumMetadataValue")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/Metadata")
                ->Property("ValueType", getType, nullptr)
                ->Property("BoolValue", BehaviorValueProperty(&CesiumMetadataValue::m_boolValue))
                ->Property("IntValue", BehaviorValueProperty(&CesiumMetadataValue::m_intValue))
                ->Property("DoubleValue", BehaviorValueProperty(&CesiumMetadataValue::m_doubleValue))
                ->Property("StringValue", BehaviorValueProperty(&CesiumMetadataValue::m_stringValue))
                ->Method("IsNone", &CesiumMetadataValue::IsNone)
                ->Method("GetBoolean", &CesiumMetadataValue::GetBoolean)
                ->Method("GetInt32", &CesiumMetadataValue::GetInt32)
                ->Method("GetInt64", &CesiumMetadataValue::GetInt64)
                ->Method("GetFloat", &CesiumMetadataValue::GetFloat)
                ->Method("GetDouble", &CesiumMetadataValue::GetDouble)
                ->Method("GetString", &CesiumMetadataValue::GetString)
                ->Method("GetVec2", &CesiumMetadataValue::GetVec2)
                ->Method("GetVec3", &CesiumMetadataValue::GetVec3)
                ->Method("GetVec4", &CesiumMetadataValue::GetVec4)
                ->Method("ToString", &CesiumMetadataValue::ToString);
        }
    }

    CesiumMetadataValue::CesiumMetadataValue() = default;

    CesiumMetadataValue::CesiumMetadataValue(bool value)
        : m_type{ CesiumMetadataValueType::Boolean }
        , m_boolValue{ value }
    {
    }

    CesiumMetadataValue::CesiumMetadataValue(AZ::s32 value)
        : m_type{ CesiumMetadataValueType::Int32 }
        , m_intValue{ static_cast<AZ::s64>(value) }
    {
    }

    CesiumMetadataValue::CesiumMetadataValue(AZ::s64 value)
        : m_type{ CesiumMetadataValueType::Int64 }
        , m_intValue{ value }
    {
    }

    CesiumMetadataValue::CesiumMetadataValue(float value)
        : m_type{ CesiumMetadataValueType::Float }
        , m_doubleValue{ static_cast<double>(value) }
    {
    }

    CesiumMetadataValue::CesiumMetadataValue(double value)
        : m_type{ CesiumMetadataValueType::Double }
        , m_doubleValue{ value }
    {
    }

    CesiumMetadataValue::CesiumMetadataValue(const AZStd::string& value)
        : m_type{ CesiumMetadataValueType::String }
        , m_stringValue{ value }
    {
    }

    CesiumMetadataValue::CesiumMetadataValue(const glm::dvec2& value)
        : m_type{ CesiumMetadataValueType::Vec2 }
        , m_vecValue{ value.x, value.y, 0.0, 0.0 }
    {
    }

    CesiumMetadataValue::CesiumMetadataValue(const glm::dvec3& value)
        : m_type{ CesiumMetadataValueType::Vec3 }
        , m_vecValue{ value.x, value.y, value.z, 0.0 }
    {
    }

    CesiumMetadataValue::CesiumMetadataValue(const glm::dvec4& value)
        : m_type{ CesiumMetadataValueType::Vec4 }
        , m_vecValue{ value }
    {
    }

    CesiumMetadataValueType CesiumMetadataValue::GetValueType() const
    {
        return m_type;
    }

    bool CesiumMetadataValue::IsNone() const
    {
        return m_type == CesiumMetadataValueType::None;
    }

    bool CesiumMetadataValue::GetBoolean(bool defaultValue) const
    {
        if (m_type == CesiumMetadataValueType::Boolean)
        {
            return m_boolValue;
        }
        return defaultValue;
    }

    AZ::s32 CesiumMetadataValue::GetInt32(AZ::s32 defaultValue) const
    {
        if (m_type == CesiumMetadataValueType::Int32 || m_type == CesiumMetadataValueType::Int64)
        {
            return static_cast<AZ::s32>(m_intValue);
        }
        return defaultValue;
    }

    AZ::s64 CesiumMetadataValue::GetInt64(AZ::s64 defaultValue) const
    {
        if (m_type == CesiumMetadataValueType::Int32 || m_type == CesiumMetadataValueType::Int64)
        {
            return m_intValue;
        }
        return defaultValue;
    }

    float CesiumMetadataValue::GetFloat(float defaultValue) const
    {
        if (m_type == CesiumMetadataValueType::Float || m_type == CesiumMetadataValueType::Double)
        {
            return static_cast<float>(m_doubleValue);
        }
        return defaultValue;
    }

    double CesiumMetadataValue::GetDouble(double defaultValue) const
    {
        if (m_type == CesiumMetadataValueType::Float || m_type == CesiumMetadataValueType::Double)
        {
            return m_doubleValue;
        }
        return defaultValue;
    }

    AZStd::string CesiumMetadataValue::GetString(const AZStd::string& defaultValue) const
    {
        if (m_type == CesiumMetadataValueType::String)
        {
            return m_stringValue;
        }
        // Auto-convert other types to string
        return ToString().empty() ? defaultValue : ToString();
    }

    glm::dvec2 CesiumMetadataValue::GetVec2(const glm::dvec2& defaultValue) const
    {
        if (m_type == CesiumMetadataValueType::Vec2 || m_type == CesiumMetadataValueType::Vec3 ||
            m_type == CesiumMetadataValueType::Vec4)
        {
            return glm::dvec2(m_vecValue.x, m_vecValue.y);
        }
        return defaultValue;
    }

    glm::dvec3 CesiumMetadataValue::GetVec3(const glm::dvec3& defaultValue) const
    {
        if (m_type == CesiumMetadataValueType::Vec3 || m_type == CesiumMetadataValueType::Vec4)
        {
            return glm::dvec3(m_vecValue.x, m_vecValue.y, m_vecValue.z);
        }
        return defaultValue;
    }

    glm::dvec4 CesiumMetadataValue::GetVec4(const glm::dvec4& defaultValue) const
    {
        if (m_type == CesiumMetadataValueType::Vec4)
        {
            return m_vecValue;
        }
        return defaultValue;
    }

    AZStd::string CesiumMetadataValue::ToString() const
    {
        switch (m_type)
        {
        case CesiumMetadataValueType::Boolean:
            return m_boolValue ? "true" : "false";
        case CesiumMetadataValueType::Int32:
        case CesiumMetadataValueType::Int64:
            return AZStd::string::format("%lld", static_cast<long long>(m_intValue));
        case CesiumMetadataValueType::Float:
        case CesiumMetadataValueType::Double:
            return AZStd::string::format("%.6f", m_doubleValue);
        case CesiumMetadataValueType::String:
            return m_stringValue;
        case CesiumMetadataValueType::Vec2:
            return AZStd::string::format("(%.4f, %.4f)", m_vecValue.x, m_vecValue.y);
        case CesiumMetadataValueType::Vec3:
            return AZStd::string::format("(%.4f, %.4f, %.4f)", m_vecValue.x, m_vecValue.y, m_vecValue.z);
        case CesiumMetadataValueType::Vec4:
            return AZStd::string::format("(%.4f, %.4f, %.4f, %.4f)", m_vecValue.x, m_vecValue.y, m_vecValue.z, m_vecValue.w);
        default:
            return "None";
        }
    }
} // namespace Cesium
