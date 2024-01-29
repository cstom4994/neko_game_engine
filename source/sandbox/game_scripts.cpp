
#include "game_scripts.h"

static void __csharp_exception_callback(std::string_view InMessage) { std::cout << "# Unhandled native exception: " << InMessage << std::endl; }

void VectorAddIcall(neko_vec3* InVec0, const neko_vec3* InVec1) {
    std::cout << "VectorAddIcall" << std::endl;
    InVec0->x += InVec1->x;
    InVec0->y += InVec1->y;
    InVec0->z += InVec1->z;
}

void PrintStringIcall(neko_cs::String InString) { neko_log_info("[csharp] %s", std::string(InString).c_str()); }

void NativeArrayIcall(neko_cs::Array<float> InValues) {
    std::cout << "NativeArrayIcall" << std::endl;
    for (auto value : InValues) {
        std::cout << value << std::endl;
    }
}

neko_cs::Array<float> ArrayReturnIcall() {
    std::cout << "ArrayReturnIcall" << std::endl;
    return neko_cs::Array<float>::New({10.0f, 5000.0f, 1000.0f});
}

void game_csharp::init(std::string managed_path) {
    auto managed_dir = std::filesystem::path(managed_path);

    neko_cs::HostSettings settings = {.NekoCSDirectory = managed_dir.string(), .ExceptionCallback = __csharp_exception_callback};

    this->hostInstance.Initialize(settings);

    this->loadContext = hostInstance.CreateAssemblyLoadContext("GameCSharpContext");

    auto assemblyPath = managed_dir / "Example.Managed.dll";
    auto& assembly = this->loadContext.LoadAssembly(assemblyPath.string());

    assembly.AddInternalCall("Example.Managed.ExampleClass", "VectorAddIcall", reinterpret_cast<void*>(&VectorAddIcall));
    assembly.AddInternalCall("Example.Managed.ExampleClass", "PrintStringIcall", reinterpret_cast<void*>(&PrintStringIcall));
    assembly.AddInternalCall("Example.Managed.ExampleClass", "NativeArrayIcall", reinterpret_cast<void*>(&NativeArrayIcall));
    assembly.AddInternalCall("Example.Managed.ExampleClass", "ArrayReturnIcall", reinterpret_cast<void*>(&ArrayReturnIcall));
    assembly.UploadInternalCalls();

    // 获取对ExampleClass类型的引用
    auto& exampleType = assembly.GetType("Example.Managed.ExampleClass");

    // 用值50调用静态方法StaticMethod
    exampleType.InvokeStaticMethod("StaticMethod", 50.0f);

    // 获取对CustomAttribute类型的引用
    auto& customAttributeType = assembly.GetType("Example.Managed.CustomAttribute");

    // 获取ExampleType类上所有属性的列表
    auto exampleTypeAttribs = exampleType.GetAttributes();
    for (auto& attribute : exampleTypeAttribs) {
        if (attribute.GetType() == customAttributeType) {
            // 从CustomAttribute属性获取Value的值
            std::cout << "CustomAttribute: " << attribute.GetFieldValue<float>("Value") << std::endl;
        }
    }

    // 创建一个Example.Managed.ExampleClass类型的实例并将50传递给构造函数
    auto exampleInstance = exampleType.CreateInstance(50);

    // 使用neko_vec3参数调用名为MemberMethod的方法(不返回任何内容)
    exampleInstance.InvokeMethod("Void MemberMethod(neko_vec3)", neko_vec3{10.0f, 10.0f, 10.0f});

    // 调用值为10的PublicProp上的setter(在C#中将乘以2)
    exampleInstance.SetPropertyValue("PublicProp", 10);

    // 以int形式获取PublicProp的值
    std::cout << exampleInstance.GetPropertyValue<int32_t>("PublicProp") << std::endl;

    // 将私有字段myPrivateValue的值设置为值10(在C#中不会乘以2)
    exampleInstance.SetFieldValue("myPrivateValue", 10);

    // 以int形式获取myPrivateValue的值
    std::cout << exampleInstance.GetFieldValue<int32_t>("myPrivateValue") << std::endl;

    // 调用StringDemo方法 该方法将使用字符串参数调用PrintStringIcall
    exampleInstance.InvokeMethod("StringDemo");

    // 调用ArrayDemo方法 该方法将调用NativeArrayIcall并传递我们在此处给出的值
    // 并调用ArrayReturnIcall
    auto arr = neko_cs::Array<float>::New({5.0f, 0.0f, 10.0f, -50.0f});
    exampleInstance.InvokeMethod("ArrayDemo", arr);
    neko_cs::Array<float>::Free(arr);
}

void game_csharp::shutdown() {
    this->hostInstance.UnloadAssemblyLoadContext(this->loadContext);
    neko_cs::GC::Collect();
}
