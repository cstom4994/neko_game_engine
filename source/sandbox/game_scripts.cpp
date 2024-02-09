
#include "game_scripts.h"

#include "engine/neko_engine.h"

#ifdef GAME_CSHARP_ENABLED

static void __csharp_exception_callback(std::string_view InMessage) { std::cout << "# Unhandled native exception: " << InMessage << std::endl; }

void VectorAddIcall(neko_vec3* InVec0, const neko_vec3* InVec1) {
    std::cout << "VectorAddIcall" << std::endl;
    InVec0->x += InVec1->x;
    InVec0->y += InVec1->y;
    InVec0->z += InVec1->z;
}

void __csharp_bind_log_info(neko_cs::String InString) { neko_log_info("[csharp] %s", std::string(InString).c_str()); }
void __csharp_bind_log_warn(neko_cs::String InString) { neko_log_warning("[csharp] %s", std::string(InString).c_str()); }
void __csharp_bind_log_error(neko_cs::String InString) { neko_log_error("[csharp] %s", std::string(InString).c_str()); }

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

void test_cs3(neko_cs::ManagedAssembly& assembly);

void game_native_binding_register(neko_cs::ManagedAssembly& assembly) {

    assembly.AddInternalCall("Example.Managed.NativeBinding", "NekoLogInfo", reinterpret_cast<void*>(&__csharp_bind_log_info));
    assembly.AddInternalCall("Example.Managed.NativeBinding", "NekoLogWarn", reinterpret_cast<void*>(&__csharp_bind_log_warn));
    assembly.AddInternalCall("Example.Managed.NativeBinding", "NekoLogError", reinterpret_cast<void*>(&__csharp_bind_log_error));

    assembly.AddInternalCall(                                                                                  //
            "Example.Managed.NativeBinding",                                                                   //
            "NekoLogTrace",                                                                                    //
            +[](neko_cs::String InString) { neko_log_trace("[csharp] %s", std::string(InString).c_str()); });  //

    assembly.AddInternalCall(                                                                         //
            "Example.Managed.NativeBinding",                                                          //
            "neko_platform_key_pressed",                                                              //
            +[](u32 key) -> bool { return neko_platform_key_pressed((neko_platform_keycode)key); });  //

    assembly.UploadInternalCalls();
}

void game_csharp::init(std::string _managed_path) {
    managed_path = _managed_path;

    auto managed_dir = std::filesystem::path(managed_path);

    neko_cs::HostSettings settings = {.NekoCSDirectory = managed_dir.string(), .ExceptionCallback = __csharp_exception_callback};

    this->host_instance.Initialize(settings);

    this->load_context = host_instance.CreateAssemblyLoadContext("GameCSharpContext");

    auto assemblyPath = managed_dir / "Example.Managed.dll";
    auto& assembly = this->load_context.LoadAssembly(assemblyPath.string());

    assembly.AddInternalCall("Example.Managed.NativeBinding", "VectorAddIcall", reinterpret_cast<void*>(&VectorAddIcall));
    assembly.AddInternalCall("Example.Managed.NativeBinding", "NativeArrayIcall", reinterpret_cast<void*>(&NativeArrayIcall));
    assembly.AddInternalCall("Example.Managed.NativeBinding", "ArrayReturnIcall", reinterpret_cast<void*>(&ArrayReturnIcall));

    assembly.UploadInternalCalls();

    game_native_binding_register(assembly);

    test_cs3(assembly);

    auto& game_instance_type = assembly.GetType("Example.Managed.ExampleClass");
    game_instance = game_instance_type.CreateInstance(1);
}

void game_csharp::shutdown() {
    this->host_instance.UnloadAssemblyLoadContext(this->load_context);
    neko_cs::GC::Collect();

    this->host_instance.Shutdown();
}

void game_csharp::update() { game_instance.InvokeMethod("TestUpdate"); }

void game_csharp::hotfix() {
    this->shutdown();
    this->init(managed_path);
}

#endif
