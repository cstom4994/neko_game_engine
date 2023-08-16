#include "engine/meta/neko_refl_lua.h"

#include <map>
#include <stdexcept>

#include "gen/neko_refl_helper_gen.hpp"

using namespace neko;
using namespace neko::cpp;

// void neko::cpp::details::stack_dump(lua_State* L) {
//   printf("\n------ stack dump begin ------\n");
//   for (int i = 1; i <= lua_gettop(L); ++i) {
//       int t = lua_type(L, i);
//       switch (t) {
//       case LUA_TNONE: {
//           printf("LUA_TNONE\n");
//       }break;
//
//       case LUA_TNIL: {
//           printf("LUA_TNIL\n");
//       }break;
//
//       case LUA_TBOOLEAN: {
//           printf("LUA_TBOOLEAN : %s\n", lua_toboolean(L, i) ? "true" : "false");
//       }break;
//
//       case LUA_TLIGHTUSERDATA: {
//           printf("LUA_TLIGHTUSERDATA\n");
//       }break;
//
//       case LUA_TNUMBER: {
//           if (lua_isinteger(L, i)) {
//               printf("LUA_TNUMBER integer : %lld \n", lua_tointeger(L, i));
//           }
//           else if (lua_isnumber(L, i)) {
//               printf("LUA_TNUMBER number: %g\n", lua_tonumber(L, i));
//           }
//       }break;
//
//       case LUA_TSTRING: {
//           printf("LUA_TSTRING : %s\n", lua_tostring(L, i));
//       }break;
//
//       case LUA_TTABLE: {
//           printf("LUA_TTABLE\n");
//       }break;
//
//       case LUA_TFUNCTION: {
//           printf("LUA_TFUNCTION\n");
//       }break;
//
//       case LUA_TUSERDATA: {
//           printf("LUA_TUSERDATA\n");
//       }break;
//
//       case LUA_TTHREAD: {
//           printf("LUA_TTHREAD\n");
//       }break;
//
//       case LUA_NUMTAGS: {
//           printf("LUA_NUMTAGS\n");
//       }break;
//
//       default: {
//           printf("%s\n", lua_typename(L, t));
//       }break;
//       }
//   }
//
//   std::cout << "------ stack dump end ------" << std::endl;
// }

ObjectView* neko::cpp::details::safe_get_Object(neko_lua_state_view L, int idx) {
    void* p = lua_touserdata(L, idx);
    if (p != nullptr) {                                                 /* value is a userdata? */
        if (lua_getmetatable(L, idx)) {                                 /* does it have a metatable? */
            luaL_getmetatable(L, type_name<ObjectView>().Data());       /* get correct metatable */
            if (!lua_rawequal(L, -1, -2)) {                             /* not the same? */
                luaL_getmetatable(L, type_name<SharedObject>().Data()); /* get correct metatable */
                if (!lua_rawequal(L, -1, -3))                           /* not the same? */
                    p = nullptr;                                        /* value is a userdata with wrong metatable */
                lua_pop(L, 3);                                          /* remove all metatables */
                p = static_cast<ObjectView*>((SharedObject*)p);
            } else
                lua_pop(L, 2); /* remove both metatables */
            return (ObjectView*)p;
        }
    }
    return nullptr;
}

int neko::cpp::details::FillArgStack(neko_lua_state_view L, ArgStack& stack, int begin, int cnt) {
    neko_assert(cnt >= 0);
    if (cnt > MaxArgNum) {
        L.pushfstring("neko::cpp::details::FillArgStack : The number of arguments (%d) is greater than MaxArgNum (%d).", cnt, static_cast<int>(MaxArgNum));
        return 1;
    }

    auto& copied_args_buffer = stack.copied_args_buffer;
    auto& argptr_buffer = stack.argptr_buffer;
    auto& argType_buffer = stack.argType_buffer;
    auto& num_copied_args = stack.num_copied_args;

    for (int i{0}; i < cnt; i++) {
        int arg = begin + i;
        int type = L.type(arg);
        switch (type) {
            case LUA_TNIL: {
                auto arg_buffer = &copied_args_buffer[num_copied_args++];
                argptr_buffer[i] = arg_buffer;
                buffer_as<Type>(&argType_buffer[i]) = Type_of<std::nullptr_t>;
                buffer_as<std::nullptr_t>(arg_buffer) = nullptr;
                break;
            }
            case LUA_TBOOLEAN: {
                auto arg_buffer = &copied_args_buffer[num_copied_args++];
                argptr_buffer[i] = arg_buffer;
                buffer_as<Type>(&argType_buffer[i]) = Type_of<bool>;
                buffer_as<bool>(arg_buffer) = static_cast<bool>(L.toboolean(arg));
                break;
            }
            case LUA_TNUMBER:
                if (L.isinteger(arg)) {
                    auto arg_buffer = &copied_args_buffer[num_copied_args++];
                    argptr_buffer[i] = arg_buffer;
                    buffer_as<Type>(&argType_buffer[i]) = Type_of<lua_Integer>;
                    buffer_as<lua_Integer>(arg_buffer) = static_cast<lua_Integer>(L.tointeger(arg));
                } else if (L.isnumber(arg)) {
                    auto arg_buffer = &copied_args_buffer[num_copied_args++];
                    argptr_buffer[i] = arg_buffer;
                    buffer_as<Type>(&argType_buffer[i]) = Type_of<lua_Number>;
                    buffer_as<lua_Number>(arg_buffer) = static_cast<lua_Number>(L.tonumber(arg));
                } else
                    neko_assert(false);
                break;
            case LUA_TSTRING: {
                auto arg_buffer = &copied_args_buffer[num_copied_args++];
                argptr_buffer[i] = arg_buffer;
                buffer_as<Type>(&argType_buffer[i]) = Type_of<const char*>;
                buffer_as<const char*>(arg_buffer) = L.tostring(arg);
                break;
            }
            case LUA_TUSERDATA:
                // TODO: speed up
                if (void* udata = L.testudata(arg, type_name<ObjectView>().Data())) {
                    ObjectView arg = *static_cast<ObjectView*>(udata);
                    auto ref_arg = arg.AddLValueReferenceWeak();
                    argptr_buffer[i] = ref_arg.GetPtr();
                    buffer_as<Type>(&argType_buffer[i]) = ref_arg.GetType();
                } else if (void* udata = L.testudata(arg, type_name<SharedObject>().Data())) {
                    ObjectView arg = *static_cast<SharedObject*>(udata);
                    auto ref_arg = arg.AddLValueReferenceWeak();
                    argptr_buffer[i] = ref_arg.GetPtr();
                    buffer_as<Type>(&argType_buffer[i]) = ref_arg.GetType();
                } else if (void* udata = L.testudata(arg, type_name<Name>().Data())) {
                    auto* arg = static_cast<Name*>(udata);
                    argptr_buffer[i] = arg;
                    buffer_as<Type>(&argType_buffer[i]) = Type_of<Name>;
                } else if (void* udata = L.testudata(arg, type_name<Type>().Data())) {
                    auto* arg = static_cast<Type*>(udata);
                    argptr_buffer[i] = arg;
                    buffer_as<Type>(&argType_buffer[i]) = Type_of<Type>;
                } else if (!L.getmetatable(arg)) {
                    argptr_buffer[i] = L.touserdata(arg);
                    buffer_as<Type>(&argType_buffer[i]) = Type_of<void*>;
                } else {
                    int success = L.getfield(-1, "__name");
                    const char* udata_name = "UNKNOWN";
                    if (success) udata_name = L.tostring(-1);
                    L.pushfstring("neko::cpp::details::FillArgStack : ArgStack doesn't support %s (%s).", L.typename_(arg), udata_name);
                    return 1;
                }
                break;
            default:
                L.pushfstring("neko::cpp::details::FillArgStack : ArgStack doesn't support %s.", L.typename_(arg));
                return 1;
        }
    }

    return 0;
}

MethodPtr::Func neko::cpp::details::LuaFuncToMethodPtrFunc(Type object_type, MethodFlag flag, Type result_type, neko_lua_ref func_ref) {
    neko_assert(enum_single(flag));
    Type ref_obj_type = flag == MethodFlag::Static ? Type{}
                                                   : (flag == MethodFlag::Const ? neko_refl_instance.tregistry.RegisterAddConstLValueReference(object_type)
                                                                                : neko_refl_instance.tregistry.RegisterAddLValueReference(object_type));
    return [ref_obj_type, result_type, fref = std::make_shared<neko_lua_ref>(std::move(func_ref))](void* obj, void* result_buffer, ArgsView args) mutable {
        auto L = fref->GetView();
        int top = L.gettop();
        fref->Get();
        const int n = static_cast<int>(args.Types().size());
        int callargnum;
        if (ref_obj_type.valid()) {
            callargnum = n + 1;
            L.checkstack(callargnum);
            push(L, ObjectView{ref_obj_type, obj});
        } else {
            callargnum = n;
            L.checkstack(callargnum);
        }

        for (std::size_t i = 0; i < n; i++) push(L, args[i]);
        int error = L.pcall(callargnum, LUA_MULTRET, 0);
        int result_construct_argnum = L.gettop() - top;
        details::LuaStackPopGuard popguard{L, result_construct_argnum};
        if (error) {
            std::stringstream ss;
            ss << type_name<SharedObject>().View() << "::new_MethodPtr::lambda(" << ref_obj_type.get_name() << "):" << std::endl << auto_get<std::string_view>(L, -1);
            std::string str = ss.str();
            std::runtime_error except{str.data()};
            L.pop(1);
            throw except;
        }

        if (!result_buffer || result_type.IsVoid()) return;

        if (result_type.IsReference()) {
            if (result_construct_argnum != 1) {
                std::stringstream ss;
                ss << type_name<SharedObject>().View() << "::new_MethodPtr::lambda: The result type is reference, so the number (" << result_construct_argnum << ") of return values must be 1";
                std::string str = ss.str();
                std::runtime_error except{str.data()};
                throw except;
            }
            ObjectView return_obj;
            if (void* obj = L.testudata(-1, type_name<ObjectView>().Data()))
                return_obj = *reinterpret_cast<ObjectView*>(obj);
            else if (void* obj = L.testudata(-1, type_name<SharedObject>().Data())) {
                auto* sobj = reinterpret_cast<SharedObject*>(obj);
                return_obj = {sobj->GetType(), sobj->GetPtr()};
            } else {
                std::stringstream ss;
                ss << type_name<SharedObject>().View() << "::new_MethodPtr::lambda: The result type is reference, so the return type must be a ObjectView/SharedObject";
                std::string str = ss.str();
                std::runtime_error except{str.data()};
                throw except;
            }

            if (!is_ref_compatible(result_type, result_type)) {
                std::stringstream ss;
                ss << type_name<SharedObject>().View() << "::new_MethodPtr::lambda: The result type is reference, but result type (" << result_type.get_name()
                   << ") is not compatible with return type (" << return_obj.GetType().get_name() << ")";
                std::string str = ss.str();
                std::runtime_error except{str.data()};
                throw except;
            }

            buffer_as<void*>(result_buffer) = return_obj.GetPtr();
            return;
        }

        ArgStack argstack;
        {  // fill argstack
            int error = details::FillArgStack(L, argstack, top + 1, result_construct_argnum);
            if (error) {
                std::stringstream ss;
                ss << type_name<SharedObject>().View() << "::new_MethodPtr::lambda:\n" << auto_get<std::string_view>(L, -1);
                std::string str = ss.str();
                std::runtime_error except{str.data()};
                L.pop(1);
                throw except;
            }
        }

        {  // construct result
            bool success = neko_refl_instance.Construct(ObjectView{result_type, result_buffer},
                                                        {argstack.argptr_buffer, {reinterpret_cast<Type*>(argstack.argType_buffer), static_cast<std::size_t>(result_construct_argnum)}});
            if (!success) {
                std::stringstream ss;
                ss << type_name<SharedObject>().View() << "::new_MethodPtr::lambda: Construct fail.";
                std::string str = ss.str();
                std::runtime_error except{str.data()};
                L.pop(1);
                throw except;
            }
        }
    };
}

namespace neko::cpp::details {
struct Invalid {};  // for f_meta
}  // namespace neko::cpp::details

template <typename Functor, typename MetaName, int LArgNum = -1, typename Ret = neko::cpp::details::Invalid, bool Inversable = false>
static int f_meta(lua_State* L_) {
    static_assert(!Inversable || IsObjectOrView_v<Functor>);

    neko_lua_state_view L{L_};

    int L_argnum = L.gettop();
    if constexpr (LArgNum != -1) {
        if (L_argnum != LArgNum) {
            return L.error("%s::%s : The number of arguments is invalid. The function needs %d argument.", type_name<Functor>().Data(), MetaName::Data(), LArgNum);
        }
    } else {
        if (L_argnum <= 0) {
            return L.error("%s::%s : The number of arguments is invalid. The function needs >= 1 argument.", type_name<Functor>().Data(), MetaName::Data());
        }
    }

    ObjectView ptr;
    Name method_name;
    int argnum = L_argnum - 1;

    int functor_idx;
    if constexpr (Inversable) {
        if (details::safe_get_Object(L, 1))
            functor_idx = 1;
        else
            functor_idx = 2;
    } else
        functor_idx = 1;

    const auto& functor = details::auto_get<Functor>(L, functor_idx);

    if constexpr (IsObjectOrView_v<Functor>) {
        ptr = ObjectView{functor.GetType(), functor.GetPtr()};
        method_name = {MetaName::View()};
    } else {
        ptr = ObjectView{functor.type, nullptr};
        method_name = functor.method_name;
        if (L_argnum >= 2) {
            if (void* udata = L.testudata(2, type_name<ObjectView>().Data())) {
                ObjectView* obj = static_cast<ObjectView*>(udata);
                if (obj->GetType() == functor.type) {
                    ptr = *obj;
                    --argnum;
                } else if (functor.type == Type_of<ObjectView>) {
                    ptr = {Type_of<ObjectView>, obj};
                    --argnum;
                }
            } else if (auto obj = (SharedObject*)L.testudata(2, type_name<SharedObject>().Data())) {
                if (obj->GetType() == functor.type) {
                    ptr = *obj;
                    --argnum;
                } else if (functor.type.Is<ObjectView>()) {
                    ptr = {Type_of<ObjectView>, static_cast<ObjectView*>(obj)};
                    --argnum;
                } else if (functor.type.Is<SharedObject>()) {
                    ptr = {Type_of<SharedObject>, obj};
                    --argnum;
                }
            }
        }
    }

    if (argnum > MaxArgNum) {
        return L.error("%s::%s : The number of arguments (%d) is greater than MaxArgNum (%d).", type_name<Functor>().Data(), MetaName::Data(), argnum, static_cast<int>(MaxArgNum));
    }

    details::ArgStack argstack;

    {  // fill argstack
        int error;
        if constexpr (Inversable) {
            if (functor_idx == 2)
                error = details::FillArgStack(L, argstack, 1, 1);
            else
                error = details::FillArgStack(L, argstack, L_argnum - argnum + 1, argnum);
        } else
            error = details::FillArgStack(L, argstack, L_argnum - argnum + 1, argnum);

        if (error) {
            return L.error("%s::new : \n%s", type_name<SharedObject>().Data(), L.tostring(-1));
        }
    }

    if constexpr (!std::is_same_v<Ret, details::Invalid>) {
        Type result_type = ptr.IsInvocable(method_name, std::span<const Type>{reinterpret_cast<Type*>(argstack.argType_buffer), static_cast<std::size_t>(argnum)});
        if (!result_type.Is<Ret>()) {
            return L.error("%s::%s : The function isn't invocable with arguments or it's return type isn't %s.", type_name<Functor>().Data(), MetaName::Data(), type_name<Ret>().Data());
        }

        if constexpr (std::is_void_v<Ret>) {
            try {
                ptr.Invoke<void>(method_name, {argstack.argptr_buffer, {reinterpret_cast<Type*>(argstack.argType_buffer), static_cast<std::size_t>(argnum)}});
            } catch (const std::runtime_error& e) {
                return L.error("%s::%s : Invoke exception.\n%s", type_name<Functor>().Data(), MetaName::Data(), e.what());
            } catch (...) {
                return L.error("%s::%s : Invoke exception.\n", type_name<Functor>().Data(), MetaName::Data());
            }

            return 0;
        } else {
            try {
                Ret rst = ptr.Invoke<Ret>(method_name, {argstack.argptr_buffer, {reinterpret_cast<Type*>(argstack.argType_buffer), static_cast<std::size_t>(argnum)}});

                details::push<Ret>(L, std::move(rst));
            } catch (const std::runtime_error& e) {
                return L.error("%s::%s : Invoke exception.\n%s", type_name<Functor>().Data(), MetaName::Data(), e.what());
            } catch (...) {
                return L.error("%s::%s : Invoke exception.\n", type_name<Functor>().Data(), MetaName::Data());
            }

            return 1;
        }
    } else {
        {  // get unsync resource
            int success = L.getfield(LUA_REGISTRYINDEX, details::UnsyncRsrc);
            neko_assert(success);
        }
        auto* rsrc = (std::pmr::unsynchronized_pool_resource*)L.touserdata(-1);
        try {
            SharedObject rst = ptr.MInvoke(method_name, rsrc, {argstack.argptr_buffer, {reinterpret_cast<Type*>(argstack.argType_buffer), static_cast<std::size_t>(argnum)}}, MethodFlag::All, rsrc);

            if (!rst.GetType()) {
                return L.error("%s::%s : Call neko::cpp::ObjectView::MInvoke (%s, %s) failed.", type_name<Functor>().Data(), MetaName::Data(), ptr.GetType().get_name().data(),
                               method_name.get_view().data());
            }

            if (rst.GetType().Is<void>()) return 0;

            if (rst.IsObjectView())
                details::push<ObjectView>(L, rst);
            else {
                rst = {ObjectView{rst.GetType(), rst.GetPtr()}, [rsrc_ref = neko_lua_ref{L}, buffer = rst.GetBuffer()](void*) {}};
                details::push(L, std::move(rst));
            }

            return 1;
        } catch (const std::runtime_error& e) {
            return L.error("%s::%s : Invoke exception.\n%s", type_name<Functor>().Data(), MetaName::Data(), e.what());
        } catch (...) {
            return L.error("%s::%s : Invoke exception.\n", type_name<Functor>().Data(), MetaName::Data());
        }
    }
}

template <typename T>
static int f_T_new(lua_State* L_) {
    neko_lua_state_view L{L_};
    int size = L.gettop();
    if (size == 0) {
        void* buffer = L.newuserdata(sizeof(T));
        new (buffer) T{};
    } else if (size == 1) {
        int type = L.type(-1);
        switch (type) {
            case LUA_TNUMBER: {
                auto value = details::auto_get<std::size_t>(L, 1);
                T t;
                if constexpr (std::is_same_v<T, Name>) {
                    name_id id{value};
                    std::string_view name = neko_refl_instance.nregistry.Viewof(id);
                    if (name.empty()) {
                        return L.error("%s::new: Not found name of ID (lua_Integer: %I)", type_name<T>().Data(), static_cast<lua_Integer>(value));
                    }
                    t = {name, id};
                } else {
                    TypeID id{value};
                    std::string_view name = neko_refl_instance.tregistry.Viewof(id);
                    if (name.empty()) {
                        return L.error("%s::new: Not found name of ID (lua_Integer: %I)", type_name<T>().Data(), static_cast<lua_Integer>(value));
                    }
                    t = {name, id};
                }
                void* buffer = L.newuserdata(sizeof(T));
                new (buffer) T{t};
                break;
            }
            case LUA_TSTRING: {
                size_t len;
                const char* str = L.tolstring(1, &len);
                void* buffer = L.newuserdata(sizeof(T));
                std::string_view sv{str, len};
                T t;
                if constexpr (std::is_same_v<T, Name>)
                    t = neko_refl_instance.nregistry.Register(sv);
                else
                    t = neko_refl_instance.tregistry.Register(sv);
                new (buffer) T{t};
                break;
            }
            default:
                return L.error("%s::new : The type of argument#1 is invalid. The function needs 0 argument / a string/integer.", type_name<T>().Data());
        }
    } else
        return L.error("%s::new : The number of arguments is invalid. The function needs 0 argument / a string/integer.", type_name<T>().Data());

    L.getmetatable(type_name<T>().Data());
    L.setmetatable(-2);
    return 1;
}

static int f_ObjectView_new(lua_State* L_) {
    neko_lua_state_view L{L_};
    int size = L.gettop();
    if (size == 0)
        details::push(L, ObjectView{});
    else if (size == 1) {
        int type = L.type(-1);
        switch (type) {
            case LUA_TUSERDATA: {
                auto* id = (Type*)L.checkudata(1, type_name<Type>().Data());
                details::push(L, ObjectView{*id});
                break;
            }
            default:
                return L.error("%s::new : The type of argument#1 is invalid. The function needs 0 argument / a Type.", type_name<ObjectView>().Data());
        }
    } else
        return L.error("%s::new : The number of arguments is invalid. The function needs 0 argument / a Type.", type_name<ObjectView>().Data());

    return 1;
}

static int f_ObjectView_tostring(lua_State* L_) {
    neko_lua_state_view L{L_};

    if (L.gettop() != 1) return L.error("%s::__tostring : The number of arguments is invalid. The function needs 1 argument (object).", type_name<ObjectView>().Data());

    auto ptr = details::auto_get<ObjectView>(L, 1);

    if (!ptr.GetPtr()) return L.error("%s::__tostring : The object is nil.", type_name<ObjectView>().Data());

    if (!ptr.IsInvocable<std::stringstream&>(NameIDRegistry::Meta::operator_shr)) {
        return L.error("%s::__tostring : The type (%s) can't convert to a string.", type_name<ObjectView>().Data(), ptr.GetType().get_name().data());
    }

    std::stringstream ss;
    try {
        ss << ptr;
    } catch (const std::runtime_error& e) {
        return L.error("%s::__tostring : Exception (<<).\n%s", type_name<ObjectView>().Data(), e.what());
    } catch (...) {
        return L.error("%s::__tostring : Exception (<<).", type_name<ObjectView>().Data());
    }
    auto str = ss.str();

    L.pushlstring(str.data(), str.size());

    return 1;
}

static int f_ObjectView_index(lua_State* L_) {
    neko_lua_state_view L{L_};

    if (L.gettop() != 2) return L.error("%s::__index : The number of arguments is invalid. The function needs 2 argument (object + key).", type_name<ObjectView>().Data());

    int type = L.type(2);

    Name key;
    switch (type) {
        case LUA_TUSERDATA: {
            if (auto* pName = (Name*)L.testudata(2, type_name<Name>().Data()))
                key = *pName;
            else
                return f_meta<ObjectView, details::Meta::t_operator_subscript, 2>(L_);

            break;
        }
        case LUA_TSTRING: {
            size_t len;
            const char* key_name = L.checklstring(2, &len);
            key = Name{std::string_view{key_name, len}};
            break;
        }
        default:
            return f_meta<ObjectView, details::Meta::t_operator_subscript, 2>(L_);
    }

    if (!key) return L.error("%s::__index : key is empty.", type_name<ObjectView>().Data());

    // order
    // 1. metatable
    // 2. var
    // 3. method
    // 4. ObjectView's method
    // 5. self

    constexpr auto contains_method = [](ObjectView obj, Name name) {
        auto methods = MethodRange{obj};
        return std::find_if(methods.begin(), methods.end(), [name](const auto& name_methodinfo) { return std::get<const Name>(name_methodinfo) == name; }) != methods.end();
    };

    if (L.getmetatable(1) && L.getfield(-1, key.get_view().data())) return 1;  // the field is already on the stack, so return directly

    auto ptr = details::auto_get<ObjectView>(L, 1);
    if (!ptr.GetType().valid()) return L.error("%s::__index : the type of object is invalid.", type_name<ObjectView>().Data());

    if (auto key_obj = ptr.Var(key); key_obj.GetPtr()) {
        auto* buffer = L.newuserdata(sizeof(ObjectView));
        new (buffer) ObjectView{key_obj};
        L.getmetatable(type_name<ObjectView>().Data());
        L.setmetatable(-2);
    } else if (contains_method(ptr, key)) {
        auto* buffer = L.newuserdata(sizeof(details::CallHandle));
        new (buffer) details::CallHandle{ptr.GetType(), key};
        L.getmetatable(type_name<details::CallHandle>().Data());
        L.setmetatable(-2);
    } else if (key.Is("self")) {
        L.pushvalue(1);
        return 1;
    } else if (contains_method(ObjectView_of<ObjectView>, key)) {
        auto* buffer = L.newuserdata(sizeof(details::CallHandle));
        new (buffer) details::CallHandle{Type_of<ObjectView>, key};
        L.getmetatable(type_name<details::CallHandle>().Data());
        L.setmetatable(-2);
    } else if (contains_method(ObjectView_of<SharedObject>, key)) {
        auto* buffer = L.newuserdata(sizeof(details::CallHandle));
        new (buffer) details::CallHandle{Type_of<SharedObject>, key};
        L.getmetatable(type_name<details::CallHandle>().Data());
        L.setmetatable(-2);
    } else {
        return L.error("%s::__index : %s index \"%s\" failed.", type_name<ObjectView>().Data(), ptr.GetType().get_name().data(), key.get_view().data());
    }

    return 1;
}

static int f_ObjectView_newindex(lua_State* L_) {
    neko_lua_state_view L{L_};
    if (L.gettop() != 3) {
        return L.error("%s::__newindex : The number of arguments is invalid. The function needs 3 argument (object, key, value).", type_name<ObjectView>().Data());
    }

    // stack : ptr, key, value
    L.getmetatable(type_name<ObjectView>().Data());
    L.getfield(-1, details::Meta::operator_assignment.Data());
    L.getfield(-2, "__index");
    L.rotate(1, -2);
    // stack : value, ..., __assignment, __index, ptr, key
    {
        int error = L.pcall(2, 1, 0);
        if (error) {
            return L.error("%s::__newindex: Call __index failed.\n%s", type_name<ObjectView>().Data(), L.tostring(-1));
        }
    }
    // stack : value, ..., __assignment, __index result (member ptr)
    L.rotate(1, -1);
    // stack : ..., __assignment, __index result (member ptr), value
    {
        int error = L.pcall(2, 1, 0);
        if (error) {
            return L.error("%s::__newindex: Call __assignment failed.\n%s", type_name<ObjectView>().Data(), L.tostring(-1));
        }
    }
    return 0;
}

static int f_ObjectView_range_next(lua_State* L_) {
    neko_lua_state_view L{L_};
    const int argnum = L.gettop();

    if (argnum != 1 && argnum != 2) {
        return L.error("%s::range_next : The number of arguments is invalid. The function needs 1/2 argument (end_iter[, iter/nil]).", type_name<ObjectView>().Data());
    }

    auto ptr = details::auto_get<ObjectView>(L, 1);
    SharedObject end_iter = ptr.end();

    if (!end_iter.GetType()) {
        return L.error("%s::range_next : The type (%s) can't invoke end.", type_name<ObjectView>().Data(), ptr.GetType().get_name().data());
    }
    if (argnum == 1) L.pushnil();

    int type = L.type(2);
    try {
        switch (type) {
            case LUA_TNIL: {
                SharedObject iter = ptr.begin();
                if (!iter.GetType()) {
                    return L.error("%s::range_next : The type (%s) can't invoke begin.", type_name<ObjectView>().Data(), ptr.GetType().get_name().data());
                }
                if (iter == end_iter)
                    L.pushnil();
                else
                    details::push<SharedObject>(L, std::move(iter));
                return 1;
            }
            case LUA_TUSERDATA: {
                ObjectView iter = *(SharedObject*)L.checkudata(2, type_name<SharedObject>().Data());
                SharedObject rst = ++iter;
                if (!rst.GetType()) {
                    return L.error("%s::range_next : The type (%s) can't invoke operator++().", type_name<ObjectView>().Data(), iter.GetType().get_name().data());
                }
                if (iter == end_iter) L.pushnil();
                return 1;  // stack top is the iter / nil
            }
            default:
                return L.error("%s::range_next : The second arguments must be a nil/iter.", type_name<ObjectView>().Data());
        }
    } catch (const std::runtime_error& e) {
        return L.error("%s::tuple_bind : Exception.\n%s", type_name<ObjectView>().Data(), e.what());
    } catch (...) {
        return L.error("%s::range_next : Exception.\n", type_name<ObjectView>().Data());
    }
}

static int f_ObjectView_range(lua_State* L_) {
    neko_lua_state_view L{L_};
    if (L.gettop() != 1) {
        return L.error("%s::range : The number of arguments is invalid. The function needs 1 argument (obj).", type_name<ObjectView>().Data());
    }
    details::auto_get<ObjectView>(L, 1);
    L.pushcfunction(f_ObjectView_range_next);
    L.pushvalue(1);
    L.pushnil();
    return 3;
}

static int f_ObjectView_concat(lua_State* L_) {
    neko_lua_state_view L{L_};

    if (L.gettop() != 2) {
        return L.error("%s::__concat : The number of arguments is invalid. The function needs 2 argument.", type_name<ObjectView>().Data());
    }

    if (L.testudata(1, type_name<ObjectView>().Data()) || L.testudata(1, type_name<SharedObject>().Data())) {
        L.pushcfunction(f_ObjectView_tostring);
        L.rotate(1, -1);
        int error = L.pcall(1, 1, 0);
        if (error) {
            return L.error("%s::__concat : The object call __tostring failed.\n", type_name<ObjectView>().Data(), L.tostring(-1));
        }
        L.rotate(1, 1);
    } else {
        L.rotate(1, 1);
        L.pushcfunction(f_ObjectView_tostring);
        L.rotate(1, -1);
        int error = L.pcall(1, 1, 0);
        if (error) {
            return L.error("%s::__concat : The object call __tostring failed.\n", type_name<ObjectView>().Data(), L.tostring(-1));
        }
    }

    L.concat(2);
    return 1;
}

static int f_ObjectView_tuple_bind(lua_State* L_) {
    neko_lua_state_view L{L_};
    auto obj = details::auto_get<ObjectView>(L, 1);
    try {
        std::size_t n = obj.tuple_size();
        for (std::size_t i{0}; i < n; i++) details::push(L, obj.get(i));
        return static_cast<int>(n);
    } catch (const std::runtime_error& e) {
        return L.error("%s::tuple_bind : Exception.\n%s", type_name<ObjectView>().Data(), e.what());
    } catch (...) {
        return L.error("%s::tuple_bind : Exception.\n", type_name<ObjectView>().Data());
    }
}

static int f_SharedObject_new(lua_State* L_) {
    neko_lua_state_view L{L_};
    const int L_argnum = L.gettop();
    if (L_argnum <= 0) return L.error("%s::new : The number of arguments is invalid.", type_name<SharedObject>().Data());

    Type type;
    int argtype = L.type(1);
    int argnum = 0;
    int arg_begin = 0;
    switch (argtype) {
        case LUA_TSTRING:
            type = Type{details::auto_get<std::string_view>(L, 1)};
            argnum = L_argnum - 1;
            arg_begin = L_argnum - argnum + 1;
            break;
        case LUA_TUSERDATA:
            type = *static_cast<Type*>(L.checkudata(1, type_name<Type>().Data()));
            argnum = L_argnum - 1;
            arg_begin = L_argnum - argnum + 1;
            break;
        case LUA_TTABLE:
            // { type = "...", init_args = { args... } }
            if (L_argnum != 1) return L.error("%s::new : The 1st argument is table, so the number of arguments must be 1.", type_name<SharedObject>().Data());
            L.getfield(1, "type");
            type = details::auto_get<Type>(L, -1);
            if (auto init_args_type = L.getfield(1, "init_args")) {
                if (init_args_type != LUA_TTABLE) return L.error("%s::new : The type of init_args must be table.", type_name<SharedObject>().Data());
                int init_args_index = L.gettop();
                arg_begin = init_args_index + 1;
                argnum = static_cast<int>(L.lenL(init_args_index));
                L.checkstack(argnum);
                for (lua_Integer i = 1; i <= static_cast<lua_Integer>(argnum); i++) L.geti(init_args_index, i);
            } else {
                arg_begin = L_argnum - argnum + 1;
                argnum = 0;
            }

            break;
        default:
            return L.error("%s::new : The function doesn't support %s.", type_name<SharedObject>().Data(), L.typename_(1));
    }

    details::ArgStack argstack;
    int error = details::FillArgStack(L, argstack, arg_begin, argnum);

    if (error) {
        return L.error("%s::new : FillArgStack Failed\n%s", type_name<SharedObject>().Data(), L.tostring(-1));
    }

    {  // get unsync resource
        int success = L.getfield(LUA_REGISTRYINDEX, details::UnsyncRsrc);
        neko_assert(success);
    }
    auto* rsrc = (std::pmr::unsynchronized_pool_resource*)L.touserdata(-1);
    SharedObject obj;
    try {
        obj = neko_refl_instance.MMakeShared(type, rsrc, {argstack.argptr_buffer, {reinterpret_cast<Type*>(argstack.argType_buffer), static_cast<std::size_t>(argnum)}});
    } catch (const std::runtime_error& e) {
        return L.error("%s::new : Fail.\n%s", type_name<SharedObject>().Data(), e.what());
    } catch (...) {
        return L.error("%s::new : Fail.", type_name<SharedObject>().Data());
    }

    if (!obj.GetType()) return L.error("%s::new : Fail.", type_name<SharedObject>().Data());

    obj = {ObjectView{obj.GetType(), obj.GetPtr()}, [rsrc_ref = neko_lua_ref{L}, buffer = obj.GetBuffer()](void*) {}};

    details::push(L, std::move(obj));

    return 1;
}

static int f_SharedObject_new_MethodPtr(lua_State* L_) {
    neko_lua_state_view L{L_};

    int L_argnum = L.gettop();

    SharedObject methodptr_obj;
    neko_lua_ref func_ref;
    Type result_type = Type_of<void>;
    Type object_type;
    ParamList list;

    // [object type, ]function[, result type = Type_of<void>][, ParamList = {}]

    constexpr auto GetParamList = [](neko_lua_state_view L, int idx) -> ParamList {
        auto obj = details::auto_get<ObjectView>(L, idx).RemoveConst();
        if (!obj.GetType().Is<ParamList>()) {
            L.error("%s::new_MethodPtr :"
                    "The %dth arguments (%s) isn't ParamList.",
                    type_name<SharedObject>().Data(), idx, obj.GetType().get_name().data());
            return {};
        }
        return obj.As<ParamList>();
    };

    switch (L_argnum) {
        case 1:  // function
            L.checktype(1, LUA_TFUNCTION);
            func_ref = std::move(neko_lua_ref{L});
            break;
        case 2:                                                // object type + function | function + result type | function + paramlist
            if (L.type(1) == LUA_TFUNCTION) {                  // function + result type | function + paramlist
                if (L.testudata(2, type_name<Type>().Data()))  // function + result type
                    result_type = details::auto_get<Type>(L, 2);
                else if (L.testudata(2, type_name<ParamList>().Data()))  // function + paramlist
                    list = GetParamList(L, 2);
                else {
                    return L.error("%s::new_MethodPtr: The 2nd argument should be a Type/ParamList.", type_name<SharedObject>().Data());
                }
                L.pop(1);
                func_ref = std::move(neko_lua_ref{L});
            } else {
                // object type + function
                object_type = details::auto_get<Type>(L, 1);
                func_ref = std::move(neko_lua_ref{L});
            }
            break;
        case 3:                                // object type + function + result type | object type + function + paramlist | function + result type + paramlist
            if (L.type(1) == LUA_TFUNCTION) {  // function + result type + paramlist
                result_type = details::auto_get<Type>(L, 2);
                list = GetParamList(L, 3);
                L.pop(2);
                func_ref = std::move(neko_lua_ref{L});
            } else {  // object type + function + result type | object type + function + paramlist
                L.checktype(2, LUA_TFUNCTION);
                object_type = details::auto_get<Type>(L, 1);
                if (L.testudata(3, type_name<Type>().Data()))  // object type + function + result type
                    result_type = details::auto_get<Type>(L, 3);
                else if (L.testudata(3, type_name<ParamList>().Data()))  // object type + function + paramlist
                    list = GetParamList(L, 3);
                else {
                    return L.error("%s::new_MethodPtr: The 2nd argument should be a Type/ParamList.", type_name<SharedObject>().Data());
                }
                L.pop(1);
                func_ref = std::move(neko_lua_ref{L});
            }
            break;
        case 4:  // object type + function + result type + paramlist
            L.checktype(2, LUA_TFUNCTION);
            object_type = details::auto_get<Type>(L, 1);
            result_type = details::auto_get<Type>(L, 3);
            list = GetParamList(L, 4);
            L.pop(2);
            func_ref = std::move(neko_lua_ref{L});
            break;
        default:
            return L.error(
                    "%s::new_MethodPtr :"
                    "The number of arguments (%d) is invalid. The function needs 1~4 arguments([object type, ]function[, result type][, ParamList]).",
                    type_name<SharedObject>().Data(), L_argnum);
    }

    neko_assert(result_type);

    if (result_type.IsConst()) {
        return L.error(
                "%s::new_MethodPtr :"
                "The result type ($s) must be non-const.",
                type_name<SharedObject>().Data(), result_type.get_name().data());
    }

    MethodFlag flag;
    if (!object_type.get_id().Valid())
        flag = MethodFlag::Static;
    else if (object_type.IsConst())
        flag = MethodFlag::Const;
    else
        flag = MethodFlag::Variable;

    methodptr_obj = {Type_of<MethodPtr>,
                     std::make_shared<MethodPtr>(details::LuaFuncToMethodPtrFunc(neko_refl_instance.tregistry.RegisterAddLValueReference(object_type), flag, result_type, std::move(func_ref)), flag,
                                                 result_type, std::move(list))};

    void* buffer = L.newuserdata(sizeof(SharedObject));
    L.setmetatable(type_name<SharedObject>().Data());
    new (buffer) SharedObject{std::move(methodptr_obj)};

    return 1;
}

static int f_UDRefl_box(lua_State* L_) {
    neko_lua_state_view L{L_};
    const int L_argnum = L.gettop();
    if (L_argnum != 1) return L.error("%s::box : The number of arguments must be 1.", type_name<SharedObject>().Data());

    Type type;
    int argtype = L.type(1);
    switch (argtype) {
        case LUA_TNIL:
            details::push(L, neko_refl_instance.MakeShared(Type_of<std::nullptr_t>));
            break;
        case LUA_TBOOLEAN:
            details::push(L, neko_refl_instance.MakeShared(Type_of<bool>, TempArgsView{details::auto_get<bool>(L, 1)}));
            break;
        case LUA_TLIGHTUSERDATA:
            details::push(L, neko_refl_instance.MakeShared(Type_of<void*>, TempArgsView{details::auto_get<void*>(L, 1)}));
            break;
        case LUA_TNUMBER:
            if (L.isinteger(1))
                details::push(L, neko_refl_instance.MakeShared(Type_of<lua_Integer>, TempArgsView{details::auto_get<lua_Integer>(L, 1)}));
            else {
                neko_assert(L.isnumber(1));
                details::push(L, neko_refl_instance.MakeShared(Type_of<lua_Number>, TempArgsView{details::auto_get<lua_Number>(L, 1)}));
            }
            break;
        case LUA_TSTRING:
            details::push(L, neko_refl_instance.MakeShared(Type_of<std::string>, TempArgsView{details::auto_get<std::string_view>(L, 1)}));
            break;
        case LUA_TUSERDATA:
            if (L.testudata(1, type_name<Type>().Data()))
                details::push(L, neko_refl_instance.MakeShared(Type_of<Type>, TempArgsView{details::auto_get<Type>(L, 1)}));
            else if (L.testudata(1, type_name<Name>().Data()))
                details::push(L, neko_refl_instance.MakeShared(Type_of<Name>, TempArgsView{details::auto_get<Name>(L, 1)}));
            else {
                return L.error("%s::box : In userdata, only Type & Name can be boxed.", type_name<SharedObject>().Data());
            }
            break;
        default:
            return L.error("%s::box : The type (%s) of the argument is not support.", type_name<SharedObject>().Data(), L.typename_(1));
    }

    return 1;
}

static int f_UDRefl_unbox(lua_State* L_) {
    neko_lua_state_view L{L_};

    if (L.gettop() != 1) return L.error("%s::__unbox : The number of arguments is invalid. The function needs 1 argument (object).", type_name<ObjectView>().Data());

    auto ptr = details::auto_get<ObjectView>(L, 1);

    if (!ptr.GetPtr()) return L.error("%s::__unbox : The object is nil.", type_name<ObjectView>().Data());

    ptr = ptr.RemoveConstReference();

    switch (ptr.GetType().get_id().GetValue()) {
        case TypeID_of<bool>.GetValue():
            L.pushboolean(ptr.As<bool>());
            break;
        case TypeID_of<std::int8_t>.GetValue():
            L.pushinteger(static_cast<lua_Integer>(ptr.As<std::int8_t>()));
            break;
        case TypeID_of<std::int16_t>.GetValue():
            L.pushinteger(static_cast<lua_Integer>(ptr.As<std::int16_t>()));
            break;
        case TypeID_of<std::int32_t>.GetValue():
            L.pushinteger(static_cast<lua_Integer>(ptr.As<std::int32_t>()));
            break;
        case TypeID_of<std::int64_t>.GetValue():
            L.pushinteger(static_cast<lua_Integer>(ptr.As<std::int64_t>()));
            break;
        case TypeID_of<std::uint8_t>.GetValue():
            L.pushinteger(static_cast<lua_Integer>(ptr.As<std::uint8_t>()));
            break;
        case TypeID_of<std::uint16_t>.GetValue():
            L.pushinteger(static_cast<lua_Integer>(ptr.As<std::uint16_t>()));
            break;
        case TypeID_of<std::uint32_t>.GetValue():
            L.pushinteger(static_cast<lua_Integer>(ptr.As<std::uint32_t>()));
            break;
        case TypeID_of<std::uint64_t>.GetValue():
            L.pushinteger(static_cast<lua_Integer>(ptr.As<std::uint64_t>()));
            break;
        case TypeID_of<float>.GetValue():
            L.pushnumber(static_cast<lua_Number>(ptr.As<float>()));
            break;
        case TypeID_of<double>.GetValue():
            L.pushnumber(static_cast<lua_Number>(ptr.As<double>()));
            break;
        case TypeID_of<void*>.GetValue():
            L.pushlightuserdata(ptr.As<void*>());
            break;
        case TypeID_of<std::nullptr_t>.GetValue():
            L.pushnil();
            break;
        case TypeID_of<Type>.GetValue():
            details::push(L, ptr.As<Type>());
            break;
        case TypeID_of<Name>.GetValue():
            details::push(L, ptr.As<Name>());
            break;
        case TypeID_of<const char*>.GetValue():
            L.pushstring(ptr.As<const char*>());
            break;
        case TypeID_of<char*>.GetValue():
            L.pushstring(ptr.As<char*>());
            break;
        case TypeID_of<std::string_view>.GetValue():
            details::push(L, ptr.As<std::string_view>());
            break;
        case TypeID_of<std::string>.GetValue():
            details::push<std::string_view>(L, ptr.As<std::string>());
            break;
        case TypeID_of<ObjectView>.GetValue():
            details::push<ObjectView>(L, ptr.As<ObjectView>());
            break;
        case TypeID_of<SharedObject>.GetValue():
            details::push<SharedObject>(L, ptr.As<SharedObject>());
            break;
        default:
            return L.error("%s::__unbox : The type (%s) can't unbox.", type_name<ObjectView>().Data(), ptr.GetType().get_name().data());
            break;
    }

    return 1;
}

static int f_UDRefl_RegisterType(lua_State* L_) {
    neko_lua_state_view L{L_};
    if (L.gettop() != 1) return L.error("RegisterType : The number of arguments must be 1");
    L.checktype(1, LUA_TTABLE);

    Type type;
    AttrSet type_attrs;

    std::vector<Type> bases;

    std::vector<Name> field_names;
    std::vector<Type> field_types;
    std::map<std::size_t, AttrSet> fields_attrs;

    std::vector<Name> method_names;
    std::vector<MethodPtr> methodptrs;
    std::map<std::size_t, AttrSet> methods_attrs;

    std::vector<Name> unowned_field_names;
    std::vector<SharedObject> unowned_field_objs;
    std::map<std::size_t, AttrSet> unowned_fields_attrs;

    {  // name
        L.getfield(1, "type");
        type = details::auto_get<Type>(L, -1);
        if (int attrs_type = L.getfield(1, "attrs")) {
            if (attrs_type != LUA_TTABLE) return L.error("RegisterType : attrs must be a table");

            lua_Integer attr_num = L.lenL(-1);
            L.pushcfunction(f_SharedObject_new);
            for (lua_Integer i = 1; i <= attr_num; i++) {
                L.pushvalue(-1);               // SharedObject.new
                L.geti(-3, i);                 // attrs[i]
                int error = L.pcall(1, 1, 0);  // SharedObject.new({...})
                if (error) {
                    return L.error("RegisterType: Call %s::new failed.\n%s", type_name<SharedObject>().Data(), L.tostring(-1));
                }
                auto attr = details::auto_get<SharedObject>(L, -1);
                auto target = type_attrs.find(attr);
                if (target != type_attrs.end()) return L.error("RegisterType: Same attr (%s)of type.\n%s", attr.GetType().get_name());
                type_attrs.insert(target, attr);
                L.pop(1);  // SharedObject
            }
            L.pop(1);  // f_SharedObject_new
        }
        L.pop(2);  // bases, attrs
    }

    do {  // bases
        auto type = L.getfield(1, "bases");
        if (type == LUA_TNIL) break;

        if (type != LUA_TTABLE) return L.error("RegisterType : table's bases must be a table");

        lua_Integer len = L.lenL(-1);
        for (lua_Integer i = 1; i <= len; i++) {
            L.geti(-1, i);
            bases.push_back(details::auto_get<Type>(L, -1));
            L.pop(1);
        }
        L.pop(1);  // bases
    } while (false);

    do {  // fields
        auto type = L.getfield(1, "fields");
        if (type == LUA_TNIL) break;

        if (type != LUA_TTABLE) return L.error("RegisterType : table's fields must be a table");
        int fields_index = L.gettop();
        lua_Integer len = L.lenL(fields_index);
        field_types.reserve(len);
        field_names.reserve(len);
        for (lua_Integer i = 1; i <= len; i++) {
            if (L.geti(fields_index, i) != LUA_TTABLE) return L.error("RegisterType : element of table's fields must be a table");
            int field_index = L.gettop();
            L.getfield(field_index, "type");
            field_types.push_back(details::auto_get<Type>(L, -1));
            L.getfield(field_index, "name");
            Name field_name = details::auto_get<Name>(L, -1);
            field_names.push_back(field_name);

            if (int attrs_type = L.getfield(field_index, "attrs")) {
                if (attrs_type != LUA_TTABLE) return L.error("RegisterType : attrs must be a table");

                lua_Integer attr_num = L.lenL(-1);
                L.pushcfunction(f_SharedObject_new);
                for (lua_Integer i = 1; i <= attr_num; i++) {
                    L.pushvalue(-1);               // SharedObject.new
                    L.geti(-3, i);                 // attrs[i]
                    int error = L.pcall(1, 1, 0);  // SharedObject.new({...})
                    if (error) {
                        return L.error("RegisterType: Call %s::new failed.\n%s", type_name<SharedObject>().Data(), L.tostring(-1));
                    }
                    auto attr = details::auto_get<SharedObject>(L, -1);
                    auto& field_i_attrs = fields_attrs[i - 1];
                    auto target = field_i_attrs.find(attr);
                    if (target != field_i_attrs.end()) {
                        return L.error("RegisterType: Same attr (%s)of field(%s).\n%s", attr.GetType().get_name(), field_name.get_view().data());
                    }
                    field_i_attrs.insert(target, attr);
                    L.pop(1);  // SharedObject
                }
                L.pop(1);  // f_SharedObject_new
            }

            L.pop(4);  // table, type, name, attrs
        }

        L.pop(1);  // fields
    } while (false);

    bool contains_ctor = false;
    bool contains_dtor = false;
    do {  // methods
        auto ftype = L.getfield(1, "methods");
        if (ftype == LUA_TNIL) break;

        if (ftype != LUA_TTABLE) return L.error("RegisterType : table's methods must be a table");

        lua_Integer mlen = L.lenL(-1);
        method_names.reserve(mlen);
        methodptrs.reserve(mlen);
        for (lua_Integer i = 1; i <= mlen; i++) {
            if (L.geti(-1, i) != LUA_TTABLE) return L.error("RegisterType : element of table's methods must be a table");
            int methodidx = L.gettop();
            L.getfield(methodidx, "name");
            Name method_name = details::auto_get<Name>(L, -1);
            if (method_name == NameIDRegistry::Meta::ctor)
                contains_ctor = true;
            else if (method_name == NameIDRegistry::Meta::dtor)
                contains_dtor = true;

            method_names.push_back(method_name);

            MethodFlag flag;
            if (L.getfield(methodidx, "flag") != LUA_TNIL) {
                auto flagname = details::auto_get<std::string_view>(L, -1);
                if (flagname == "Variable")
                    flag = MethodFlag::Variable;
                else if (flagname == "Const")
                    flag = MethodFlag::Const;
                else if (flagname == "Static")
                    flag = MethodFlag::Static;
                else
                    return L.error("RegisterType : flag(%s) of table's methods[%I] must be a Variable/Const/Static.", flagname.data(), i);
            } else
                flag = MethodFlag::Variable;

            Type result_type;
            if (L.getfield(methodidx, "result") != LUA_TNIL)
                result_type = details::auto_get<Type>(L, -1);
            else
                result_type = Type_of<void>;

            if (L.getfield(methodidx, "body") != LUA_TFUNCTION) return L.error("RegisterType : body of table's methods[%I] must be a function.", i);
            neko_lua_ref func_ref{L};  // pop

            ParamList params;
            if (auto fparamstype = L.getfield(-4, "params"); fparamstype != LUA_TNIL) {
                if (fparamstype != LUA_TTABLE) return L.error("RegisterType : params of table's methods[%I] must be a table", i);
                lua_Integer len = L.lenL(-1);
                params.reserve(len);
                for (lua_Integer i = 1; i <= len; i++) {
                    L.geti(-1, i);
                    params.push_back(details::auto_get<Type>(L, -1));
                    L.pop(1);
                }
            }
            methodptrs.emplace_back(details::LuaFuncToMethodPtrFunc(neko_refl_instance.tregistry.RegisterAddLValueReference(type), flag, result_type, std::move(func_ref)), flag, result_type,
                                    std::move(params));

            if (int attrs_type = L.getfield(methodidx, "attrs")) {
                if (attrs_type != LUA_TTABLE) return L.error("RegisterType : attrs must be a table");

                lua_Integer attr_num = L.lenL(-1);
                L.pushcfunction(f_SharedObject_new);
                for (lua_Integer i = 1; i <= attr_num; i++) {
                    L.pushvalue(-1);               // SharedObject.new
                    L.geti(-3, i);                 // attrs[i]
                    int error = L.pcall(1, 1, 0);  // SharedObject.new({...})
                    if (error) {
                        return L.error("RegisterType: Call %s::new failed.\n%s", type_name<SharedObject>().Data(), L.tostring(-1));
                    }
                    auto attr = details::auto_get<SharedObject>(L, -1);
                    auto& method_i_attrs = methods_attrs[i - 1];
                    auto target = method_i_attrs.find(attr);
                    if (target != method_i_attrs.end()) {
                        return L.error("RegisterType: Same attr (%s)of method(%s).\n%s", attr.GetType().get_name(), method_name.get_view().data());
                    }
                    method_i_attrs.insert(target, attr);
                    L.pop(1);  // SharedObject
                }
                L.pop(1);  // f_SharedObject_new
            }

            L.pop(6);  // method, name, flag, result, body, attrs
        }
    } while (false);

    do {  // unowned_fields
        auto ftype = L.getfield(1, "unowned_fields");
        if (ftype == LUA_TNIL) break;

        if (ftype != LUA_TTABLE) return L.error("RegisterType : table's unowned_fields must be a table");

        int unowned_fields_index = L.gettop();

        {  // get unsync resource
            int success = L.getfield(LUA_REGISTRYINDEX, details::UnsyncRsrc);
            neko_assert(success);
        }
        auto* rsrc = (std::pmr::unsynchronized_pool_resource*)L.touserdata(-1);

        lua_Integer flen = L.lenL(unowned_fields_index);
        unowned_field_names.reserve(flen);
        unowned_field_objs.reserve(flen);
        for (lua_Integer i = 1; i <= flen; i++) {
            if (L.geti(unowned_fields_index, i) != LUA_TTABLE) return L.error("RegisterType : element of table's unowned_fields must be a table");

            int unowned_field_index = L.gettop();

            L.getfield(unowned_field_index, "name");
            Name unowned_field_name = details::auto_get<Name>(L, -1);
            unowned_field_names.push_back(unowned_field_name);

            L.pushcfunction(f_SharedObject_new);
            L.pushvalue(unowned_field_index);
            int error = L.pcall(1, 1, 0);  // SharedObject.new({...})
            if (error) {
                return L.error("RegisterType: Call %s::new failed.\n%s", type_name<SharedObject>().Data(), L.tostring(-1));
            }
            unowned_field_objs.push_back(details::auto_get<SharedObject>(L, -1));

            if (int attrs_type = L.getfield(unowned_field_index, "attrs")) {
                if (attrs_type != LUA_TTABLE) return L.error("RegisterType : attrs must be a table");

                lua_Integer attr_num = L.lenL(-1);
                L.pushcfunction(f_SharedObject_new);
                for (lua_Integer i = 1; i <= attr_num; i++) {
                    L.pushvalue(-1);               // SharedObject.new
                    L.geti(-3, i);                 // attrs[i]
                    int error = L.pcall(1, 1, 0);  // SharedObject.new({...})
                    if (error) {
                        return L.error("RegisterType: Call %s::new failed.\n%s", type_name<SharedObject>().Data(), L.tostring(-1));
                    }
                    auto attr = details::auto_get<SharedObject>(L, -1);
                    auto& unowned_field_i_attrs = unowned_fields_attrs[i - 1];
                    auto target = unowned_field_i_attrs.find(attr);
                    if (target != unowned_field_i_attrs.end()) {
                        return L.error("RegisterType: Same attr (%s)of unowned_field(%s).\n%s", attr.GetType().get_name(), unowned_field_name.get_view().data());
                    }
                    unowned_field_i_attrs.insert(target, attr);
                    L.pop(1);  // SharedObject
                }
                L.pop(1);  // f_SharedObject_new
            }

            L.pop(2);  // table, name
        }
    } while (false);

    Type rst = neko_refl_instance.RegisterType(type, bases, field_types, field_names, !contains_ctor && !contains_dtor);
    if (!rst) return L.error("RegisterType : Call neko::cpp::ReflMngr::RegisterType failed.");

    for (std::size_t i = 0; i < method_names.size(); i++) {
        Name mrst = neko_refl_instance.AddMethod(type, method_names[i], MethodInfo{std::move(methodptrs[i])});
        if (!mrst) {
            neko_refl_instance.typeinfos.erase(rst);
            return L.error("RegisterType : Call neko::cpp::ReflMngr::AddMethod for %s failed.", method_names[i].get_view().data());
        }
    }

    if (!contains_ctor) neko_refl_instance.AddDefaultConstructor(type);
    if (!contains_dtor) neko_refl_instance.AddDestructor(type);

    for (std::size_t i = 0; i < unowned_field_names.size(); i++) {
        Name frst = neko_refl_instance.AddField(type, unowned_field_names[i], FieldInfo{FieldPtr{std::move(unowned_field_objs[i])}});
        if (!frst) {
            neko_refl_instance.typeinfos.erase(rst);
            return L.error("RegisterType : Call neko::cpp::ReflMngr::AddField for unowned %s failed.", unowned_field_names[i].get_view().data());
        }
    }

    for (auto attr : type_attrs) neko_refl_instance.AddTypeAttr(type, attr);

    for (const auto& [idx, attrs] : fields_attrs) {
        for (auto attr : attrs) neko_refl_instance.AddFieldAttr(type, field_names[idx], attr);
    }

    for (const auto& [idx, attrs] : methods_attrs) {
        for (auto attr : attrs) neko_refl_instance.AddMethodAttr(type, method_names[idx], attr);
    }

    for (const auto& [idx, attrs] : unowned_fields_attrs) {
        for (auto attr : attrs) neko_refl_instance.AddFieldAttr(type, unowned_field_names[idx], attr);
    }

    details::push(L, rst);
    return 1;
}

static const struct luaL_Reg lib_Name[] = {"new", f_T_new<Name>, NULL, NULL};

static const struct luaL_Reg meta_Name[] = {"GetID",   details::wrap<&Name::get_id, Name>(TSTR("GetID")),
                                            "GetView", details::wrap<&Name::get_view, Name>(TSTR("GetView")),
                                            "Valid",   details::wrap<&Name::valid, Name>(TSTR("Valid")),
                                            "Is",      details::wrap<&Name::Is, Name>(TSTR("Is")),
                                            NULL,      NULL};

static const struct luaL_Reg lib_Type[] = {"new", f_T_new<Type>, NULL, NULL};

static const struct luaL_Reg meta_Type[] = {"GetID",    details::wrap<&Type::get_id, Type>(TSTR("GetID")),
                                            "get_name", details::wrap<&Type::get_name, Type>(TSTR("get_name")),
                                            "Valid",    details::wrap<&Type::valid, Type>(TSTR("Valid")),
                                            "Is",       details::wrap<MemFuncOf<Type, bool(std::string_view) const noexcept>::get(&Type::Is), Type>(TSTR("Is")),
                                            NULL,       NULL};

static const struct luaL_Reg lib_ObjectView[] = {"new", f_ObjectView_new, "range", f_ObjectView_range, "tuple_bind", f_ObjectView_tuple_bind, NULL, NULL};

static const struct luaL_Reg meta_ObjectView[] = {
        "__add", &f_meta<ObjectView, details::Meta::t_operator_add, 2, details::Invalid, true>, "__band", &f_meta<ObjectView, details::Meta::t_operator_band, 2, details::Invalid, true>, "__bnot",
        &f_meta<ObjectView, details::Meta::t_operator_bnot, 1>, "__bor", &f_meta<ObjectView, details::Meta::t_operator_bor, 2, details::Invalid, true>, "__bxor",
        &f_meta<ObjectView, details::Meta::t_operator_bxor, 2, details::Invalid, true>, "__call", &f_meta<ObjectView, details::Meta::t_operator_call>, "__concat", f_ObjectView_concat, "__div",
        &f_meta<ObjectView, details::Meta::t_operator_div, 2>, "__eq", &f_meta<ObjectView, details::Meta::t_operator_eq, 2, bool, true>, "__index", f_ObjectView_index, "__le",
        &f_meta<ObjectView, details::Meta::t_operator_le, 2, bool>, "__lt", &f_meta<ObjectView, details::Meta::t_operator_lt, 2, bool>, "__mod", &f_meta<ObjectView, details::Meta::t_operator_mod, 2>,
        "__mul", &f_meta<ObjectView, details::Meta::t_operator_mul, 2, details::Invalid, true>, "__newindex", f_ObjectView_newindex,
        // ^ is bxor in c++
        "__pow", &f_meta<ObjectView, details::Meta::t_operator_bxor, 2, details::Invalid, true>, "__shl", &f_meta<ObjectView, details::Meta::t_operator_shl, 2>, "__shr",
        &f_meta<ObjectView, details::Meta::t_operator_shr, 2>, "__sub", &f_meta<ObjectView, details::Meta::t_operator_sub, 2>, "__unbox", f_UDRefl_unbox, "__tostring", f_ObjectView_tostring, "__unm",
        &f_meta<ObjectView, details::Meta::t_operator_sub, 1>,

        "__assignment", &f_meta<ObjectView, details::Meta::t_operator_assignment, 2>, "__begin", &f_meta<ObjectView, details::Meta::t_container_begin, 1>, "__end",
        &f_meta<ObjectView, details::Meta::t_container_end, 1>, "__range", f_ObjectView_range, "__tuple_bind", f_ObjectView_tuple_bind,

        NULL, NULL};

static const struct luaL_Reg lib_SharedObject[] = {"new", f_SharedObject_new, "new_MethodPtr", f_SharedObject_new_MethodPtr, NULL, NULL};

static const struct luaL_Reg lib_UDRefl[] = {"RegisterType", f_UDRefl_RegisterType, "box", f_UDRefl_box, "unbox", f_UDRefl_unbox, NULL, NULL};

static const struct luaL_Reg* meta_SharedObject = meta_ObjectView;

static void init_CallHandle(lua_State* L_) {
    neko_lua_state_view L{L_};

    L.newmetatable(type_name<details::CallHandle>().Data());
    L.pushcfunction(&f_meta<details::CallHandle, details::Meta::t_operator_call>);
    L.setfield(-2, "__call");

    L.pop(1);
}

static int luaopen_Name(lua_State* L_) {
    neko_lua_state_view L{L_};
    L.newmetatable(type_name<Name>().Data());
    L.setfuncs(meta_Name, 0);
    L.pushvalue(-1);
    L.setfield(-2, "__index");
    L.newlib(lib_Name);
    return 1;
}

static int luaopen_Type(lua_State* L_) {
    neko_lua_state_view L{L_};
    L.newmetatable(type_name<Type>().Data());
    L.setfuncs(meta_Type, 0);
    L.pushvalue(-1);
    L.setfield(-2, "__index");
    L.newlib(lib_Type);
    return 1;
}

static int luaopen_ObjectView(lua_State* L_) {
    neko_lua_state_view L{L_};
    L.newmetatable(type_name<ObjectView>().Data());
    L.setfuncs(meta_ObjectView, 0);
    L.newlib(lib_ObjectView);
    {  // register Global
        void* buffer = L.newuserdata(sizeof(ObjectView));
        new (buffer) ObjectView{Global};
        L.setmetatable(type_name<ObjectView>().Data());
        L.setfield(-2, "Global");
    }
    {  // register ReflMngr
        __neko_dyrefl_ext_Bootstrap();
        void* buffer = L.newuserdata(sizeof(ObjectView));
        new (buffer) ObjectView{neko_refl_view};
        L.setmetatable(type_name<ObjectView>().Data());
        L.setfield(-2, "ReflMngr");
    }
    return 1;
}

static int luaopen_SharedObject(lua_State* L_) {
    neko_lua_state_view L{L_};
    L.newmetatable(type_name<SharedObject>().Data());
    L.pushcfunction(details::wrap_dtor<SharedObject>());
    L.setfield(-2, "__gc");
    L.setfuncs(meta_SharedObject, 0);
    L.newlib(lib_SharedObject);
    return 1;
}

static int luaopen_UDRefl(lua_State* L_) {
    neko_lua_state_view L{L_};
    L.newlib(lib_UDRefl);
    return 1;
}

static const luaL_Reg __neko_refl_lua_libs[] = {
        {"Name", luaopen_Name}, {"Type", luaopen_Type}, {"ObjectView", luaopen_ObjectView}, {"SharedObject", luaopen_SharedObject}, {"UDRefl", luaopen_UDRefl}, {NULL, NULL}};

extern void neko_refl_lua(lua_State* L_) {
    init_CallHandle(L_);

    neko_lua_state_view L{L_};

    void* buffer = L.newuserdata(sizeof(std::pmr::unsynchronized_pool_resource));
    new (buffer) std::pmr::unsynchronized_pool_resource{};
    int success = L.newmetatable(type_name<std::pmr::unsynchronized_pool_resource>().Data());
    L.pushcfunction(details::wrap_dtor<std::pmr::unsynchronized_pool_resource>());
    L.setfield(-2, "__gc");
    L.setmetatable(-2);
    L.setfield(LUA_REGISTRYINDEX, details::UnsyncRsrc);
    L.pop(1);

    const luaL_Reg* lib;

    for (lib = __neko_refl_lua_libs; lib->func; lib++) {
        L.requiref(lib->name, lib->func, 1);
        lua_pop(L, 1);  // remove lib
    }
}
