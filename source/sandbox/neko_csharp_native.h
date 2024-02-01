
#ifndef NEKO_CSHARP_NATIVE_H
#define NEKO_CSHARP_NATIVE_H

#include <array>
#include <atomic>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <mutex>
#include <set>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "engine/neko.h"

// HostFXR / CoreCLR
// #include <nethost.h>
#include <coreclr_delegates.h>
#include <hostfxr.h>

#ifdef NEKO_PLATFORM_WIN
#define NEKO_CS_CALLTYPE __cdecl
#define NEKO_CS_HOSTFXR_NAME "hostfxr.dll"

#ifdef _WCHAR_T_DEFINED
#define NEKO_CS_STR(s) L##s
#define NEKO_CS_WIDE_CHARS

using CharType = wchar_t;
using StringView = std::wstring_view;

#else
#define NEKO_CS_STR(s) s

using CharType = unsigned short;
using StringView = std::string_view;
#endif
#else
#define NEKO_CS_CALLTYPE
#define NEKO_CS_STR(s) s
#define NEKO_CS_HOSTFXR_NAME "libhostfxr.so"

using CharType = char;
using StringView = std::string_view;
#endif

#define NEKO_CS_DOTNET_TARGET_VERSION_MAJOR 8
#define NEKO_CS_DOTNET_TARGET_VERSION_MAJOR_STR '8'
#define NEKO_CS_UNMANAGED_CALLERS_ONLY ((const CharType*)-1)

#ifdef __clang__
#define NEKO_CS_HAS_SOURCE_LOCATION 0
#else
#define NEKO_CS_HAS_SOURCE_LOCATION __has_include(<source_location>)
#endif

#if NEKO_CS_HAS_SOURCE_LOCATION
#include <source_location>

#define NEKO_CS_SOURCE_LOCATION                                      \
    std::source_location location = std::source_location::current(); \
    const char* file = location.file_name();                         \
    int line = location.line()
#else
#define NEKO_CS_SOURCE_LOCATION  \
    const char* file = __FILE__; \
    int line = __LINE__
#endif

#define NEKO_CS_VERIFY(expr)                                                                                               \
    {                                                                                                                      \
        if (!(expr)) {                                                                                                     \
            NEKO_CS_SOURCE_LOCATION;                                                                                       \
            std::cerr << "[NekoCS.Native]: Assert Failed! Expression: " << #expr << " at " << file << ":" << line << "\n"; \
            neko_debugbreak();                                                                                             \
        }                                                                                                                  \
    }

namespace neko_cs {

using Bool32 = uint32_t;

enum class TypeAccessibility { Public, Private, Protected, Internal, ProtectedPublic, PrivateProtected };

using TypeId = int32_t;
using ManagedHandle = int32_t;

struct InternalCall {
    const CharType* Name;
    void* NativeFunctionPtr;
};

class HostInstance;
class ManagedAssembly;
class Type;
class Attribute;
class ManagedField;
class ManagedObject;

struct UnmanagedArray;

enum class AssemblyLoadStatus;
enum class GCCollectionMode;
enum class ManagedType;

enum class GCCollectionMode {
    // 默认设置与直接使用FORCED相同
    Default,

    // 强制立即进行垃圾回收
    Forced,

    // 允许垃圾回收器确定是否应立即回收对象
    Optimized,

    // 请求垃圾回收器释放尽可能多的内存
    Aggressive
};

class String {
public:
    static String New(const char* InString);
    static String New(std::string_view InString);
    static void Free(String& InString);

    void Assign(std::string_view InString);

    operator std::string() const;

    bool operator==(const String& InOther) const;
    bool operator==(std::string_view InOther) const;

    CharType* Data() { return m_String; }
    const CharType* Data() const { return m_String; }

private:
    CharType* m_String = nullptr;
    Bool32 m_IsDisposed = false;  // 布局需要匹配 C# NativeString 结构 在 C++ 中未使用
};

struct ScopedString {
    ScopedString(String InString) : m_String(InString) {}

    ScopedString& operator=(String InOther) {
        String::Free(m_String);
        m_String = InOther;
        return *this;
    }

    ScopedString& operator=(const ScopedString& InOther) {
        String::Free(m_String);
        m_String = InOther.m_String;
        return *this;
    }

    ~ScopedString() { String::Free(m_String); }

    operator std::string() const { return m_String; }
    operator String() const { return m_String; }

    bool operator==(const ScopedString& InOther) const { return m_String == InOther.m_String; }

    bool operator==(std::string_view InOther) const { return m_String == InOther; }

private:
    String m_String;
};

class StringHelper {
public:
#if defined(NEKO_CS_WIDE_CHARS)
    static std::wstring ConvertUtf8ToWide(std::string_view InString);
    static std::string ConvertWideToUtf8(std::wstring_view InString);
#else
    static std::string ConvertUtf8ToWide(std::string_view InString);
    static std::string ConvertWideToUtf8(std::string_view InString);
#endif
};

class Attribute {
public:
    Type& GetType();

    template <typename TReturn>
    TReturn GetFieldValue(std::string_view InFieldName) {
        TReturn result;
        GetFieldValueInternal(InFieldName, &result);
        return result;
    }

private:
    void GetFieldValueInternal(std::string_view InFieldName, void* OutValue) const;

private:
    ManagedHandle m_Handle = -1;
    Type* m_Type = nullptr;

    friend class Type;
    friend class MethodInfo;
    friend class FieldInfo;
    friend class PropertyInfo;
};

class GC {
public:
    static void Collect();
    static void Collect(int32_t InGeneration, GCCollectionMode InCollectionMode = GCCollectionMode::Default, bool InBlocking = true, bool InCompacting = false);

    static void WaitForPendingFinalizers();
};

using SetInternalCallsFn = void (*)(void*, int32_t);
using CreateAssemblyLoadContextFn = int32_t (*)(String);
using UnloadAssemblyLoadContextFn = void (*)(int32_t);
using LoadManagedAssemblyFn = int32_t (*)(int32_t, String);
using GetLastLoadStatusFn = AssemblyLoadStatus (*)();
using GetAssemblyNameFn = String (*)(int32_t);

#pragma region TypeInterface

using GetAssemblyTypesFn = void (*)(int32_t, TypeId*, int32_t*);
using GetTypeIdFn = void (*)(String, TypeId*);
using GetFullTypeNameFn = String (*)(TypeId);
using GetAssemblyQualifiedNameFn = String (*)(TypeId);
using GetBaseTypeFn = void (*)(TypeId, TypeId*);
using GetTypeSizeFn = int32_t (*)(TypeId);
using IsTypeSubclassOfFn = Bool32 (*)(TypeId, TypeId);
using IsTypeAssignableToFn = Bool32 (*)(TypeId, TypeId);
using IsTypeAssignableFromFn = Bool32 (*)(TypeId, TypeId);
using IsTypeSZArrayFn = Bool32 (*)(TypeId);
using GetElementTypeFn = void (*)(TypeId, TypeId*);
using GetTypeMethodsFn = void (*)(TypeId, ManagedHandle*, int32_t*);
using GetTypeFieldsFn = void (*)(TypeId, ManagedHandle*, int32_t*);
using GetTypePropertiesFn = void (*)(TypeId, ManagedHandle*, int32_t*);
using HasTypeAttributeFn = Bool32 (*)(TypeId, TypeId);
using GetTypeAttributesFn = void (*)(ManagedHandle, TypeId*, int32_t*);
using GetTypeManagedTypeFn = ManagedType (*)(TypeId);

#pragma endregion

#pragma region MethodInfo
using GetMethodInfoNameFn = String (*)(ManagedHandle);
using GetMethodInfoReturnTypeFn = void (*)(ManagedHandle, TypeId*);
using GetMethodInfoParameterTypesFn = void (*)(ManagedHandle, TypeId*, int32_t*);
using GetMethodInfoAccessibilityFn = TypeAccessibility (*)(ManagedHandle);
using GetMethodInfoAttributesFn = void (*)(ManagedHandle, TypeId*, int32_t*);
#pragma endregion

#pragma region FieldInfo
using GetFieldInfoNameFn = String (*)(ManagedHandle);
using GetFieldInfoTypeFn = void (*)(ManagedHandle, TypeId*);
using GetFieldInfoAccessibilityFn = TypeAccessibility (*)(ManagedHandle);
using GetFieldInfoAttributesFn = void (*)(ManagedHandle, TypeId*, int32_t*);
#pragma endregion

#pragma region PropertyInfo
using GetPropertyInfoNameFn = String (*)(ManagedHandle);
using GetPropertyInfoTypeFn = void (*)(ManagedHandle, TypeId*);
using GetPropertyInfoAttributesFn = void (*)(ManagedHandle, TypeId*, int32_t*);
#pragma endregion

#pragma region Attribute
using GetAttributeFieldValueFn = void (*)(ManagedHandle, String, void*);
using GetAttributeTypeFn = void (*)(ManagedHandle, TypeId*);
#pragma endregion

using CreateObjectFn = void* (*)(TypeId, Bool32, const void**, const ManagedType*, int32_t);
using InvokeMethodFn = void (*)(void*, String, const void**, const ManagedType*, int32_t);
using InvokeMethodRetFn = void (*)(void*, String, const void**, const ManagedType*, int32_t, void*);
using InvokeStaticMethodFn = void (*)(TypeId, String, const void**, const ManagedType*, int32_t);
using InvokeStaticMethodRetFn = void (*)(TypeId, String, const void**, const ManagedType*, int32_t, void*);
using SetFieldValueFn = void (*)(void*, String, void*);
using GetFieldValueFn = void (*)(void*, String, void*);
using SetPropertyValueFn = void (*)(void*, String, void*);
using GetPropertyValueFn = void (*)(void*, String, void*);
using DestroyObjectFn = void (*)(void*);

using CollectGarbageFn = void (*)(int32_t, GCCollectionMode, Bool32, Bool32);
using WaitForPendingFinalizersFn = void (*)();

struct ManagedFunctions {
    SetInternalCallsFn SetInternalCallsFptr = nullptr;
    LoadManagedAssemblyFn LoadManagedAssemblyFptr = nullptr;
    UnloadAssemblyLoadContextFn UnloadAssemblyLoadContextFptr = nullptr;
    GetLastLoadStatusFn GetLastLoadStatusFptr = nullptr;
    GetAssemblyNameFn GetAssemblyNameFptr = nullptr;

#pragma region TypeInterface

    GetAssemblyTypesFn GetAssemblyTypesFptr = nullptr;
    GetTypeIdFn GetTypeIdFptr = nullptr;
    GetFullTypeNameFn GetFullTypeNameFptr = nullptr;
    GetAssemblyQualifiedNameFn GetAssemblyQualifiedNameFptr = nullptr;
    GetBaseTypeFn GetBaseTypeFptr = nullptr;
    GetTypeSizeFn GetTypeSizeFptr = nullptr;
    IsTypeSubclassOfFn IsTypeSubclassOfFptr = nullptr;
    IsTypeAssignableToFn IsTypeAssignableToFptr = nullptr;
    IsTypeAssignableFromFn IsTypeAssignableFromFptr = nullptr;
    IsTypeSZArrayFn IsTypeSZArrayFptr = nullptr;
    GetElementTypeFn GetElementTypeFptr = nullptr;
    GetTypeMethodsFn GetTypeMethodsFptr = nullptr;
    GetTypeFieldsFn GetTypeFieldsFptr = nullptr;
    GetTypePropertiesFn GetTypePropertiesFptr = nullptr;
    HasTypeAttributeFn HasTypeAttributeFptr = nullptr;
    GetTypeAttributesFn GetTypeAttributesFptr = nullptr;
    GetTypeManagedTypeFn GetTypeManagedTypeFptr = nullptr;

#pragma endregion

#pragma region MethodInfo
    GetMethodInfoNameFn GetMethodInfoNameFptr = nullptr;
    GetMethodInfoReturnTypeFn GetMethodInfoReturnTypeFptr = nullptr;
    GetMethodInfoParameterTypesFn GetMethodInfoParameterTypesFptr = nullptr;
    GetMethodInfoAccessibilityFn GetMethodInfoAccessibilityFptr = nullptr;
    GetMethodInfoAttributesFn GetMethodInfoAttributesFptr = nullptr;
#pragma endregion

#pragma region FieldInfo
    GetFieldInfoNameFn GetFieldInfoNameFptr = nullptr;
    GetFieldInfoTypeFn GetFieldInfoTypeFptr = nullptr;
    GetFieldInfoAccessibilityFn GetFieldInfoAccessibilityFptr = nullptr;
    GetFieldInfoAttributesFn GetFieldInfoAttributesFptr = nullptr;
#pragma endregion

#pragma region PropertyInfo
    GetPropertyInfoNameFn GetPropertyInfoNameFptr = nullptr;
    GetPropertyInfoTypeFn GetPropertyInfoTypeFptr = nullptr;
    GetPropertyInfoAttributesFn GetPropertyInfoAttributesFptr = nullptr;
#pragma endregion

#pragma region Attribute
    GetAttributeFieldValueFn GetAttributeFieldValueFptr = nullptr;
    GetAttributeTypeFn GetAttributeTypeFptr = nullptr;
#pragma endregion

    CreateObjectFn CreateObjectFptr = nullptr;
    CreateAssemblyLoadContextFn CreateAssemblyLoadContextFptr = nullptr;
    InvokeMethodFn InvokeMethodFptr = nullptr;
    InvokeMethodRetFn InvokeMethodRetFptr = nullptr;
    InvokeStaticMethodFn InvokeStaticMethodFptr = nullptr;
    InvokeStaticMethodRetFn InvokeStaticMethodRetFptr = nullptr;
    SetFieldValueFn SetFieldValueFptr = nullptr;
    GetFieldValueFn GetFieldValueFptr = nullptr;
    SetPropertyValueFn SetPropertyValueFptr = nullptr;
    GetPropertyValueFn GetPropertyValueFptr = nullptr;
    DestroyObjectFn DestroyObjectFptr = nullptr;

    CollectGarbageFn CollectGarbageFptr = nullptr;
    WaitForPendingFinalizersFn WaitForPendingFinalizersFptr = nullptr;
};

inline ManagedFunctions s_ManagedFunctions;

struct Memory {
    static void* AllocHGlobal(size_t InSize);
    static void FreeHGlobal(void* InPtr);

    static CharType* StringToCoTaskMemAuto(StringView InString);
    static void FreeCoTaskMem(void* InMemory);
};

template <typename TValue>
class Array {
public:
    static Array New(int32_t InLength) {
        Array<TValue> result;
        if (InLength > 0) {
            result.m_Ptr = static_cast<TValue*>(Memory::AllocHGlobal(InLength * sizeof(TValue)));
            result.m_Length = InLength;
        }
        return result;
    }

    static Array New(const std::vector<TValue>& InValues) {
        Array<TValue> result;

        if (!InValues.empty()) {
            result.m_Ptr = static_cast<TValue*>(Memory::AllocHGlobal(InValues.size() * sizeof(TValue)));
            result.m_Length = static_cast<int32_t>(InValues.size());
            memcpy(result.m_Ptr, InValues.data(), InValues.size() * sizeof(TValue));
        }

        return result;
    }

    static Array New(std::initializer_list<TValue> InValues) {
        Array result;

        if (InValues.size() > 0) {
            result.m_Ptr = static_cast<TValue*>(Memory::AllocHGlobal(InValues.size() * sizeof(TValue)));
            result.m_Length = static_cast<int32_t>(InValues.size());
            memcpy(result.m_Ptr, InValues.begin(), InValues.size() * sizeof(TValue));
        }

        return result;
    }

    static void Free(Array InArray) {
        if (!InArray.m_Ptr || InArray.m_Length == 0) return;

        Memory::FreeHGlobal(InArray.m_Ptr);
        InArray.m_Ptr = nullptr;
        InArray.m_Length = 0;
    }

    void Assign(const Array& InOther) { memcpy(m_Ptr, InOther.m_Ptr, InOther.m_Length * sizeof(TValue)); }

    bool IsEmpty() const { return m_Length == 0 || m_Ptr == nullptr; }

    TValue& operator[](size_t InIndex) { return m_Ptr[InIndex]; }
    const TValue& operator[](size_t InIndex) const { return m_Ptr[InIndex]; }

    size_t Length() const { return m_Length; }
    size_t ByteLength() const { return m_Length * sizeof(TValue); }

    TValue* Data() { return m_Ptr; }
    const TValue* Data() const { return m_Ptr; }

    TValue* begin() { return m_Ptr; }
    TValue* end() { return m_Ptr + m_Length; }

    const TValue* begin() const { return m_Ptr; }
    const TValue* end() const { return m_Ptr + m_Length; }

    const TValue* cbegin() const { return m_Ptr; }
    const TValue* cend() const { return m_Ptr + m_Length; }

private:
    TValue* m_Ptr = nullptr;
    int32_t m_Length = 0;
    Bool32 m_IsDisposed = false;
};

enum StatusCode {
    // Success
    Success = 0,
    Success_HostAlreadyInitialized = 0x00000001,
    Success_DifferentRuntimeProperties = 0x00000002,

    // Failure
    InvalidArgFailure = 0x80008081,
    CoreHostLibLoadFailure = 0x80008082,
    CoreHostLibMissingFailure = 0x80008083,
    CoreHostEntryPointFailure = 0x80008084,
    CoreHostCurHostFindFailure = 0x80008085,

    CoreClrResolveFailure = 0x80008087,
    CoreClrBindFailure = 0x80008088,
    CoreClrInitFailure = 0x80008089,
    CoreClrExeFailure = 0x8000808a,
    ResolverInitFailure = 0x8000808b,
    ResolverResolveFailure = 0x8000808c,
    LibHostCurExeFindFailure = 0x8000808d,
    LibHostInitFailure = 0x8000808e,

    LibHostExecModeFailure = 0x80008090,
    LibHostSdkFindFailure = 0x80008091,
    LibHostInvalidArgs = 0x80008092,
    InvalidConfigFile = 0x80008093,
    AppArgNotRunnable = 0x80008094,
    AppHostExeNotBoundFailure = 0x80008095,
    FrameworkMissingFailure = 0x80008096,
    HostApiFailed = 0x80008097,
    HostApiBufferTooSmall = 0x80008098,
    LibHostUnknownCommand = 0x80008099,
    LibHostAppRootFindFailure = 0x8000809a,
    SdkResolverResolveFailure = 0x8000809b,
    FrameworkCompatFailure = 0x8000809c,
    FrameworkCompatRetry = 0x8000809d,

    BundleExtractionFailure = 0x8000809f,
    BundleExtractionIOError = 0x800080a0,
    LibHostDuplicateProperty = 0x800080a1,
    HostApiUnsupportedVersion = 0x800080a2,
    HostInvalidState = 0x800080a3,
    HostPropertyNotFound = 0x800080a4,
    CoreHostIncompatibleConfig = 0x800080a5,
    HostApiUnsupportedScenario = 0x800080a6,
    HostFeatureDisabled = 0x800080a7,
};

enum class MessageLevel { Info = 1 << 0, Warning = 1 << 1, Error = 1 << 2, All = Info | Warning | Error };

template <typename T>
constexpr auto ToUnderlying(T InValue) {
    return static_cast<std::underlying_type_t<T>>(InValue);
}

constexpr MessageLevel operator|(const MessageLevel InLHS, const MessageLevel InRHS) noexcept { return static_cast<MessageLevel>(ToUnderlying(InLHS) | ToUnderlying(InRHS)); }
constexpr bool operator&(const MessageLevel InLHS, const MessageLevel InRHS) noexcept { return (ToUnderlying(InLHS) & ToUnderlying(InRHS)) != 0; }
constexpr MessageLevel operator~(const MessageLevel InValue) noexcept { return static_cast<MessageLevel>(~ToUnderlying(InValue)); }
constexpr MessageLevel& operator|=(MessageLevel& InLHS, const MessageLevel& InRHS) noexcept { return (InLHS = (InLHS | InRHS)); }

using MessageCallbackFn = std::function<void(std::string_view, MessageLevel)>;

class MethodInfo {
public:
    String GetName() const;

    Type& GetReturnType();
    const std::vector<Type*>& GetParameterTypes();

    TypeAccessibility GetAccessibility() const;

    std::vector<Attribute> GetAttributes() const;

private:
    ManagedHandle m_Handle = -1;
    Type* m_ReturnType = nullptr;
    std::vector<Type*> m_ParameterTypes;

    friend class Type;
};

class FieldInfo {
public:
    String GetName() const;
    Type& GetType();

    TypeAccessibility GetAccessibility() const;

    std::vector<Attribute> GetAttributes() const;

private:
    ManagedHandle m_Handle = -1;
    Type* m_Type = nullptr;

    friend class Type;
};

enum class ManagedType {
    Unknown,

    SByte,
    Byte,
    Short,
    UShort,
    Int,
    UInt,
    Long,
    ULong,

    Float,
    Double,

    Bool,

    Pointer,
};

// 有没有其他更好的方法
template <typename TArg>
constexpr ManagedType GetManagedType() {
    if constexpr (std::is_pointer_v<std::remove_reference_t<TArg>>)
        return ManagedType::Pointer;
    else if constexpr (std::same_as<TArg, uint8_t> || std::same_as<TArg, std::byte>)
        return ManagedType::Byte;
    else if constexpr (std::same_as<TArg, uint16_t>)
        return ManagedType::UShort;
    else if constexpr (std::same_as<TArg, uint32_t> || (std::same_as<TArg, unsigned long> && sizeof(TArg) == 4))
        return ManagedType::UInt;
    else if constexpr (std::same_as<TArg, uint64_t> || (std::same_as<TArg, unsigned long> && sizeof(TArg) == 8))
        return ManagedType::ULong;
    else if constexpr (std::same_as<TArg, char8_t>)
        return ManagedType::SByte;
    else if constexpr (std::same_as<TArg, int16_t>)
        return ManagedType::Short;
    else if constexpr (std::same_as<TArg, int32_t> || (std::same_as<TArg, long> && sizeof(TArg) == 4))
        return ManagedType::Int;
    else if constexpr (std::same_as<TArg, int64_t> || (std::same_as<TArg, long> && sizeof(TArg) == 8))
        return ManagedType::Long;
    else if constexpr (std::same_as<TArg, float>)
        return ManagedType::Float;
    else if constexpr (std::same_as<TArg, double>)
        return ManagedType::Double;
    else if constexpr (std::same_as<TArg, bool>)
        return ManagedType::Bool;
    else
        return ManagedType::Unknown;
}

template <typename TArg, size_t TIndex>
inline void AddToArrayI(const void** InArgumentsArr, ManagedType* InParameterTypes, TArg&& InArg) {
    InParameterTypes[TIndex] = GetManagedType<TArg>();

    if constexpr (std::is_pointer_v<std::remove_reference_t<TArg>>) {
        InArgumentsArr[TIndex] = reinterpret_cast<const void*>(InArg);
    } else {
        InArgumentsArr[TIndex] = reinterpret_cast<const void*>(&InArg);
    }
}

template <typename... TArgs, size_t... TIndices>
inline void AddToArray(const void** InArgumentsArr, ManagedType* InParameterTypes, TArgs&&... InArgs, const std::index_sequence<TIndices...>&) {
    (AddToArrayI<TArgs, TIndices>(InArgumentsArr, InParameterTypes, std::forward<TArgs>(InArgs)), ...);
}

class ManagedObject {
public:
    template <typename TReturn, typename... TArgs>
    TReturn InvokeMethod(std::string_view InMethodName, TArgs&&... InParameters) {
        constexpr size_t parameterCount = sizeof...(InParameters);

        TReturn result;

        if constexpr (parameterCount > 0) {
            const void* parameterValues[parameterCount];
            ManagedType parameterTypes[parameterCount];
            AddToArray<TArgs...>(parameterValues, parameterTypes, std::forward<TArgs>(InParameters)..., std::make_index_sequence<parameterCount>{});
            InvokeMethodRetInternal(InMethodName, parameterValues, parameterTypes, parameterCount, &result);
        } else {
            InvokeMethodRetInternal(InMethodName, nullptr, nullptr, 0, &result);
        }

        return result;
    }

    template <typename... TArgs>
    void InvokeMethod(std::string_view InMethodName, TArgs&&... InParameters) {
        constexpr size_t parameterCount = sizeof...(InParameters);

        if constexpr (parameterCount > 0) {
            const void* parameterValues[parameterCount];
            ManagedType parameterTypes[parameterCount];
            AddToArray<TArgs...>(parameterValues, parameterTypes, std::forward<TArgs>(InParameters)..., std::make_index_sequence<parameterCount>{});
            InvokeMethodInternal(InMethodName, parameterValues, parameterTypes, parameterCount);
        } else {
            InvokeMethodInternal(InMethodName, nullptr, nullptr, 0);
        }
    }

    template <typename TValue>
    void SetFieldValue(std::string_view InFieldName, TValue InValue) {
        SetFieldValueRaw(InFieldName, &InValue);
    }

    template <typename TReturn>
    TReturn GetFieldValue(std::string_view InFieldName) {
        TReturn result;
        GetFieldValueRaw(InFieldName, &result);
        return result;
    }

    template <typename TValue>
    void SetPropertyValue(std::string_view InPropertyName, TValue InValue) {
        SetPropertyValueRaw(InPropertyName, &InValue);
    }

    template <typename TReturn>
    TReturn GetPropertyValue(std::string_view InPropertyName) {
        TReturn result;
        GetPropertyValueRaw(InPropertyName, &result);
        return result;
    }

    void SetFieldValueRaw(std::string_view InFieldName, void* InValue) const;
    void GetFieldValueRaw(std::string_view InFieldName, void* OutValue) const;
    void SetPropertyValueRaw(std::string_view InPropertyName, void* InValue) const;
    void GetPropertyValueRaw(std::string_view InPropertyName, void* OutValue) const;

    const Type& GetType() const;

    void Destroy();

private:
    void InvokeMethodInternal(std::string_view InMethodName, const void** InParameters, const ManagedType* InParameterTypes, size_t InLength) const;
    void InvokeMethodRetInternal(std::string_view InMethodName, const void** InParameters, const ManagedType* InParameterTypes, size_t InLength, void* InResultStorage) const;

private:
    void* m_Handle = nullptr;
    Type* m_Type;

private:
    friend class ManagedAssembly;
    friend class Type;
};

class PropertyInfo {
public:
    String GetName() const;
    Type& GetType();

    std::vector<Attribute> GetAttributes() const;

private:
    ManagedHandle m_Handle = -1;
    Type* m_Type = nullptr;

    friend class Type;
};

class Type {
public:
    String GetFullName() const;
    String GetAssemblyQualifiedName() const;

    Type& GetBaseType();

    int32_t GetSize() const;

    bool IsSubclassOf(const Type& InOther) const;
    bool IsAssignableTo(const Type& InOther) const;
    bool IsAssignableFrom(const Type& InOther) const;

    std::vector<MethodInfo> GetMethods() const;
    std::vector<FieldInfo> GetFields() const;
    std::vector<PropertyInfo> GetProperties() const;

    bool HasAttribute(const Type& InAttributeType) const;
    std::vector<Attribute> GetAttributes() const;

    ManagedType GetManagedType() const;

    bool IsSZArray() const;
    Type& GetElementType();

    bool operator==(const Type& InOther) const;

    operator bool() const { return m_Id != -1; }

    TypeId GetTypeId() const { return m_Id; }

public:
    template <typename... TArgs>
    ManagedObject CreateInstance(TArgs&&... InArguments) {
        constexpr size_t argumentCount = sizeof...(InArguments);

        ManagedObject result;

        if constexpr (argumentCount > 0) {
            const void* argumentsArr[argumentCount];
            ManagedType argumentTypes[argumentCount];
            AddToArray<TArgs...>(argumentsArr, argumentTypes, std::forward<TArgs>(InArguments)..., std::make_index_sequence<argumentCount>{});
            result = CreateInstanceInternal(argumentsArr, argumentTypes, argumentCount);
        } else {
            result = CreateInstanceInternal(nullptr, nullptr, 0);
        }

        return result;
    }

    template <typename TReturn, typename... TArgs>
    TReturn InvokeStaticMethod(std::string_view InMethodName, TArgs&&... InParameters) {
        constexpr size_t parameterCount = sizeof...(InParameters);

        TReturn result;

        if constexpr (parameterCount > 0) {
            const void* parameterValues[parameterCount];
            ManagedType parameterTypes[parameterCount];
            AddToArray<TArgs...>(parameterValues, parameterTypes, std::forward<TArgs>(InParameters)..., std::make_index_sequence<parameterCount>{});
            InvokeStaticMethodRetInternal(InMethodName, parameterValues, parameterTypes, parameterCount, &result);
        } else {
            InvokeStaticMethodRetInternal(InMethodName, nullptr, nullptr, 0, &result);
        }

        return result;
    }

    template <typename... TArgs>
    void InvokeStaticMethod(std::string_view InMethodName, TArgs&&... InParameters) {
        constexpr size_t parameterCount = sizeof...(InParameters);

        if constexpr (parameterCount > 0) {
            const void* parameterValues[parameterCount];
            ManagedType parameterTypes[parameterCount];
            AddToArray<TArgs...>(parameterValues, parameterTypes, std::forward<TArgs>(InParameters)..., std::make_index_sequence<parameterCount>{});
            InvokeStaticMethodInternal(InMethodName, parameterValues, parameterTypes, parameterCount);
        } else {
            InvokeStaticMethodInternal(InMethodName, nullptr, nullptr, 0);
        }
    }

private:
    ManagedObject CreateInstanceInternal(const void** InParameters, const ManagedType* InParameterTypes, size_t InLength);
    void InvokeStaticMethodInternal(std::string_view InMethodName, const void** InParameters, const ManagedType* InParameterTypes, size_t InLength) const;
    void InvokeStaticMethodRetInternal(std::string_view InMethodName, const void** InParameters, const ManagedType* InParameterTypes, size_t InLength, void* InResultStorage) const;

private:
    TypeId m_Id = -1;
    Type* m_BaseType = nullptr;
    Type* m_ElementType = nullptr;

    friend class HostInstance;
    friend class ManagedAssembly;
    friend class AssemblyLoadContext;
    friend class MethodInfo;
    friend class FieldInfo;
    friend class PropertyInfo;
    friend class Attribute;
    friend class ReflectionType;
};

class ReflectionType {
public:
    operator Type&() const;

private:
    TypeId m_TypeID;
};

template <typename TElement, size_t PageSize = 256>
class StableVector {
public:
    StableVector() = default;

    StableVector(const StableVector& other) {
        other.ForEach([this](const TElement& elem) mutable { EmplaceBackNoLock().second = elem; });
    }

    ~StableVector() {
        for (size_t i = 0; i < m_PageCount; i++) delete m_PageTable[i];
    }

    StableVector& operator=(const StableVector& other) {
        Clear();

        other.ForEach([this](const TElement& elem) mutable { EmplaceBackNoLock().second = elem; });

        return *this;
    }

    void Clear() {
        for (size_t i = 0; i < m_PageCount; i++) delete m_PageTable[i];

        m_ElementCount = 0;
        m_Capacity = 0;
        m_PageCount = 0;
    }

    TElement& operator[](size_t index) {
        size_t pageIndex = index / PageSize;
        return m_PageTable[pageIndex]->Elements[index - (pageIndex * PageSize)];
    }

    const TElement& operator[](size_t index) const {
        size_t pageIndex = index / PageSize;
        return m_PageTable[pageIndex]->Elements[index - (pageIndex * PageSize)];
    }

    size_t GetElementCount() const { return m_ElementCount; }

    std::pair<uint32_t, TElement&> Insert(TElement&& InElement) {
        size_t pageIndex = m_ElementCount / PageSize;

        if (m_ElementCount >= m_Capacity) {
            std::scoped_lock lock(m_Mutex);

            if (m_ElementCount >= m_Capacity) {
                auto* newPage = new Page();

                if (pageIndex >= m_PageCount) {
                    auto oldPages = m_PageCount;
                    m_PageCount = (std::max)(uint64_t(16), m_PageCount * 2);
                    auto newPageTable = std::make_unique<Page*[]>(m_PageCount);
                    std::memcpy(newPageTable.get(), m_PageTable.load(), oldPages * sizeof(void*));
                    m_PageTable.exchange(newPageTable.get());
                    m_PageTables.push_back(std::move(newPageTable));
                }

                m_PageTable[pageIndex] = newPage;

                m_Capacity += PageSize;
            }
        }

        uint32_t index = (++m_ElementCount - 1);
        m_PageTable[pageIndex]->Elements[index - (pageIndex * PageSize)] = std::move(InElement);
        return {index, m_PageTable[pageIndex]->Elements[index - (pageIndex * PageSize)]};
    }

    std::pair<uint32_t, TElement&> InsertNoLock(TElement&& InElement) {
        size_t pageIndex = m_ElementCount / PageSize;

        if (m_ElementCount >= m_Capacity) {
            std::scoped_lock lock(m_Mutex);

            if (m_ElementCount >= m_Capacity) {
                auto* newPage = new Page();

                if (pageIndex >= m_PageCount) {
                    auto oldPages = m_PageCount;
                    m_PageCount = (std::max)(uint64_t(16), m_PageCount * 2);
                    auto newPageTable = std::make_unique<Page*[]>(m_PageCount);
                    std::memcpy(newPageTable.get(), m_PageTable.load(), oldPages * sizeof(void*));
                    m_PageTable.exchange(newPageTable.get());
                    m_PageTables.push_back(std::move(newPageTable));
                }

                m_PageTable[pageIndex] = newPage;

                m_Capacity += PageSize;
            }
        }

        uint32_t index = (++m_ElementCount - 1);
        m_PageTable[pageIndex]->Elements[index - (pageIndex * PageSize)] = std::move(InElement);
        return {index, m_PageTable[pageIndex]->Elements[index - (pageIndex * PageSize)]};
    }

    std::pair<uint32_t, TElement&> EmplaceBack() {
        size_t pageIndex = m_ElementCount / PageSize;

        if (m_ElementCount >= m_Capacity) {
            auto* newPage = new Page();

            if (pageIndex >= m_PageCount) {
                auto oldPages = m_PageCount;
                m_PageCount = (std::max)(uint64_t(16), m_PageCount * 2);
                auto newPageTable = std::make_unique<Page*[]>(m_PageCount);
                std::memcpy(newPageTable.get(), m_PageTable.load(), oldPages * sizeof(void*));
                m_PageTable.exchange(newPageTable.get());
                m_PageTables.push_back(std::move(newPageTable));
            }

            m_PageTable[pageIndex] = newPage;

            m_Capacity += PageSize;
        }

        uint32_t index = (++m_ElementCount - 1);
        return {index, m_PageTable[pageIndex]->Elements[index - (pageIndex * PageSize)]};
    }

    std::pair<uint32_t, TElement&> EmplaceBackNoLock() {
        size_t pageIndex = m_ElementCount / PageSize;

        if (m_ElementCount >= m_Capacity) {
            auto* newPage = new Page();

            if (pageIndex >= m_PageCount) {
                auto oldPages = m_PageCount;
                m_PageCount = (std::max)(uint64_t(16), m_PageCount * 2);
                auto newPageTable = std::make_unique<Page*[]>(m_PageCount);
                std::memcpy(newPageTable.get(), m_PageTable.load(), oldPages * sizeof(void*));
                m_PageTable.exchange(newPageTable.get());
                m_PageTables.push_back(std::move(newPageTable));
            }

            m_PageTable[pageIndex] = newPage;

            m_Capacity += PageSize;
        }

        uint32_t index = (++m_ElementCount - 1);
        return {index, m_PageTable[pageIndex]->Elements[index - (pageIndex * PageSize)]};
    }

    template <typename Fn>
    void ForEach(Fn&& fn) {
        for (uint32_t i = 0; i < m_ElementCount; ++i) fn((*this)[i]);
    }

    template <typename Fn>
    void ForEach(Fn&& fn) const {
        for (uint32_t i = 0; i < m_ElementCount; ++i) fn((*this)[i]);
    }

private:
    struct Page {
        std::array<TElement, PageSize> Elements;
    };

    std::shared_mutex m_Mutex;
    std::list<std::unique_ptr<Page*[]>> m_PageTables;

    std::atomic<Page**> m_PageTable;
    std::atomic<uint32_t> m_ElementCount = 0;
    std::atomic<uint32_t> m_Capacity = 0;
    uint64_t m_PageCount = 0;
};

enum class AssemblyLoadStatus { Success, FileNotFound, FileLoadFailure, InvalidFilePath, InvalidAssembly, UnknownError };

class ManagedAssembly {
public:
    int32_t GetAssemblyID() const { return m_AssemblyID; }
    AssemblyLoadStatus GetLoadStatus() const { return m_LoadStatus; }
    std::string_view GetName() const { return m_Name; }

    void AddInternalCall(std::string_view InClassName, std::string_view InVariableName, void* InFunctionPtr);
    void UploadInternalCalls();

    Type& GetType(std::string_view InClassName) const;
    const std::vector<Type*>& GetTypes() const;

private:
    HostInstance* m_Host = nullptr;
    int32_t m_AssemblyID = -1;
    AssemblyLoadStatus m_LoadStatus = AssemblyLoadStatus::UnknownError;
    std::string m_Name;

#if defined(NEKO_CS_WIDE_CHARS)
    std::vector<std::wstring> m_InternalCallNameStorage;
#else
    std::vector<std::string> m_InternalCallNameStorage;
#endif

    std::vector<InternalCall> m_InternalCalls;

    std::vector<Type*> m_Types;

    friend class HostInstance;
    friend class AssemblyLoadContext;
};

class AssemblyLoadContext {
public:
    ManagedAssembly& LoadAssembly(std::string_view InFilePath);
    const StableVector<ManagedAssembly>& GetLoadedAssemblies() const { return m_LoadedAssemblies; }

private:
    int32_t m_ContextId;
    StableVector<ManagedAssembly> m_LoadedAssemblies;

    HostInstance* m_Host = nullptr;

    friend class HostInstance;
};

using ExceptionCallbackFn = std::function<void(std::string_view)>;

struct HostSettings {
    // path to runtimeconfig.json
    std::string NekoCSDirectory;

    MessageCallbackFn MessageCallback = nullptr;
    MessageLevel MessageFilter = MessageLevel::All;

    ExceptionCallbackFn ExceptionCallback = nullptr;
};

class HostInstance {
public:
    bool Initialize(HostSettings InSettings);
    void Shutdown();

    AssemblyLoadContext CreateAssemblyLoadContext(std::string_view InName);
    void UnloadAssemblyLoadContext(AssemblyLoadContext& InLoadContext);

private:
    void LoadHostFXR() const;
    bool InitializeNekoCSManaged();
    void LoadNekoCSFunctions();

    void* LoadNekoCSManagedFunctionPtr(const std::filesystem::path& InAssemblyPath, const CharType* InTypeName, const CharType* InMethodName,
                                       const CharType* InDelegateType = NEKO_CS_UNMANAGED_CALLERS_ONLY) const;

    template <typename TFunc>
    TFunc LoadNekoCSManagedFunctionPtr(const CharType* InTypeName, const CharType* InMethodName, const CharType* InDelegateType = NEKO_CS_UNMANAGED_CALLERS_ONLY) const {
        return (TFunc)LoadNekoCSManagedFunctionPtr(m_NekoCSManagedAssemblyPath, InTypeName, InMethodName, InDelegateType);
    }

private:
    HostSettings m_Settings;
    std::filesystem::path m_NekoCSManagedAssemblyPath;
    void* m_HostFXRContext = nullptr;
    bool m_Initialized = false;

    friend class AssemblyLoadContext;
};

class TypeCache {
public:
    static TypeCache& Get();

    Type* CacheType(Type&& InType);
    Type* GetTypeByName(std::string_view InName) const;
    Type* GetTypeByID(TypeId InTypeID) const;

private:
    StableVector<Type> m_Types;
    std::unordered_map<std::string, Type*> m_NameCache;
    std::unordered_map<TypeId, Type*> m_IDCache;
};

}  // namespace NekoCS

#endif