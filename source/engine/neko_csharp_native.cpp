#include "neko_csharp_native.h"

#include <codecvt>

#if defined(NEKO_PLATFORM_WIN)
#include <ShlObj_core.h>
#else
#include <dlfcn.h>
#endif

namespace neko_cs {

/*================================================================================
// String
================================================================================*/

String String::New(const char* InString) {
    String result;
    result.Assign(InString);
    return result;
}

String String::New(std::string_view InString) {
    String result;
    result.Assign(InString);
    return result;
}

void String::Free(String& InString) {
    if (InString.m_String == nullptr) return;

    Memory::FreeCoTaskMem(InString.m_String);
    InString.m_String = nullptr;
}

void String::Assign(std::string_view InString) {
    if (m_String != nullptr) Memory::FreeCoTaskMem(m_String);

    m_String = Memory::StringToCoTaskMemAuto(StringHelper::ConvertUtf8ToWide(InString));
}

String::operator std::string() const {
    StringView string(m_String);

#if defined(NEKO_CS_WIDE_CHARS)
    return StringHelper::ConvertWideToUtf8(string);
#else
    return std::string(string);
#endif
}

bool String::operator==(const String& InOther) const {
    if (m_String == InOther.m_String) return true;

    if (m_String == nullptr || InOther.m_String == nullptr) return false;

#if defined(NEKO_CS_WIDE_CHARS)
    return wcscmp(m_String, InOther.m_String) == 0;
#else
    return strcmp(m_String, InOther.m_String) == 0;
#endif
}

bool String::operator==(std::string_view InOther) const {
#if defined(NEKO_CS_WIDE_CHARS)
    auto str = StringHelper::ConvertUtf8ToWide(InOther);
    return wcscmp(m_String, str.data()) == 0;
#else !
    return strcmp(m_String, InOther.data()) == 0;
#endif
}

#if defined(NEKO_CS_WIDE_CHARS)
std::wstring StringHelper::ConvertUtf8ToWide(std::string_view InString) {
    int length = MultiByteToWideChar(CP_UTF8, 0, InString.data(), static_cast<int>(InString.length()), nullptr, 0);
    auto result = std::wstring(length, wchar_t(0));
    MultiByteToWideChar(CP_UTF8, 0, InString.data(), static_cast<int>(InString.length()), result.data(), length);
    return result;
}

std::string StringHelper::ConvertWideToUtf8(std::wstring_view InString) {
    int requiredLength = WideCharToMultiByte(CP_UTF8, 0, InString.data(), static_cast<int>(InString.length()), nullptr, 0, nullptr, nullptr);
    std::string result(requiredLength, 0);
    (void)WideCharToMultiByte(CP_UTF8, 0, InString.data(), static_cast<int>(InString.length()), result.data(), requiredLength, nullptr, nullptr);
    return result;
}

#else
std::string StringHelper::ConvertUtf8ToWide(std::string_view InString) { return std::string(InString); }

std::string StringHelper::ConvertWideToUtf8(std::string_view InString) { return std::string(InString); }
#endif

/*================================================================================
// TypeCache
================================================================================*/

TypeCache& TypeCache::Get() {
    static TypeCache s_Instance;
    return s_Instance;
}

Type* TypeCache::CacheType(Type&& InType) {
    Type* type = &m_Types.Insert(std::move(InType)).second;
    m_NameCache[type->GetFullName()] = type;
    m_IDCache[type->GetTypeId()] = type;
    return type;
}

Type* TypeCache::GetTypeByName(std::string_view InName) const {
    auto name = std::string(InName);
    return m_NameCache.contains(name) ? m_NameCache.at(name) : nullptr;
}

Type* TypeCache::GetTypeByID(TypeId InTypeID) const { return m_IDCache.contains(InTypeID) ? m_IDCache.at(InTypeID) : nullptr; }

/*================================================================================
// Type
================================================================================*/

String Type::GetFullName() const { return s_ManagedFunctions.GetFullTypeNameFptr(m_Id); }

String Type::GetAssemblyQualifiedName() const { return s_ManagedFunctions.GetAssemblyQualifiedNameFptr(m_Id); }

Type& Type::GetBaseType() {
    if (!m_BaseType) {
        Type baseType;
        s_ManagedFunctions.GetBaseTypeFptr(m_Id, &baseType.m_Id);
        m_BaseType = TypeCache::Get().CacheType(std::move(baseType));
    }

    return *m_BaseType;
}

int32_t Type::GetSize() const { return s_ManagedFunctions.GetTypeSizeFptr(m_Id); }

bool Type::IsSubclassOf(const Type& InOther) const { return s_ManagedFunctions.IsTypeSubclassOfFptr(m_Id, InOther.m_Id); }

bool Type::IsAssignableTo(const Type& InOther) const { return s_ManagedFunctions.IsTypeAssignableToFptr(m_Id, InOther.m_Id); }

bool Type::IsAssignableFrom(const Type& InOther) const { return s_ManagedFunctions.IsTypeAssignableFromFptr(m_Id, InOther.m_Id); }

std::vector<MethodInfo> Type::GetMethods() const {
    int32_t methodCount = 0;
    s_ManagedFunctions.GetTypeMethodsFptr(m_Id, nullptr, &methodCount);
    std::vector<ManagedHandle> handles(methodCount);
    s_ManagedFunctions.GetTypeMethodsFptr(m_Id, handles.data(), &methodCount);

    std::vector<MethodInfo> methods(handles.size());
    for (size_t i = 0; i < handles.size(); i++) methods[i].m_Handle = handles[i];

    return methods;
}

std::vector<FieldInfo> Type::GetFields() const {
    int32_t fieldCount = 0;
    s_ManagedFunctions.GetTypeFieldsFptr(m_Id, nullptr, &fieldCount);
    std::vector<ManagedHandle> handles(fieldCount);
    s_ManagedFunctions.GetTypeFieldsFptr(m_Id, handles.data(), &fieldCount);

    std::vector<FieldInfo> fields(handles.size());
    for (size_t i = 0; i < handles.size(); i++) fields[i].m_Handle = handles[i];

    return fields;
}

std::vector<PropertyInfo> Type::GetProperties() const {
    int32_t propertyCount = 0;
    s_ManagedFunctions.GetTypePropertiesFptr(m_Id, nullptr, &propertyCount);
    std::vector<ManagedHandle> handles(propertyCount);
    s_ManagedFunctions.GetTypePropertiesFptr(m_Id, handles.data(), &propertyCount);

    std::vector<PropertyInfo> properties(handles.size());
    for (size_t i = 0; i < handles.size(); i++) properties[i].m_Handle = handles[i];

    return properties;
}

bool Type::HasAttribute(const Type& InAttributeType) const { return s_ManagedFunctions.HasTypeAttributeFptr(m_Id, InAttributeType.m_Id); }

std::vector<Attribute> Type::GetAttributes() const {
    int32_t attributeCount;
    s_ManagedFunctions.GetTypeAttributesFptr(m_Id, nullptr, &attributeCount);
    std::vector<ManagedHandle> attributeHandles(attributeCount);
    s_ManagedFunctions.GetTypeAttributesFptr(m_Id, attributeHandles.data(), &attributeCount);

    std::vector<Attribute> result(attributeHandles.size());
    for (size_t i = 0; i < attributeHandles.size(); i++) result[i].m_Handle = attributeHandles[i];

    return result;
}

ManagedType Type::GetManagedType() const { return s_ManagedFunctions.GetTypeManagedTypeFptr(m_Id); }

bool Type::IsSZArray() const { return s_ManagedFunctions.IsTypeSZArrayFptr(m_Id); }

Type& Type::GetElementType() {
    if (!m_ElementType) {
        Type elementType;
        s_ManagedFunctions.GetElementTypeFptr(m_Id, &elementType.m_Id);
        m_ElementType = TypeCache::Get().CacheType(std::move(elementType));
    }

    return *m_ElementType;
}

bool Type::operator==(const Type& InOther) const { return m_Id == InOther.m_Id; }

ManagedObject Type::CreateInstanceInternal(const void** InParameters, const ManagedType* InParameterTypes, size_t InLength) {
    ManagedObject result;
    result.m_Handle = s_ManagedFunctions.CreateObjectFptr(m_Id, false, InParameters, InParameterTypes, static_cast<int32_t>(InLength));
    result.m_Type = this;
    return result;
}

void Type::InvokeStaticMethodInternal(std::string_view InMethodName, const void** InParameters, const ManagedType* InParameterTypes, size_t InLength) const {
    auto methodName = String::New(InMethodName);
    s_ManagedFunctions.InvokeStaticMethodFptr(m_Id, methodName, InParameters, InParameterTypes, static_cast<int32_t>(InLength));
    String::Free(methodName);
}

void Type::InvokeStaticMethodRetInternal(std::string_view InMethodName, const void** InParameters, const ManagedType* InParameterTypes, size_t InLength, void* InResultStorage) const {
    auto methodName = String::New(InMethodName);
    s_ManagedFunctions.InvokeStaticMethodRetFptr(m_Id, methodName, InParameters, InParameterTypes, static_cast<int32_t>(InLength), InResultStorage);
    String::Free(methodName);
}

ReflectionType::operator Type&() const {
    static Type s_NullType;

    auto* result = TypeCache::Get().GetTypeByID(m_TypeID);

    if (result == nullptr) {
        Type type;
        type.m_Id = m_TypeID;
        result = TypeCache::Get().CacheType(std::move(type));
    }

    return result != nullptr ? *result : s_NullType;
}

String PropertyInfo::GetName() const { return s_ManagedFunctions.GetPropertyInfoNameFptr(m_Handle); }

Type& PropertyInfo::GetType() {
    if (!m_Type) {
        Type propertyType;
        s_ManagedFunctions.GetPropertyInfoTypeFptr(m_Handle, &propertyType.m_Id);
        m_Type = TypeCache::Get().CacheType(std::move(propertyType));
    }

    return *m_Type;
}

std::vector<Attribute> PropertyInfo::GetAttributes() const {
    int32_t attributeCount;
    s_ManagedFunctions.GetPropertyInfoAttributesFptr(m_Handle, nullptr, &attributeCount);
    std::vector<ManagedHandle> attributeHandles(attributeCount);
    s_ManagedFunctions.GetPropertyInfoAttributesFptr(m_Handle, attributeHandles.data(), &attributeCount);

    std::vector<Attribute> result(attributeHandles.size());
    for (size_t i = 0; i < attributeHandles.size(); i++) result[i].m_Handle = attributeHandles[i];

    return result;
}

/*================================================================================
// MethodInfo
================================================================================*/

String MethodInfo::GetName() const { return s_ManagedFunctions.GetMethodInfoNameFptr(m_Handle); }

Type& MethodInfo::GetReturnType() {
    if (!m_ReturnType) {
        Type returnType;
        s_ManagedFunctions.GetMethodInfoReturnTypeFptr(m_Handle, &returnType.m_Id);
        m_ReturnType = TypeCache::Get().CacheType(std::move(returnType));
    }

    return *m_ReturnType;
}

const std::vector<Type*>& MethodInfo::GetParameterTypes() {
    if (m_ParameterTypes.empty()) {
        int32_t parameterCount;
        s_ManagedFunctions.GetMethodInfoParameterTypesFptr(m_Handle, nullptr, &parameterCount);
        std::vector<TypeId> parameterTypes(parameterCount);
        s_ManagedFunctions.GetMethodInfoParameterTypesFptr(m_Handle, parameterTypes.data(), &parameterCount);

        m_ParameterTypes.resize(parameterTypes.size());

        for (size_t i = 0; i < parameterTypes.size(); i++) {
            Type type;
            type.m_Id = parameterTypes[i];
            m_ParameterTypes[i] = TypeCache::Get().CacheType(std::move(type));
        }
    }

    return m_ParameterTypes;
}

TypeAccessibility MethodInfo::GetAccessibility() const { return s_ManagedFunctions.GetMethodInfoAccessibilityFptr(m_Handle); }

std::vector<Attribute> MethodInfo::GetAttributes() const {
    int32_t attributeCount;
    s_ManagedFunctions.GetMethodInfoAttributesFptr(m_Handle, nullptr, &attributeCount);
    std::vector<ManagedHandle> attributeHandles(attributeCount);
    s_ManagedFunctions.GetMethodInfoAttributesFptr(m_Handle, attributeHandles.data(), &attributeCount);

    std::vector<Attribute> result(attributeHandles.size());
    for (size_t i = 0; i < attributeHandles.size(); i++) result[i].m_Handle = attributeHandles[i];

    return result;
}

/*================================================================================
// Memory
================================================================================*/

void* Memory::AllocHGlobal(size_t InSize) {
#if defined(NEKO_PLATFORM_WIN)
    return LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, InSize);
#else
    return malloc(InSize);
#endif
}

void Memory::FreeHGlobal(void* InPtr) {
#if defined(NEKO_PLATFORM_WIN)
    LocalFree(InPtr);
#else
    free(InPtr);
#endif
}

CharType* Memory::StringToCoTaskMemAuto(StringView InString) {
    size_t length = InString.length() + 1;
    size_t size = length * sizeof(CharType);

#if defined(NEKO_PLATFORM_WIN)
    auto* buffer = static_cast<CharType*>(CoTaskMemAlloc(size));

    if (buffer != nullptr) {
        memset(buffer, 0xCE, size);
        wcscpy(buffer, InString.data());
    }
#else
    auto* buffer = static_cast<CharType*>(AllocHGlobal(size));

    if (buffer != nullptr) {
        memset(buffer, 0, size);
        strcpy(buffer, InString.data());
    }
#endif

    return buffer;
}

void Memory::FreeCoTaskMem(void* InMemory) {
#if defined(NEKO_PLATFORM_WIN)
    CoTaskMemFree(InMemory);
#else
    FreeHGlobal(InMemory);
#endif
}

void ManagedObject::InvokeMethodInternal(std::string_view InMethodName, const void** InParameters, const ManagedType* InParameterTypes, size_t InLength) const {
    // 如果在此函数中遇到异常很可能是因为在 Visual Studio 中使用仅本机调试器类型
    auto methodName = String::New(InMethodName);
    s_ManagedFunctions.InvokeMethodFptr(m_Handle, methodName, InParameters, InParameterTypes, static_cast<int32_t>(InLength));
    String::Free(methodName);
}

void ManagedObject::InvokeMethodRetInternal(std::string_view InMethodName, const void** InParameters, const ManagedType* InParameterTypes, size_t InLength, void* InResultStorage) const {
    auto methodName = String::New(InMethodName);
    s_ManagedFunctions.InvokeMethodRetFptr(m_Handle, methodName, InParameters, InParameterTypes, static_cast<int32_t>(InLength), InResultStorage);
    String::Free(methodName);
}

void ManagedObject::SetFieldValueRaw(std::string_view InFieldName, void* InValue) const {
    auto fieldName = String::New(InFieldName);
    s_ManagedFunctions.SetFieldValueFptr(m_Handle, fieldName, InValue);
    String::Free(fieldName);
}

void ManagedObject::GetFieldValueRaw(std::string_view InFieldName, void* OutValue) const {
    auto fieldName = String::New(InFieldName);
    s_ManagedFunctions.GetFieldValueFptr(m_Handle, fieldName, OutValue);
    String::Free(fieldName);
}

void ManagedObject::SetPropertyValueRaw(std::string_view InPropertyName, void* InValue) const {
    auto propertyName = String::New(InPropertyName);
    s_ManagedFunctions.SetPropertyValueFptr(m_Handle, propertyName, InValue);
    String::Free(propertyName);
}

void ManagedObject::GetPropertyValueRaw(std::string_view InPropertyName, void* OutValue) const {
    auto propertyName = String::New(InPropertyName);
    s_ManagedFunctions.GetPropertyValueFptr(m_Handle, propertyName, OutValue);
    String::Free(propertyName);
}

const Type& ManagedObject::GetType() const { return *m_Type; }

void ManagedObject::Destroy() {
    if (!m_Handle) return;

    s_ManagedFunctions.DestroyObjectFptr(m_Handle);
    m_Handle = nullptr;
    m_Type = nullptr;
}

/*================================================================================
// Host
================================================================================*/

struct CoreCLRFunctions {
    hostfxr_set_error_writer_fn SetHostFXRErrorWriter = nullptr;
    hostfxr_initialize_for_runtime_config_fn InitHostFXRForRuntimeConfig = nullptr;
    hostfxr_get_runtime_delegate_fn GetRuntimeDelegate = nullptr;
    hostfxr_close_fn CloseHostFXR = nullptr;
    load_assembly_and_get_function_pointer_fn GetManagedFunctionPtr = nullptr;
};
static CoreCLRFunctions s_CoreCLRFunctions;

MessageCallbackFn MessageCallback = nullptr;
MessageLevel MessageFilter;
ExceptionCallbackFn ExceptionCallback = nullptr;

void DefaultMessageCallback(std::string_view InMessage, MessageLevel InLevel) {
    const char* level = "";

    switch (InLevel) {
        case MessageLevel::Info:
            neko_log_info("[csharp] %s", InMessage.data());
            break;
        case MessageLevel::Warning:
            neko_log_warning("[csharp] %s", InMessage.data());
            break;
        case MessageLevel::Error:
            neko_log_error("[csharp] %s", InMessage.data());
            break;
    }
}

bool HostInstance::Initialize(HostSettings InSettings) {
    NEKO_CS_VERIFY(!m_Initialized);

    LoadHostFXR();

    // Setup settings
    m_Settings = std::move(InSettings);

    if (!m_Settings.MessageCallback) m_Settings.MessageCallback = DefaultMessageCallback;
    MessageCallback = m_Settings.MessageCallback;
    MessageFilter = m_Settings.MessageFilter;

    s_CoreCLRFunctions.SetHostFXRErrorWriter([](const CharType* InMessage) {
        auto message = StringHelper::ConvertWideToUtf8(InMessage);
        MessageCallback(message, MessageLevel::Error);
    });

    m_NekoCSManagedAssemblyPath = std::filesystem::path(m_Settings.NekoCSDirectory) / "Neko.Managed.dll";

    if (!std::filesystem::exists(m_NekoCSManagedAssemblyPath)) {
        MessageCallback("Failed to find Neko.Managed.dll", MessageLevel::Error);
        return false;
    }

    m_Initialized = InitializeNekoCSManaged();

    return m_Initialized;
}

void HostInstance::Shutdown() { s_CoreCLRFunctions.CloseHostFXR(m_HostFXRContext); }

AssemblyLoadContext HostInstance::CreateAssemblyLoadContext(std::string_view InName) {
    ScopedString name = String::New(InName);
    AssemblyLoadContext alc;
    alc.m_ContextId = s_ManagedFunctions.CreateAssemblyLoadContextFptr(name);
    alc.m_Host = this;
    return alc;
}

void HostInstance::UnloadAssemblyLoadContext(AssemblyLoadContext& InLoadContext) {
    s_ManagedFunctions.UnloadAssemblyLoadContextFptr(InLoadContext.m_ContextId);
    InLoadContext.m_ContextId = -1;
    InLoadContext.m_LoadedAssemblies.Clear();
}

#ifdef NEKO_PLATFORM_WIN
template <typename TFunc>
TFunc LoadFunctionPtr(void* InLibraryHandle, const char* InFunctionName) {
    auto result = (TFunc)GetProcAddress((HMODULE)InLibraryHandle, InFunctionName);
    NEKO_CS_VERIFY(result);
    return result;
}
#else
template <typename TFunc>
TFunc LoadFunctionPtr(void* InLibraryHandle, const char* InFunctionName) {
    auto result = (TFunc)dlsym(InLibraryHandle, InFunctionName);
    NEKO_CS_VERIFY(result);
    return result;
}
#endif

std::filesystem::path GetHostFXRPath() {
#if defined(NEKO_PLATFORM_WIN)
    std::filesystem::path basePath = "";

    // Find the Program Files folder
    TCHAR pf[MAX_PATH];
    SHGetSpecialFolderPath(nullptr, pf, CSIDL_PROGRAM_FILES, FALSE);

    basePath = pf;
    basePath /= "dotnet/host/fxr/";

    auto searchPaths = std::array{basePath};

#elif defined(NEKO_PLATFORM_LINUX)
    auto searchPaths = std::array{
            std::filesystem::path("/usr/lib/dotnet/host/fxr/"),
            std::filesystem::path("/usr/share/dotnet/host/fxr/"),
    };
#endif

    for (const auto& path : searchPaths) {
        if (!std::filesystem::exists(path)) continue;

        for (const auto& dir : std::filesystem::recursive_directory_iterator(path)) {
            if (!dir.is_directory()) continue;

            auto dirPath = dir.path().string();

            if (dirPath.find(NEKO_CS_DOTNET_TARGET_VERSION_MAJOR_STR) == std::string::npos) continue;

            auto res = dir / std::filesystem::path(NEKO_CS_HOSTFXR_NAME);
            NEKO_CS_VERIFY(std::filesystem::exists(res));
            return res;
        }
    }

    return "";
}

void HostInstance::LoadHostFXR() const {
    // Retrieve the file path to the CoreCLR library
    auto hostfxrPath = GetHostFXRPath();

    // Load the CoreCLR library
    void* libraryHandle = nullptr;

#ifdef NEKO_PLATFORM_WIN
#ifdef NEKO_CS_WIDE_CHARS
    libraryHandle = LoadLibraryW(hostfxrPath.c_str());
#else
    libraryHandle = LoadLibraryA(hostfxrPath.string().c_str());
#endif
#else
    libraryHandle = dlopen(hostfxrPath.string().data(), RTLD_NOW | RTLD_GLOBAL);
#endif

    NEKO_CS_VERIFY(libraryHandle != nullptr);

    // Load CoreCLR functions
    s_CoreCLRFunctions.SetHostFXRErrorWriter = LoadFunctionPtr<hostfxr_set_error_writer_fn>(libraryHandle, "hostfxr_set_error_writer");
    s_CoreCLRFunctions.InitHostFXRForRuntimeConfig = LoadFunctionPtr<hostfxr_initialize_for_runtime_config_fn>(libraryHandle, "hostfxr_initialize_for_runtime_config");
    s_CoreCLRFunctions.GetRuntimeDelegate = LoadFunctionPtr<hostfxr_get_runtime_delegate_fn>(libraryHandle, "hostfxr_get_runtime_delegate");
    s_CoreCLRFunctions.CloseHostFXR = LoadFunctionPtr<hostfxr_close_fn>(libraryHandle, "hostfxr_close");
}

bool HostInstance::InitializeNekoCSManaged() {
    // Fetch load_assembly_and_get_function_pointer_fn from CoreCLR
    {
        auto runtimeConfigPath = std::filesystem::path(m_Settings.NekoCSDirectory) / "Neko.Managed.runtimeconfig.json";

        if (!std::filesystem::exists(runtimeConfigPath)) {
            MessageCallback("Failed to find Neko.Managed.runtimeconfig.json", MessageLevel::Error);
            return false;
        }

        int status = s_CoreCLRFunctions.InitHostFXRForRuntimeConfig(runtimeConfigPath.c_str(), nullptr, &m_HostFXRContext);
        NEKO_CS_VERIFY(status == StatusCode::Success || status == StatusCode::Success_HostAlreadyInitialized || status == StatusCode::Success_DifferentRuntimeProperties);
        NEKO_CS_VERIFY(m_HostFXRContext != nullptr);

        status = s_CoreCLRFunctions.GetRuntimeDelegate(m_HostFXRContext, hdt_load_assembly_and_get_function_pointer, (void**)&s_CoreCLRFunctions.GetManagedFunctionPtr);
        NEKO_CS_VERIFY(status == StatusCode::Success);
    }

    using InitializeFn = void (*)(void (*)(String, MessageLevel), void (*)(String));
    InitializeFn coralManagedEntryPoint = nullptr;
    coralManagedEntryPoint = LoadNekoCSManagedFunctionPtr<InitializeFn>(NEKO_CS_STR("Neko.Managed.ManagedHost, Neko.Managed"), NEKO_CS_STR("Initialize"));

    LoadNekoCSFunctions();

    coralManagedEntryPoint(
            [](String InMessage, MessageLevel InLevel) {
                if (MessageFilter & InLevel) {
                    std::string message = InMessage;
                    MessageCallback(message, InLevel);
                }
            },
            [](String InMessage) {
                std::string message = InMessage;
                if (!ExceptionCallback) {
                    MessageCallback(message, MessageLevel::Error);
                    return;
                }

                ExceptionCallback(message);
            });

    ExceptionCallback = m_Settings.ExceptionCallback;

    return true;
}

void HostInstance::LoadNekoCSFunctions() {
    s_ManagedFunctions.CreateAssemblyLoadContextFptr =
            LoadNekoCSManagedFunctionPtr<CreateAssemblyLoadContextFn>(NEKO_CS_STR("Neko.Managed.AssemblyLoader, Neko.Managed"), NEKO_CS_STR("CreateAssemblyLoadContext"));
    s_ManagedFunctions.UnloadAssemblyLoadContextFptr =
            LoadNekoCSManagedFunctionPtr<UnloadAssemblyLoadContextFn>(NEKO_CS_STR("Neko.Managed.AssemblyLoader, Neko.Managed"), NEKO_CS_STR("UnloadAssemblyLoadContext"));
    s_ManagedFunctions.LoadManagedAssemblyFptr = LoadNekoCSManagedFunctionPtr<LoadManagedAssemblyFn>(NEKO_CS_STR("Neko.Managed.AssemblyLoader, Neko.Managed"), NEKO_CS_STR("LoadAssembly"));
    s_ManagedFunctions.UnloadAssemblyLoadContextFptr =
            LoadNekoCSManagedFunctionPtr<UnloadAssemblyLoadContextFn>(NEKO_CS_STR("Neko.Managed.AssemblyLoader, Neko.Managed"), NEKO_CS_STR("UnloadAssemblyLoadContext"));
    s_ManagedFunctions.GetLastLoadStatusFptr = LoadNekoCSManagedFunctionPtr<GetLastLoadStatusFn>(NEKO_CS_STR("Neko.Managed.AssemblyLoader, Neko.Managed"), NEKO_CS_STR("GetLastLoadStatus"));
    s_ManagedFunctions.GetAssemblyNameFptr = LoadNekoCSManagedFunctionPtr<GetAssemblyNameFn>(NEKO_CS_STR("Neko.Managed.AssemblyLoader, Neko.Managed"), NEKO_CS_STR("GetAssemblyName"));

    s_ManagedFunctions.GetAssemblyTypesFptr = LoadNekoCSManagedFunctionPtr<GetAssemblyTypesFn>(NEKO_CS_STR("Neko.Managed.TypeInterface, Neko.Managed"), NEKO_CS_STR("GetAssemblyTypes"));
    s_ManagedFunctions.GetTypeIdFptr = LoadNekoCSManagedFunctionPtr<GetTypeIdFn>(NEKO_CS_STR("Neko.Managed.TypeInterface, Neko.Managed"), NEKO_CS_STR("GetTypeId"));
    s_ManagedFunctions.GetFullTypeNameFptr = LoadNekoCSManagedFunctionPtr<GetFullTypeNameFn>(NEKO_CS_STR("Neko.Managed.TypeInterface, Neko.Managed"), NEKO_CS_STR("GetFullTypeName"));
    s_ManagedFunctions.GetAssemblyQualifiedNameFptr =
            LoadNekoCSManagedFunctionPtr<GetAssemblyQualifiedNameFn>(NEKO_CS_STR("Neko.Managed.TypeInterface, Neko.Managed"), NEKO_CS_STR("GetAssemblyQualifiedName"));
    s_ManagedFunctions.GetBaseTypeFptr = LoadNekoCSManagedFunctionPtr<GetBaseTypeFn>(NEKO_CS_STR("Neko.Managed.TypeInterface, Neko.Managed"), NEKO_CS_STR("GetBaseType"));
    s_ManagedFunctions.GetTypeSizeFptr = LoadNekoCSManagedFunctionPtr<GetTypeSizeFn>(NEKO_CS_STR("Neko.Managed.TypeInterface, Neko.Managed"), NEKO_CS_STR("GetTypeSize"));
    s_ManagedFunctions.IsTypeSubclassOfFptr = LoadNekoCSManagedFunctionPtr<IsTypeSubclassOfFn>(NEKO_CS_STR("Neko.Managed.TypeInterface, Neko.Managed"), NEKO_CS_STR("IsTypeSubclassOf"));
    s_ManagedFunctions.IsTypeAssignableToFptr = LoadNekoCSManagedFunctionPtr<IsTypeAssignableToFn>(NEKO_CS_STR("Neko.Managed.TypeInterface, Neko.Managed"), NEKO_CS_STR("IsTypeAssignableTo"));
    s_ManagedFunctions.IsTypeAssignableFromFptr = LoadNekoCSManagedFunctionPtr<IsTypeAssignableFromFn>(NEKO_CS_STR("Neko.Managed.TypeInterface, Neko.Managed"), NEKO_CS_STR("IsTypeAssignableFrom"));
    s_ManagedFunctions.IsTypeSZArrayFptr = LoadNekoCSManagedFunctionPtr<IsTypeSZArrayFn>(NEKO_CS_STR("Neko.Managed.TypeInterface, Neko.Managed"), NEKO_CS_STR("IsTypeSZArray"));
    s_ManagedFunctions.GetElementTypeFptr = LoadNekoCSManagedFunctionPtr<GetElementTypeFn>(NEKO_CS_STR("Neko.Managed.TypeInterface, Neko.Managed"), NEKO_CS_STR("GetElementType"));
    s_ManagedFunctions.GetTypeMethodsFptr = LoadNekoCSManagedFunctionPtr<GetTypeMethodsFn>(NEKO_CS_STR("Neko.Managed.TypeInterface, Neko.Managed"), NEKO_CS_STR("GetTypeMethods"));
    s_ManagedFunctions.GetTypeFieldsFptr = LoadNekoCSManagedFunctionPtr<GetTypeFieldsFn>(NEKO_CS_STR("Neko.Managed.TypeInterface, Neko.Managed"), NEKO_CS_STR("GetTypeFields"));
    s_ManagedFunctions.GetTypePropertiesFptr = LoadNekoCSManagedFunctionPtr<GetTypePropertiesFn>(NEKO_CS_STR("Neko.Managed.TypeInterface, Neko.Managed"), NEKO_CS_STR("GetTypeProperties"));
    s_ManagedFunctions.HasTypeAttributeFptr = LoadNekoCSManagedFunctionPtr<HasTypeAttributeFn>(NEKO_CS_STR("Neko.Managed.TypeInterface, Neko.Managed"), NEKO_CS_STR("HasTypeAttribute"));
    s_ManagedFunctions.GetTypeAttributesFptr = LoadNekoCSManagedFunctionPtr<GetTypeAttributesFn>(NEKO_CS_STR("Neko.Managed.TypeInterface, Neko.Managed"), NEKO_CS_STR("GetTypeAttributes"));
    s_ManagedFunctions.GetTypeManagedTypeFptr = LoadNekoCSManagedFunctionPtr<GetTypeManagedTypeFn>(NEKO_CS_STR("Neko.Managed.TypeInterface, Neko.Managed"), NEKO_CS_STR("GetTypeManagedType"));
    s_ManagedFunctions.InvokeStaticMethodFptr = LoadNekoCSManagedFunctionPtr<InvokeStaticMethodFn>(NEKO_CS_STR("Neko.Managed.ManagedObject, Neko.Managed"), NEKO_CS_STR("InvokeStaticMethod"));
    s_ManagedFunctions.InvokeStaticMethodRetFptr = LoadNekoCSManagedFunctionPtr<InvokeStaticMethodRetFn>(NEKO_CS_STR("Neko.Managed.ManagedObject, Neko.Managed"), NEKO_CS_STR("InvokeStaticMethodRet"));

    s_ManagedFunctions.GetMethodInfoNameFptr = LoadNekoCSManagedFunctionPtr<GetMethodInfoNameFn>(NEKO_CS_STR("Neko.Managed.TypeInterface, Neko.Managed"), NEKO_CS_STR("GetMethodInfoName"));
    s_ManagedFunctions.GetMethodInfoReturnTypeFptr =
            LoadNekoCSManagedFunctionPtr<GetMethodInfoReturnTypeFn>(NEKO_CS_STR("Neko.Managed.TypeInterface, Neko.Managed"), NEKO_CS_STR("GetMethodInfoReturnType"));
    s_ManagedFunctions.GetMethodInfoParameterTypesFptr =
            LoadNekoCSManagedFunctionPtr<GetMethodInfoParameterTypesFn>(NEKO_CS_STR("Neko.Managed.TypeInterface, Neko.Managed"), NEKO_CS_STR("GetMethodInfoParameterTypes"));
    s_ManagedFunctions.GetMethodInfoAccessibilityFptr =
            LoadNekoCSManagedFunctionPtr<GetMethodInfoAccessibilityFn>(NEKO_CS_STR("Neko.Managed.TypeInterface, Neko.Managed"), NEKO_CS_STR("GetMethodInfoAccessibility"));
    s_ManagedFunctions.GetMethodInfoAttributesFptr =
            LoadNekoCSManagedFunctionPtr<GetMethodInfoAttributesFn>(NEKO_CS_STR("Neko.Managed.TypeInterface, Neko.Managed"), NEKO_CS_STR("GetMethodInfoAttributes"));

    s_ManagedFunctions.GetFieldInfoNameFptr = LoadNekoCSManagedFunctionPtr<GetFieldInfoNameFn>(NEKO_CS_STR("Neko.Managed.TypeInterface, Neko.Managed"), NEKO_CS_STR("GetFieldInfoName"));
    s_ManagedFunctions.GetFieldInfoTypeFptr = LoadNekoCSManagedFunctionPtr<GetFieldInfoTypeFn>(NEKO_CS_STR("Neko.Managed.TypeInterface, Neko.Managed"), NEKO_CS_STR("GetFieldInfoType"));
    s_ManagedFunctions.GetFieldInfoAccessibilityFptr =
            LoadNekoCSManagedFunctionPtr<GetFieldInfoAccessibilityFn>(NEKO_CS_STR("Neko.Managed.TypeInterface, Neko.Managed"), NEKO_CS_STR("GetFieldInfoAccessibility"));
    s_ManagedFunctions.GetFieldInfoAttributesFptr =
            LoadNekoCSManagedFunctionPtr<GetFieldInfoAttributesFn>(NEKO_CS_STR("Neko.Managed.TypeInterface, Neko.Managed"), NEKO_CS_STR("GetFieldInfoAttributes"));

    s_ManagedFunctions.GetPropertyInfoNameFptr = LoadNekoCSManagedFunctionPtr<GetPropertyInfoNameFn>(NEKO_CS_STR("Neko.Managed.TypeInterface, Neko.Managed"), NEKO_CS_STR("GetPropertyInfoName"));
    s_ManagedFunctions.GetPropertyInfoTypeFptr = LoadNekoCSManagedFunctionPtr<GetPropertyInfoTypeFn>(NEKO_CS_STR("Neko.Managed.TypeInterface, Neko.Managed"), NEKO_CS_STR("GetPropertyInfoType"));
    s_ManagedFunctions.GetPropertyInfoAttributesFptr =
            LoadNekoCSManagedFunctionPtr<GetPropertyInfoAttributesFn>(NEKO_CS_STR("Neko.Managed.TypeInterface, Neko.Managed"), NEKO_CS_STR("GetPropertyInfoAttributes"));

    s_ManagedFunctions.GetAttributeFieldValueFptr =
            LoadNekoCSManagedFunctionPtr<GetAttributeFieldValueFn>(NEKO_CS_STR("Neko.Managed.TypeInterface, Neko.Managed"), NEKO_CS_STR("GetAttributeFieldValue"));
    s_ManagedFunctions.GetAttributeTypeFptr = LoadNekoCSManagedFunctionPtr<GetAttributeTypeFn>(NEKO_CS_STR("Neko.Managed.TypeInterface, Neko.Managed"), NEKO_CS_STR("GetAttributeType"));

    s_ManagedFunctions.SetInternalCallsFptr = LoadNekoCSManagedFunctionPtr<SetInternalCallsFn>(NEKO_CS_STR("Neko.Managed.Interop.InternalCallsManager, Neko.Managed"), NEKO_CS_STR("SetInternalCalls"));
    s_ManagedFunctions.CreateObjectFptr = LoadNekoCSManagedFunctionPtr<CreateObjectFn>(NEKO_CS_STR("Neko.Managed.ManagedObject, Neko.Managed"), NEKO_CS_STR("CreateObject"));
    s_ManagedFunctions.InvokeMethodFptr = LoadNekoCSManagedFunctionPtr<InvokeMethodFn>(NEKO_CS_STR("Neko.Managed.ManagedObject, Neko.Managed"), NEKO_CS_STR("InvokeMethod"));
    s_ManagedFunctions.InvokeMethodRetFptr = LoadNekoCSManagedFunctionPtr<InvokeMethodRetFn>(NEKO_CS_STR("Neko.Managed.ManagedObject, Neko.Managed"), NEKO_CS_STR("InvokeMethodRet"));
    s_ManagedFunctions.SetFieldValueFptr = LoadNekoCSManagedFunctionPtr<SetFieldValueFn>(NEKO_CS_STR("Neko.Managed.ManagedObject, Neko.Managed"), NEKO_CS_STR("SetFieldValue"));
    s_ManagedFunctions.GetFieldValueFptr = LoadNekoCSManagedFunctionPtr<GetFieldValueFn>(NEKO_CS_STR("Neko.Managed.ManagedObject, Neko.Managed"), NEKO_CS_STR("GetFieldValue"));
    s_ManagedFunctions.SetPropertyValueFptr = LoadNekoCSManagedFunctionPtr<SetFieldValueFn>(NEKO_CS_STR("Neko.Managed.ManagedObject, Neko.Managed"), NEKO_CS_STR("SetPropertyValue"));
    s_ManagedFunctions.GetPropertyValueFptr = LoadNekoCSManagedFunctionPtr<GetFieldValueFn>(NEKO_CS_STR("Neko.Managed.ManagedObject, Neko.Managed"), NEKO_CS_STR("GetPropertyValue"));
    s_ManagedFunctions.DestroyObjectFptr = LoadNekoCSManagedFunctionPtr<DestroyObjectFn>(NEKO_CS_STR("Neko.Managed.ManagedObject, Neko.Managed"), NEKO_CS_STR("DestroyObject"));
    s_ManagedFunctions.CollectGarbageFptr = LoadNekoCSManagedFunctionPtr<CollectGarbageFn>(NEKO_CS_STR("Neko.Managed.GarbageCollector, Neko.Managed"), NEKO_CS_STR("CollectGarbage"));
    s_ManagedFunctions.WaitForPendingFinalizersFptr =
            LoadNekoCSManagedFunctionPtr<WaitForPendingFinalizersFn>(NEKO_CS_STR("Neko.Managed.GarbageCollector, Neko.Managed"), NEKO_CS_STR("WaitForPendingFinalizers"));
}

void* HostInstance::LoadNekoCSManagedFunctionPtr(const std::filesystem::path& InAssemblyPath, const CharType* InTypeName, const CharType* InMethodName, const CharType* InDelegateType) const {
    void* funcPtr = nullptr;
    int status = s_CoreCLRFunctions.GetManagedFunctionPtr(InAssemblyPath.c_str(), InTypeName, InMethodName, InDelegateType, nullptr, &funcPtr);
    NEKO_CS_VERIFY(status == StatusCode::Success && funcPtr != nullptr);
    return funcPtr;
}

/*================================================================================
// GC
================================================================================*/

void GC::Collect() { Collect(-1, GCCollectionMode::Default, true, false); }

void GC::Collect(int32_t InGeneration, GCCollectionMode InCollectionMode, bool InBlocking, bool InCompacting) {
    s_ManagedFunctions.CollectGarbageFptr(InGeneration, InCollectionMode, InBlocking, InCompacting);
}

void GC::WaitForPendingFinalizers() { s_ManagedFunctions.WaitForPendingFinalizersFptr(); }

/*================================================================================
// FieldInfo
================================================================================*/

String FieldInfo::GetName() const { return s_ManagedFunctions.GetFieldInfoNameFptr(m_Handle); }

Type& FieldInfo::GetType() {
    if (!m_Type) {
        Type fieldType;
        s_ManagedFunctions.GetFieldInfoTypeFptr(m_Handle, &fieldType.m_Id);
        m_Type = TypeCache::Get().CacheType(std::move(fieldType));
    }

    return *m_Type;
}

TypeAccessibility FieldInfo::GetAccessibility() const { return s_ManagedFunctions.GetFieldInfoAccessibilityFptr(m_Handle); }

std::vector<Attribute> FieldInfo::GetAttributes() const {
    int32_t attributeCount;
    s_ManagedFunctions.GetFieldInfoAttributesFptr(m_Handle, nullptr, &attributeCount);
    std::vector<ManagedHandle> attributeHandles(attributeCount);
    s_ManagedFunctions.GetFieldInfoAttributesFptr(m_Handle, attributeHandles.data(), &attributeCount);

    std::vector<Attribute> result(attributeHandles.size());
    for (size_t i = 0; i < attributeHandles.size(); i++) result[i].m_Handle = attributeHandles[i];

    return result;
}

/*================================================================================
// Attribute
================================================================================*/

Type& Attribute::GetType() {
    if (!m_Type) {
        Type type;
        s_ManagedFunctions.GetAttributeTypeFptr(m_Handle, &type.m_Id);
        m_Type = TypeCache::Get().CacheType(std::move(type));
    }

    return *m_Type;
}

void Attribute::GetFieldValueInternal(std::string_view InFieldName, void* OutValue) const {
    auto fieldName = String::New(InFieldName);
    s_ManagedFunctions.GetAttributeFieldValueFptr(m_Handle, fieldName, OutValue);
    String::Free(fieldName);
}

/*================================================================================
// Managed
================================================================================*/

void ManagedAssembly::AddInternalCall(std::string_view InClassName, std::string_view InVariableName, void* InFunctionPtr) {
    NEKO_CS_VERIFY(InFunctionPtr != nullptr);

    std::string assemblyQualifiedName(InClassName);
    assemblyQualifiedName += "+";
    assemblyQualifiedName += InVariableName;
    assemblyQualifiedName += ", ";
    assemblyQualifiedName += m_Name;

    const auto& name = m_InternalCallNameStorage.emplace_back(StringHelper::ConvertUtf8ToWide(assemblyQualifiedName));

    InternalCall internalCall;
    internalCall.Name = name.c_str();
    internalCall.NativeFunctionPtr = InFunctionPtr;
    m_InternalCalls.emplace_back(internalCall);
}

void ManagedAssembly::UploadInternalCalls() { s_ManagedFunctions.SetInternalCallsFptr(m_InternalCalls.data(), static_cast<int32_t>(m_InternalCalls.size())); }

Type& ManagedAssembly::GetType(std::string_view InClassName) const {
    static Type s_NullType;
    Type* type = TypeCache::Get().GetTypeByName(InClassName);
    return type != nullptr ? *type : s_NullType;
}

const std::vector<Type*>& ManagedAssembly::GetTypes() const { return m_Types; }

ManagedAssembly& AssemblyLoadContext::LoadAssembly(std::string_view InFilePath) {
    auto filepath = String::New(InFilePath);

    auto [idx, result] = m_LoadedAssemblies.EmplaceBack();
    result.m_Host = m_Host;
    result.m_AssemblyID = s_ManagedFunctions.LoadManagedAssemblyFptr(m_ContextId, filepath);
    result.m_LoadStatus = s_ManagedFunctions.GetLastLoadStatusFptr();

    if (result.m_LoadStatus == AssemblyLoadStatus::Success) {
        auto assemblyName = s_ManagedFunctions.GetAssemblyNameFptr(result.m_AssemblyID);
        result.m_Name = assemblyName;
        String::Free(assemblyName);

        int32_t typeCount = 0;
        s_ManagedFunctions.GetAssemblyTypesFptr(result.m_AssemblyID, nullptr, &typeCount);
        std::vector<TypeId> typeIds(typeCount);
        s_ManagedFunctions.GetAssemblyTypesFptr(result.m_AssemblyID, typeIds.data(), &typeCount);

        for (auto typeId : typeIds) {
            Type type;
            type.m_Id = typeId;
            result.m_Types.push_back(TypeCache::Get().CacheType(std::move(type)));
        }
    }

    String::Free(filepath);

    return result;
}

}  // namespace NekoCS
