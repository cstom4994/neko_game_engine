
#include "engine/meta/neko_refl.hpp"

#include <string>

#include "gen/neko_refl_util_gen.hpp"

using namespace neko::cpp;

namespace neko::cpp::details {
static ObjectView StaticCast_BaseToDerived(ObjectView obj, Type type) {
    neko_assert(obj.GetType().GetCVRefMode() == CVRefMode::None);

    if (obj.GetType() == type) return obj;

    auto target = neko_refl_instance.typeinfos.find(type);
    if (target == neko_refl_instance.typeinfos.end()) return {};

    const auto& typeinfo = target->second;

    for (const auto& [base, baseinfo] : typeinfo.baseinfos) {
        auto ptr = StaticCast_BaseToDerived(obj, base);
        if (ptr.GetType()) return {base, baseinfo.IsVirtual() ? nullptr : baseinfo.StaticCast_BaseToDerived(obj.GetPtr())};
    }

    return {};
}

static ObjectView DynamicCast_BaseToDerived(ObjectView obj, Type type) {
    neko_assert(obj.GetType().GetCVRefMode() == CVRefMode::None);

    if (obj.GetType() == type) return obj;

    auto target = neko_refl_instance.typeinfos.find(obj.GetType());
    if (target == neko_refl_instance.typeinfos.end()) return {};

    const auto& typeinfo = target->second;

    for (const auto& [base, baseinfo] : typeinfo.baseinfos) {
        auto ptr = DynamicCast_BaseToDerived(ObjectView{base, baseinfo.DynamicCast_BaseToDerived(obj.GetPtr())}, type);
        if (ptr.GetType()) return {base, baseinfo.IsPolymorphic() ? baseinfo.DynamicCast_BaseToDerived(obj.GetPtr()) : nullptr};
    }

    return {};
}
}  // namespace neko::cpp::details

neko_refl::neko_refl() : temporary_resource{std::make_shared<std::pmr::synchronized_pool_resource>()}, object_resource{std::make_shared<std::pmr::synchronized_pool_resource>()} {
    RegisterType(GlobalType, 0, 1, false, true);

    details::__neko_refl_util_gen_0(*this);
    details::__neko_refl_util_gen_1(*this);
    details::__neko_refl_util_gen_2(*this);
    details::__neko_refl_util_gen_3(*this);
    details::__neko_refl_util_gen_4(*this);
    details::__neko_refl_util_gen_5(*this);
    details::__neko_refl_util_gen_6(*this);
    details::__neko_refl_util_gen_7(*this);
}

neko_refl& neko_refl::Instance() noexcept {
    static neko_refl instance;
    return instance;
}

TypeInfo* neko_refl::GetTypeInfo(Type type) const {
    auto target = typeinfos.find(type.RemoveCVRef());
    if (target == typeinfos.end()) return nullptr;
    return const_cast<TypeInfo*>(&target->second);
}

SharedObject neko_refl::GetTypeAttr(Type type, Type attr_type) const {
    TypeInfo* typeinfo = GetTypeInfo(type);
    if (!typeinfo) return {};

    auto target = typeinfo->attrs.find(attr_type);
    if (target == typeinfo->attrs.end()) return {};

    return *target;
}

SharedObject neko_refl::GetFieldAttr(Type type, Name field_name, Type attr_type) const {
    for (const auto& [typeinfo, baseobj] : ObjectTree{type}) {
        if (!typeinfo) continue;

        auto ftarget = typeinfo->fieldinfos.find(field_name);
        if (ftarget == typeinfo->fieldinfos.end()) continue;

        const auto& finfo = ftarget->second;

        auto target = finfo.attrs.find(attr_type);
        if (target == finfo.attrs.end()) return {};

        return *target;
    }
    return {};
}

SharedObject neko_refl::GetMethodAttr(Type type, Name method_name, Type attr_type) const {
    for (const auto& [typeinfo, baseobj] : ObjectTree{type}) {
        if (!typeinfo) continue;

        auto mtarget = typeinfo->methodinfos.find(method_name);
        if (mtarget == typeinfo->methodinfos.end()) continue;

        const auto& minfo = mtarget->second;

        auto target = minfo.attrs.find(attr_type);
        if (target == minfo.attrs.end()) return {};

        return *target;
    }
    return {};
}

void neko_refl::SetTemporaryResource(std::shared_ptr<std::pmr::memory_resource> rsrc) {
    neko_assert(rsrc.get());
    temporary_resource = std::move(rsrc);
}

void neko_refl::SetObjectResource(std::shared_ptr<std::pmr::memory_resource> rsrc) {
    neko_assert(rsrc.get());
    object_resource = std::move(rsrc);
}

void neko_refl::Clear() noexcept {
    // field attrs
    for (auto& [type, typeinfo] : typeinfos) {
        for (auto& [field, fieldinfo] : typeinfo.fieldinfos) fieldinfo.attrs.clear();
    }

    // type attrs
    for (auto& [ID, typeinfo] : typeinfos) typeinfo.attrs.clear();

    // type dynamic field
    for (auto& [type, typeinfo] : typeinfos) {
        auto iter = typeinfo.fieldinfos.begin();
        while (iter != typeinfo.fieldinfos.end()) {
            auto cur = iter;
            ++iter;

            if (cur->second.fieldptr.GetFieldFlag() == FieldFlag::DynamicShared) typeinfo.fieldinfos.erase(cur);
        }
    }

    typeinfos.clear();
}

neko_refl::~neko_refl() { Clear(); }

bool neko_refl::ContainsVirtualBase(Type type) const {
    auto* info = GetTypeInfo(type);
    if (!info) return false;

    for (const auto& [base, baseinfo] : info->baseinfos) {
        if (baseinfo.IsVirtual() || ContainsVirtualBase(base)) return true;
    }

    return false;
}

Type neko_refl::RegisterType(Type type, size_t size, size_t alignment, bool is_polymorphic, bool is_trivial) {
    neko_assert(alignment > 0 && (alignment & (alignment - 1)) == 0);
    auto target = typeinfos.find(type.RemoveCVRef());
    if (target != typeinfos.end()) return {};
    Type new_type = {tregistry.Register(type.get_id(), type.get_name()), type.get_id()};
    typeinfos.emplace_hint(target, new_type, TypeInfo{size, alignment, is_polymorphic, is_trivial});
    if (is_trivial) AddTrivialCopyConstructor(type);
    return new_type;
}

Type neko_refl::RegisterType(Type type, std::span<const Type> bases, std::span<const Type> field_types, std::span<const Name> field_names, bool is_trivial) {
    neko_assert(field_types.size() == field_names.size());

    if (typeinfos.contains(type)) return {};

    std::size_t size = 0;
    std::size_t alignment = 1;

    const size_t num_field = field_types.size();

    std::pmr::vector<std::size_t> base_offsets(temporary_resource.get());
    base_offsets.resize(bases.size());

    for (size_t i = 0; i < bases.size(); i++) {
        const auto& basetype = bases[i];
        auto btarget = typeinfos.find(basetype);
        if (btarget == typeinfos.end()) return {};
        const auto& baseinfo = btarget->second;
        if (baseinfo.is_polymorphic || ContainsVirtualBase(basetype)) return {};
        if (!baseinfo.is_trivial) is_trivial = false;
        neko_assert(baseinfo.alignment > 0 && (baseinfo.alignment & (baseinfo.alignment - 1)) == 0);
        size = (size + (baseinfo.alignment - 1)) & ~(baseinfo.alignment - 1);
        base_offsets[i] = size;
        size += baseinfo.size;
        if (baseinfo.alignment > alignment) alignment = baseinfo.alignment;
    }

    std::pmr::vector<std::size_t> field_offsets(temporary_resource.get());
    field_offsets.resize(num_field);

    for (size_t i = 0; i < num_field; ++i) {
        const auto& field_type = field_types[i];
        auto fttarget = typeinfos.find(field_type);
        if (fttarget == typeinfos.end()) return {};
        const auto& ftinfo = fttarget->second;
        if (!ftinfo.is_trivial) is_trivial = false;
        neko_assert(ftinfo.alignment > 0 && (ftinfo.alignment & (ftinfo.alignment - 1)) == 0);
        size = (size + (ftinfo.alignment - 1)) & ~(ftinfo.alignment - 1);
        field_offsets[i] = size;
        size += ftinfo.size;
        if (ftinfo.alignment > alignment) alignment = ftinfo.alignment;
    }

    size = (size + (alignment - 1)) & ~(alignment - 1);

    Type newtype = RegisterType(type, size, alignment, false, is_trivial);
    for (size_t i = 0; i < bases.size(); i++) {
        AddBase(type, bases[i],
                BaseInfo{{[offset = base_offsets[i]](void* derived) { return forward_offset(derived, offset); }, [offset = base_offsets[i]](void* base) { return backward_offset(base, offset); }}});
    }

    for (size_t i = 0; i < num_field; ++i) {
        AddField(type, field_names[i], FieldInfo{{field_types[i], field_offsets[i]}});
    }

    return newtype;
}

Name neko_refl::AddField(Type type, Name field_name, FieldInfo fieldinfo) {
    auto* typeinfo = GetTypeInfo(type);
    if (!typeinfo) {
        neko_assert(false);
        return {};
    }
    auto ftarget = typeinfo->fieldinfos.find(field_name);
    if (ftarget != typeinfo->fieldinfos.end()) return {};

    Name new_field_name = {nregistry.Register(field_name.get_id(), field_name.get_view()), field_name.get_id()};
    typeinfo->fieldinfos.emplace_hint(ftarget, new_field_name, std::move(fieldinfo));

    return new_field_name;
}

Name neko_refl::AddMethod(Type type, Name method_name, MethodInfo methodinfo) {
    auto* typeinfo = GetTypeInfo(type);
    if (!typeinfo) {
        neko_assert(false);
        return {};
    }

    auto [begin_iter, end_iter] = typeinfo->methodinfos.equal_range(method_name);
    for (auto iter = begin_iter; iter != end_iter; ++iter) {
        if (!iter->second.methodptr.IsDistinguishableWith(methodinfo.methodptr)) return {};
    }
    Name new_method_name = {nregistry.Register(method_name.get_id(), method_name.get_view()), method_name.get_id()};
    typeinfo->methodinfos.emplace(new_method_name, std::move(methodinfo));
    return new_method_name;
}

Name neko_refl::AddTrivialDefaultConstructor(Type type) {
    auto target = typeinfos.find(type);
    if (target == typeinfos.end()) {
        neko_assert(false);
        return {};
    }
    if (target->second.is_polymorphic || ContainsVirtualBase(type)) return {};
    for (const auto& [basetype, baseinfo] : target->second.baseinfos) {
        neko_assert(!baseinfo.IsPolymorphic());  // type isn't polymorphic => bases aren't polymorphic
        if (baseinfo.IsVirtual()) return {};
    }
    return AddMethod(type, NameIDRegistry::Meta::ctor, MethodInfo{{[](void* obj, void*, ArgsView) {}, MethodFlag::Variable}});
}

Name neko_refl::AddTrivialCopyConstructor(Type type) {
    auto target = typeinfos.find(type);
    if (target == typeinfos.end()) return {};
    auto& typeinfo = target->second;
    return AddMethod(type, NameIDRegistry::Meta::ctor,
                     MethodInfo{{
                             [size = typeinfo.size](void* obj, void*, ArgsView args) { memcpy(obj, args[0].GetPtr(), size); },
                             MethodFlag::Variable,
                             {},                                                // result type
                             {tregistry.RegisterAddConstLValueReference(type)}  // paramlist
                     }});
}

Name neko_refl::AddZeroDefaultConstructor(Type type) {
    auto target = typeinfos.find(type);
    if (target == typeinfos.end() || target->second.is_polymorphic || ContainsVirtualBase(type)) return {};
    for (const auto& [basetype, baseinfo] : target->second.baseinfos) {
        neko_assert(!baseinfo.IsPolymorphic());  // type isn't polymorphic => bases aren't polymorphic
        if (baseinfo.IsVirtual()) return {};
    }
    return AddMethod(type, NameIDRegistry::Meta::ctor, MethodInfo{{[size = target->second.size](void* obj, void*, ArgsView) { std::memset(obj, 0, size); }, MethodFlag::Variable}});
}

Name neko_refl::AddDefaultConstructor(Type type) {
    if (IsConstructible(type)) return {};

    auto target = typeinfos.find(type);
    if (target == typeinfos.end() || target->second.is_polymorphic || ContainsVirtualBase(type)) return {};
    const auto& typeinfo = target->second;
    for (const auto& [basetype, baseinfo] : typeinfo.baseinfos) {
        neko_assert(!baseinfo.IsPolymorphic() && !baseinfo.IsVirtual());  // type isn't polymorphic => bases aren't polymorphic
        if (!IsConstructible(basetype)) return {};
    }
    for (const auto& [fieldname, fieldinfo] : typeinfo.fieldinfos) {
        if (fieldinfo.fieldptr.GetFieldFlag() == FieldFlag::Unowned) continue;

        if (!IsConstructible(fieldinfo.fieldptr.GetType())) return {};
    }
    const auto& t = target->first;
    return AddMethod(type, NameIDRegistry::Meta::ctor,
                     MethodInfo{{[t](void* obj, void*, ArgsView) {
                                     const auto& typeinfo = neko_refl_instance.typeinfos.at(t);
                                     for (const auto& [basetype, baseinfo] : typeinfo.baseinfos) {
                                         void* baseptr = baseinfo.StaticCast_DerivedToBase(obj);
                                         bool success = neko_refl_instance.Construct(ObjectView{basetype, baseptr});
                                         neko_assert(success);
                                     }

                                     for (const auto& [fieldname, fieldinfo] : typeinfo.fieldinfos) {
                                         if (fieldinfo.fieldptr.GetFieldFlag() == FieldFlag::Unowned) continue;

                                         if (fieldinfo.fieldptr.GetType().IsPointer())
                                             buffer_as<void*>(fieldinfo.fieldptr.Var(obj).GetPtr()) = nullptr;
                                         else
                                             neko_refl_instance.Construct(fieldinfo.fieldptr.Var(obj));
                                     }
                                 },
                                 MethodFlag::Variable}});
}

Name neko_refl::AddDestructor(Type type) {
    if (IsDestructible(type)) return {};

    auto target = typeinfos.find(type);
    if (target == typeinfos.end() || target->second.is_polymorphic || ContainsVirtualBase(type)) return {};
    const auto& typeinfo = target->second;
    for (const auto& [basetype, baseinfo] : typeinfo.baseinfos) {
        neko_assert(!baseinfo.IsPolymorphic() && !baseinfo.IsVirtual());  // type isn't polymorphic => bases aren't polymorphic
        if (!IsDestructible(basetype)) return {};
    }
    for (const auto& [fieldname, fieldinfo] : typeinfo.fieldinfos) {
        if (fieldinfo.fieldptr.GetFieldFlag() == FieldFlag::Unowned) continue;

        if (!type.IsReference() && !IsDestructible(fieldinfo.fieldptr.GetType())) return {};
    }
    const auto& t = target->first;
    return AddMethod(type, NameIDRegistry::Meta::dtor,
                     MethodInfo{{[t](void* obj, void*, ArgsView) {
                                     const auto& typeinfo = neko_refl_instance.typeinfos.at(t);

                                     for (const auto& [fieldname, fieldinfo] : typeinfo.fieldinfos) {
                                         if (fieldinfo.fieldptr.GetFieldFlag() == FieldFlag::Unowned) continue;
                                         Type ftype = fieldinfo.fieldptr.GetType();
                                         if (ftype.IsReference()) continue;
                                         neko_refl_instance.Destruct(fieldinfo.fieldptr.Var(obj));
                                     }

                                     for (const auto& [basetype, baseinfo] : typeinfo.baseinfos) {
                                         void* baseptr = baseinfo.StaticCast_DerivedToBase(obj);
                                         neko_refl_instance.Destruct(ObjectView{basetype, baseptr});
                                     }
                                 },
                                 MethodFlag::Variable}});
}

Type neko_refl::AddBase(Type derived, Type base, BaseInfo baseinfo) {
    auto* typeinfo = GetTypeInfo(derived);
    if (!typeinfo) return {};
    auto btarget = typeinfo->baseinfos.find(base.RemoveCVRef());
    if (btarget != typeinfo->baseinfos.end()) return {};
    Type new_base_type = {tregistry.Register(base.get_id(), base.get_name()), base.get_id()};
    typeinfo->baseinfos.emplace_hint(btarget, new_base_type, std::move(baseinfo));
    return new_base_type;
}

bool neko_refl::AddTypeAttr(Type type, Attr attr) {
    auto* typeinfo = GetTypeInfo(type);
    if (!typeinfo) return false;
    auto& attrs = typeinfo->attrs;
    auto atarget = attrs.find(attr);
    if (atarget != attrs.end()) return false;
    attrs.emplace_hint(atarget, std::move(attr));
    return true;
}

bool neko_refl::AddFieldAttr(Type type, Name name, Attr attr) {
    auto* typeinfo = GetTypeInfo(type);
    if (!typeinfo) return false;
    auto ftarget = typeinfo->fieldinfos.find(name);
    if (ftarget == typeinfo->fieldinfos.end()) return false;
    auto& attrs = ftarget->second.attrs;
    auto atarget = attrs.find(attr);
    if (atarget != attrs.end()) return false;
    attrs.emplace_hint(atarget, std::move(attr));
    return true;
}

bool neko_refl::AddMethodAttr(Type type, Name name, Attr attr) {
    auto* typeinfo = GetTypeInfo(type);
    if (!typeinfo) return false;
    auto mtarget = typeinfo->methodinfos.find(name);
    if (mtarget == typeinfo->methodinfos.end()) return false;
    auto& attrs = mtarget->second.attrs;
    auto atarget = attrs.find(attr);
    if (atarget != attrs.end()) return false;
    attrs.emplace_hint(atarget, std::move(attr));
    return true;
}

SharedObject neko_refl::MMakeShared(Type type, std::pmr::memory_resource* rsrc, ArgsView args) const {
    if (!IsDestructible(type)) return {};

    ObjectView obj = MNew(type, rsrc, args);

    if (!obj.GetType().valid()) return {};

    return {obj, [rsrc, type](void* ptr) { neko_refl_instance.MDelete({type, ptr}, rsrc); }};
}

ObjectView neko_refl::StaticCast_DerivedToBase(ObjectView obj, Type type) const {
    constexpr auto cast = [](ObjectView obj, Type type) -> ObjectView {
        for (const auto& [typeinfo, base_obj] : ObjectTree{obj}) {
            if (base_obj.GetType() == type) return base_obj;
        }
        return {};
    };

    const CVRefMode cvref_mode = obj.GetType().GetCVRefMode();
    neko_assert(!CVRefMode_IsVolatile(cvref_mode));
    switch (cvref_mode) {
        case CVRefMode::Left:
            return cast(obj.RemoveLValueReference(), type).AddLValueReference();
        case CVRefMode::Right:
            return cast(obj.RemoveRValueReference(), type).AddRValueReference();
        case CVRefMode::Const:
            return cast(obj.RemoveConst(), type).AddConst();
        case CVRefMode::ConstLeft:
            return cast(obj.RemoveConstReference(), type).AddConstLValueReference();
        case CVRefMode::ConstRight:
            return cast(obj.RemoveConstReference(), type).AddConstRValueReference();
        default:
            return cast(obj, type);
    }
}

ObjectView neko_refl::StaticCast_BaseToDerived(ObjectView obj, Type type) const {
    if (obj.GetPtr() == nullptr) return {type, nullptr};

    const CVRefMode cvref_mode = obj.GetType().GetCVRefMode();
    neko_assert(!CVRefMode_IsVolatile(cvref_mode));
    switch (cvref_mode) {
        case CVRefMode::Left:
            return details::StaticCast_BaseToDerived(obj.RemoveLValueReference(), type).AddLValueReference();
        case CVRefMode::Right:
            return details::StaticCast_BaseToDerived(obj.RemoveRValueReference(), type).AddRValueReference();
        case CVRefMode::Const:
            return details::StaticCast_BaseToDerived(obj.RemoveConst(), type).AddConst();
        case CVRefMode::ConstLeft:
            return details::StaticCast_BaseToDerived(obj.RemoveConstReference(), type).AddConstLValueReference();
        case CVRefMode::ConstRight:
            return details::StaticCast_BaseToDerived(obj.RemoveConstReference(), type).AddConstRValueReference();
        default:
            return details::StaticCast_BaseToDerived(obj, type);
    }
}

ObjectView neko_refl::DynamicCast_BaseToDerived(ObjectView obj, Type type) const {
    if (obj.GetPtr() == nullptr) return {type, nullptr};

    const CVRefMode cvref_mode = obj.GetType().GetCVRefMode();
    neko_assert(!CVRefMode_IsVolatile(cvref_mode));
    switch (cvref_mode) {
        case CVRefMode::Left:
            return details::DynamicCast_BaseToDerived(obj.RemoveLValueReference(), type).AddLValueReference();
        case CVRefMode::Right:
            return details::DynamicCast_BaseToDerived(obj.RemoveRValueReference(), type).AddRValueReference();
        case CVRefMode::Const:
            return details::DynamicCast_BaseToDerived(obj.RemoveConst(), type).AddConst();
        case CVRefMode::ConstLeft:
            return details::DynamicCast_BaseToDerived(obj.RemoveConstReference(), type).AddConstLValueReference();
        case CVRefMode::ConstRight:
            return details::DynamicCast_BaseToDerived(obj.RemoveConstReference(), type).AddConstRValueReference();
        default:
            return details::DynamicCast_BaseToDerived(obj, type);
    }
}

ObjectView neko_refl::StaticCast(ObjectView obj, Type type) const {
    auto ptr_d2b = StaticCast_DerivedToBase(obj, type);
    if (ptr_d2b.GetType()) return ptr_d2b;

    auto ptr_b2d = StaticCast_BaseToDerived(obj, type);
    if (ptr_b2d.GetType()) return ptr_b2d;

    return {};
}

ObjectView neko_refl::DynamicCast(ObjectView obj, Type type) const {
    auto ptr_b2d = DynamicCast_BaseToDerived(obj, type);
    if (ptr_b2d.GetType()) return ptr_b2d;

    auto ptr_d2b = StaticCast_DerivedToBase(obj, type);
    if (ptr_d2b.GetType()) return ptr_d2b;

    return {};
}

ObjectView neko_refl::Var(ObjectView obj, Name field_name, FieldFlag flag) const {
    for (const auto& [name, var] : VarRange{obj, flag}) {
        if (name == field_name) return var;
    }
    return {};
}

ObjectView neko_refl::Var(ObjectView obj, Type base, Name field_name, FieldFlag flag) const {
    auto base_obj = StaticCast_DerivedToBase(obj, base);
    if (!base_obj.GetType()) return {};
    return Var(base_obj, field_name);
}

bool neko_refl::IsCompatible(std::span<const Type> paramTypes, std::span<const Type> argTypes) const {
    if (paramTypes.size() != argTypes.size()) return false;

    for (size_t i = 0; i < paramTypes.size(); i++) {
        if (paramTypes[i] == argTypes[i] || paramTypes[i].Is<ObjectView>()) continue;

        const auto& lhs = paramTypes[i];
        const auto& rhs = argTypes[i];

        if (lhs.IsLValueReference()) {                                        // &{T} | &{const{T}}
            const auto unref_lhs = lhs.Name_RemoveLValueReference();          // T | const{T}
            if (type_name_is_const(unref_lhs)) {                              // &{const{T}}
                if (unref_lhs == rhs.Name_RemoveRValueReference()) continue;  // &{const{T}} <- &&{const{T}} / const{T}

                const auto raw_lhs = type_name_remove_const(unref_lhs);  // T

                if (rhs.Is(raw_lhs) || raw_lhs == rhs.Name_RemoveReference()) continue;  // &{const{T}} <- T | &{T} | &&{T}

                if (details::IsRefConstructible(raw_lhs, std::span<const Type>{&rhs, 1}) && IsDestructible(raw_lhs)) continue;  // &{const{T}} <- T{arg}
            }
        } else if (lhs.IsRValueReference()) {                    // &&{T} | &&{const{T}}
            const auto unref_lhs = lhs.RemoveRValueReference();  // T | const{T}

            if (type_name_is_const(unref_lhs)) {                         // &&{const{T}}
                const auto raw_lhs = type_name_remove_const(unref_lhs);  // T

                if (raw_lhs == type_name_remove_const(rhs)) continue;  // &&{const{T}} <- T / const{T}

                if (raw_lhs == rhs.Name_RemoveRValueReference())  // &&{const{T}}
                    continue;                                     // &&{const{T}} <- &&{T}

                if (details::IsRefConstructible(raw_lhs, std::span<const Type>{&rhs, 1})) continue;  // &&{const{T}} <- T{arg}
            } else {
                if (rhs.Is(unref_lhs)) continue;  // &&{T} <- T

                if (details::IsRefConstructible(unref_lhs, std::span<const Type>{&rhs, 1}) && IsDestructible(unref_lhs)) continue;  // &&{T} <- T{arg}
            }
        } else {                                                     // T
            if (lhs.Is(rhs.Name_RemoveRValueReference())) continue;  // T <- &&{T}

            if (details::IsRefConstructible(lhs, std::span<const Type>{&rhs, 1}) && IsDestructible(lhs)) continue;  // T <- T{arg}
        }

        if (is_pointer_array_compatible(lhs, rhs)) continue;

        return false;
    }

    return true;
}

Type neko_refl::IsInvocable(Type type, Name method_name, std::span<const Type> argTypes, MethodFlag flag) const {
    const CVRefMode cvref_mode = type.GetCVRefMode();
    neko_assert(!CVRefMode_IsVolatile(cvref_mode));
    switch (cvref_mode) {
        case CVRefMode::Left:
            [[fallthrough]];
        case CVRefMode::Right:
            type = type.RemoveReference();
            break;
        case CVRefMode::Const:
            [[fallthrough]];
        case CVRefMode::ConstLeft:
            [[fallthrough]];
        case CVRefMode::ConstRight:
            type = type.RemoveCVRef();
            flag = enum_remove(flag, MethodFlag::Variable);
            break;
        default:
            break;
    }

    auto is_invocable = [&](bool is_priority, MethodFlag filter) -> Type {
        if (!enum_contain_any(flag, filter)) return {};

        MethodFlag newflag = enum_within(flag, filter);

        for (const auto& [typeinfo, baseobj] : ObjectTree{type}) {
            if (!typeinfo) continue;

            auto [begin_iter, end_iter] = typeinfo->methodinfos.equal_range(method_name);
            for (auto iter = begin_iter; iter != end_iter; ++iter) {
                if (enum_contain_any(newflag, iter->second.methodptr.GetMethodFlag()) &&
                    (is_priority ? details::IsPriorityCompatible(iter->second.methodptr.GetParamList(), argTypes) : neko_refl_instance.IsCompatible(iter->second.methodptr.GetParamList(), argTypes))) {
                    return iter->second.methodptr.GetResultType();
                }
            }
        }

        return {};
    };

    if (auto rst = is_invocable(true, MethodFlag::Priority)) return rst;
    if (auto rst = is_invocable(true, MethodFlag::Const)) return rst;
    if (auto rst = is_invocable(false, MethodFlag::Priority)) return rst;
    if (auto rst = is_invocable(false, MethodFlag::Const)) return rst;

    return {};
}

Type neko_refl::BInvoke(ObjectView obj, Name method_name, void* result_buffer, ArgsView args, MethodFlag flag, std::pmr::memory_resource* temp_args_rsrc) const {
    neko_assert(temp_args_rsrc);

    const CVRefMode cvref_mode = obj.GetType().GetCVRefMode();
    neko_assert(!CVRefMode_IsVolatile(cvref_mode));
    switch (cvref_mode) {
        case CVRefMode::Left:
            [[fallthrough]];
        case CVRefMode::Right:
            obj = obj.RemoveReference();
            break;
        case CVRefMode::Const:
            obj = obj.RemoveConst();
            flag = enum_remove(flag, MethodFlag::Variable);
            break;
        case CVRefMode::ConstLeft:
            [[fallthrough]];
        case CVRefMode::ConstRight:
            obj = obj.RemoveConstReference();
            flag = enum_remove(flag, MethodFlag::Variable);
            break;
        default:
            break;
    }

    if (!obj.GetPtr()) flag = enum_within(flag, MethodFlag::Static);

    auto binvoke = [&](bool is_priority, MethodFlag filter) -> Type {
        if (!enum_contain_any(flag, filter)) return {};

        MethodFlag newflag = enum_within(flag, filter);

        for (const auto& [typeinfo, baseobj] : ObjectTree{obj}) {
            if (!typeinfo) continue;

            auto [begin_iter, end_iter] = typeinfo->methodinfos.equal_range(method_name);

            for (auto iter = begin_iter; iter != end_iter; ++iter) {
                if (!enum_contain_any(newflag, iter->second.methodptr.GetMethodFlag())) continue;

                details::NewArgsGuard guard{is_priority, temp_args_rsrc, iter->second.methodptr.GetParamList(), args};
                if (!guard.IsCompatible()) continue;
                iter->second.methodptr.Invoke(baseobj.GetPtr(), result_buffer, guard.GetArgsView());
                return iter->second.methodptr.GetResultType();
            }
        }
        return {};
    };

    if (auto rst = binvoke(true, MethodFlag::Priority)) return rst;
    if (auto rst = binvoke(true, MethodFlag::Const)) return rst;
    if (auto rst = binvoke(false, MethodFlag::Priority)) return rst;
    if (auto rst = binvoke(false, MethodFlag::Const)) return rst;

    return {};
}

SharedObject neko_refl::MInvoke(ObjectView obj, Name method_name, std::pmr::memory_resource* rst_rsrc, ArgsView args, MethodFlag flag, std::pmr::memory_resource* temp_args_rsrc) const {
    neko_assert(rst_rsrc);
    neko_assert(temp_args_rsrc);

    const CVRefMode cvref_mode = obj.GetType().GetCVRefMode();
    neko_assert(!CVRefMode_IsVolatile(cvref_mode));
    switch (cvref_mode) {
        case CVRefMode::Left:
            [[fallthrough]];
        case CVRefMode::Right:
            obj = obj.RemoveReference();
            break;
        case CVRefMode::Const:
            obj = obj.RemoveConst();
            flag = enum_remove(flag, MethodFlag::Variable);
            break;
        case CVRefMode::ConstLeft:
            [[fallthrough]];
        case CVRefMode::ConstRight:
            obj = obj.RemoveConstReference();
            flag = enum_remove(flag, MethodFlag::Variable);
            break;
        default:
            break;
    }

    if (!obj.GetPtr()) flag = enum_within(flag, MethodFlag::Static);

    auto minvoke = [&](bool is_priority, MethodFlag filter) -> SharedObject {
        if (!enum_contain_any(flag, filter)) return {};

        MethodFlag newflag = enum_within(flag, filter);

        for (const auto& [typeinfo, baseobj] : ObjectTree{obj}) {
            if (!typeinfo) continue;

            auto [begin_iter, end_iter] = typeinfo->methodinfos.equal_range(method_name);

            for (auto iter = begin_iter; iter != end_iter; ++iter) {
                if (!enum_contain_any(newflag, iter->second.methodptr.GetMethodFlag())) continue;

                details::NewArgsGuard guard{is_priority, temp_args_rsrc, iter->second.methodptr.GetParamList(), args};

                if (!guard.IsCompatible()) continue;

                const auto& methodptr = iter->second.methodptr;
                const auto& rst_type = methodptr.GetResultType();

                if (rst_type.Is<void>()) {
                    iter->second.methodptr.Invoke(baseobj.GetPtr(), nullptr, guard.GetArgsView());
                    return SharedObject{Type_of<void>};
                } else if (rst_type.IsReference()) {
                    std::aligned_storage_t<sizeof(void*)> buffer;
                    iter->second.methodptr.Invoke(baseobj.GetPtr(), &buffer, guard.GetArgsView());
                    return {rst_type, buffer_as<void*>(&buffer)};
                } else if (rst_type.Is<ObjectView>()) {
                    std::aligned_storage_t<sizeof(ObjectView)> buffer;
                    iter->second.methodptr.Invoke(baseobj.GetPtr(), &buffer, guard.GetArgsView());
                    return SharedObject{buffer_as<ObjectView>(&buffer)};
                } else if (rst_type.Is<SharedObject>()) {
                    SharedObject buffer;
                    iter->second.methodptr.Invoke(baseobj.GetPtr(), &buffer, guard.GetArgsView());
                    return buffer;
                } else {
                    if (!neko_refl_instance.IsDestructible(rst_type)) return {};
                    auto* result_typeinfo = neko_refl_instance.GetTypeInfo(rst_type);
                    if (!result_typeinfo) return {};
                    void* result_buffer = rst_rsrc->allocate(result_typeinfo->size, result_typeinfo->alignment);
                    iter->second.methodptr.Invoke(baseobj.GetPtr(), result_buffer, guard.GetArgsView());
                    return {{rst_type, result_buffer}, [rst_type, rst_rsrc](void* ptr) { neko_refl_instance.MDelete({rst_type, ptr}, rst_rsrc); }};
                }
            }
        }
        return {};
    };

    if (auto rst = minvoke(true, MethodFlag::Priority); rst.GetType()) return rst;
    if (auto rst = minvoke(true, MethodFlag::Const); rst.GetType()) return rst;
    if (auto rst = minvoke(false, MethodFlag::Priority); rst.GetType()) return rst;
    if (auto rst = minvoke(false, MethodFlag::Const); rst.GetType()) return rst;

    return {};
}

ObjectView neko_refl::MNew(Type type, std::pmr::memory_resource* rsrc, ArgsView args) const {
    neko_assert(rsrc);

    if (!IsConstructible(type, args.Types())) return {};

    const auto& typeinfo = typeinfos.at(type);

    void* buffer = rsrc->allocate(std::max<std::size_t>(1, typeinfo.size), typeinfo.alignment);

    if (!buffer) return {};

    ObjectView obj{type, buffer};
    bool success = Construct(obj, args);
    neko_assert(success);

    return obj;
}

bool neko_refl::MDelete(ObjectView obj, std::pmr::memory_resource* rsrc) const {
    neko_assert(rsrc);

    Destruct(obj);

    const auto& typeinfo = typeinfos.at(obj.GetType());

    rsrc->deallocate(obj.GetPtr(), std::max<std::size_t>(1, typeinfo.size), typeinfo.alignment);

    return true;
}

ObjectView neko_refl::New(Type type, ArgsView args) const { return MNew(type, object_resource.get(), args); }

bool neko_refl::Delete(ObjectView obj) const { return MDelete(obj, object_resource.get()); }

SharedObject neko_refl::MakeShared(Type type, ArgsView args) const { return MMakeShared(type, object_resource.get(), args); }

bool neko_refl::IsConstructible(Type type, std::span<const Type> argTypes) const {
    auto target = typeinfos.find(type);
    if (target == typeinfos.end()) return false;
    const auto& typeinfo = target->second;

    if (typeinfo.is_trivial && (argTypes.empty()                                                   // default ctor
                                || argTypes.size() == 1 && argTypes.front().RemoveCVRef() == type  // const/ref ctor
                                )) {
        return true;
    }

    auto [begin_iter, end_iter] = typeinfo.methodinfos.equal_range(NameIDRegistry::Meta::ctor);
    for (auto iter = begin_iter; iter != end_iter; ++iter) {
        if (IsCompatible(iter->second.methodptr.GetParamList(), argTypes)) return true;
    }
    return false;
}

bool neko_refl::IsCopyConstructible(Type type) const {
    const Type clref_type = tregistry.RegisterAddConstLValueReference(type);
    return details::IsRefConstructible(type, std::span<const Type>{&clref_type, 1});
}

bool neko_refl::IsMoveConstructible(Type type) const {
    const Type rref_type = tregistry.RegisterAddRValueReference(type);
    return details::IsRefConstructible(type, std::span<const Type>{&rref_type, 1});
}

bool neko_refl::IsDestructible(Type type) const {
    neko_assert(type.GetCVRefMode() == CVRefMode::None);

    auto target = typeinfos.find(type);
    if (target == typeinfos.end()) return false;
    const auto& typeinfo = target->second;
    if (typeinfo.is_trivial) return true;
    auto [begin_iter, end_iter] = typeinfo.methodinfos.equal_range(NameIDRegistry::Meta::dtor);
    if (begin_iter == end_iter) return true;
    for (auto iter = begin_iter; iter != end_iter; ++iter) {
        if (iter->second.methodptr.GetMethodFlag() == MethodFlag::Variable && IsCompatible(iter->second.methodptr.GetParamList(), {})) return true;
    }
    return false;
}

bool neko_refl::Construct(ObjectView obj, ArgsView args) const {
    auto target = typeinfos.find(obj.GetType());
    if (target == typeinfos.end()) return false;
    const auto& typeinfo = target->second;
    if (args.Types().empty() && typeinfo.is_trivial) return true;  // trivial ctor
    auto [begin_iter, end_iter] = typeinfo.methodinfos.equal_range(NameIDRegistry::Meta::ctor);
    for (auto iter = begin_iter; iter != end_iter; ++iter) {
        if (iter->second.methodptr.GetMethodFlag() == MethodFlag::Variable) {
            details::NewArgsGuard guard{false, temporary_resource.get(), iter->second.methodptr.GetParamList(), args};
            if (!guard.IsCompatible()) continue;
            iter->second.methodptr.Invoke(obj.GetPtr(), nullptr, guard.GetArgsView());
            return true;
        }
    }
    return false;
}

bool neko_refl::Destruct(ObjectView obj) const {
    auto target = typeinfos.find(obj.GetType());
    if (target == typeinfos.end()) return false;
    const auto& typeinfo = target->second;
    if (typeinfo.is_trivial) return true;  // trivial ctor
    auto [begin_iter, end_iter] = typeinfo.methodinfos.equal_range(NameIDRegistry::Meta::dtor);
    for (auto iter = begin_iter; iter != end_iter; ++iter) {
        if (iter->second.methodptr.GetMethodFlag() == MethodFlag::Variable && IsCompatible(iter->second.methodptr.GetParamList(), {})) {
            iter->second.methodptr.Invoke(obj.GetPtr(), nullptr, {});
            return true;
        }
    }
    return false;
}

FieldFlag FieldPtr::GetFieldFlag() const noexcept {
    switch (data.index()) {
        case 0:
            return FieldFlag::Basic;
        case 1:
            return FieldFlag::Virtual;
        case 2:
            return FieldFlag::Static;
        case 3:
            return FieldFlag::DynamicShared;
        case 4:
            return FieldFlag::DynamicBuffer;
        default:
            return FieldFlag::None;
    }
}

ObjectView FieldPtr::Var() {
    return std::visit(
            [this]<typename T>(T& value) -> ObjectView {
                if constexpr (std::is_same_v<T, size_t>) {
                    neko_assert(false);
                    return {};
                } else if constexpr (std::is_same_v<T, Offsetor>) {
                    neko_assert(false);
                    return {};
                } else if constexpr (std::is_same_v<T, void*>) {
                    return {type, value};
                } else if constexpr (std::is_same_v<T, SharedBuffer>) {
                    return {type, value.get()};
                } else if constexpr (std::is_same_v<T, Buffer>) {
                    return {type, &value};
                } else
                    static_assert(always_false<T>);
            },
            data);
}

ObjectView FieldPtr::Var(void* obj) {
    return std::visit(
            [obj, this]<typename T>(T& value) -> ObjectView {
                if constexpr (std::is_same_v<T, size_t>) {
                    neko_assert(obj);
                    return {type, forward_offset(obj, value)};
                } else if constexpr (std::is_same_v<T, Offsetor>) {
                    neko_assert(obj);
                    return {type, value(obj)};
                } else if constexpr (std::is_same_v<T, void*>) {
                    return {type, value};
                } else if constexpr (std::is_same_v<T, SharedBuffer>) {
                    return {type, value.get()};
                } else if constexpr (std::is_same_v<T, Buffer>) {
                    return {type, &value};
                } else
                    static_assert(always_false<T>);
            },
            data);
}

ObjectView FieldPtr::Var() const {
    return std::visit(
            [this]<typename T>(const T& value) -> ObjectView {
                if constexpr (std::is_same_v<T, size_t>) {
                    neko_assert(false);
                    return {};
                } else if constexpr (std::is_same_v<T, Offsetor>) {
                    neko_assert(false);
                    return {};
                } else if constexpr (std::is_same_v<T, void*>) {
                    return {type, value};
                } else if constexpr (std::is_same_v<T, SharedBuffer>) {
                    return {type, value.get()};
                } else if constexpr (std::is_same_v<T, Buffer>) {
                    neko_assert(false);
                    return {};
                } else
                    static_assert(always_false<T>);
            },
            data);
}

ObjectView FieldPtr::Var(void* obj) const {
    return std::visit(
            [obj, this]<typename T>(const T& value) -> ObjectView {
                if constexpr (std::is_same_v<T, size_t>) {
                    neko_assert(obj);
                    return {type, forward_offset(obj, value)};
                } else if constexpr (std::is_same_v<T, Offsetor>) {
                    neko_assert(obj);
                    return {type, value(obj)};
                } else if constexpr (std::is_same_v<T, void*>) {
                    return {type, value};
                } else if constexpr (std::is_same_v<T, SharedBuffer>) {
                    return {type, value.get()};
                } else if constexpr (std::is_same_v<T, Buffer>) {
                    neko_assert(false);
                    return {};
                } else
                    static_assert(always_false<T>);
            },
            data);
}

#define NEKO_DYREFL_META_GET_ID_VIEW(meta) meta.get_id(), meta.get_view()
#define NEKO_DYREFL_META_GET_ID_NAME(meta) meta.get_id(), meta.get_name()

NameIDRegistry::NameIDRegistry() {
    // operators
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::operator_bool));

    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::operator_add));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::operator_sub));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::operator_mul));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::operator_div));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::operator_mod));

    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::operator_bnot));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::operator_band));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::operator_bor));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::operator_bxor));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::operator_shl));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::operator_shr));

    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::operator_pre_inc));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::operator_pre_dec));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::operator_post_inc));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::operator_post_dec));

    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::operator_assignment));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::operator_assignment_add));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::operator_assignment_sub));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::operator_assignment_mul));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::operator_assignment_div));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::operator_assignment_mod));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::operator_assignment_band));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::operator_assignment_bor));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::operator_assignment_bxor));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::operator_assignment_shl));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::operator_assignment_shr));

    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::operator_eq));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::operator_ne));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::operator_lt));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::operator_le));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::operator_gt));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::operator_ge));

    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::operator_and));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::operator_or));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::operator_not));

    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::operator_subscript));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::operator_indirection));

    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::operator_call));

    // non-member functions

    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::ctor));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::dtor));

    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::get));

    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::tuple_size));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::tuple_element));

    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::holds_alternative));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::get_if));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::variant_size));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::variant_alternative));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::variant_visit_get));

    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::advance));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::distance));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::next));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::prev));

    // member functions

    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::container_assign));

    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::container_begin));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::container_cbegin));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::container_end));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::container_cend));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::container_rbegin));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::container_crbegin));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::container_rend));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::container_crend));

    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::container_at));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::container_data));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::container_front));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::container_back));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::container_top));

    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::container_empty));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::container_size));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::container_size_bytes));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::container_resize));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::container_capacity));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::container_bucket_count));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::container_reserve));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::container_shrink_to_fit));

    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::container_clear));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::container_insert));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::container_insert_after));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::container_insert_or_assign));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::container_erase));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::container_erase_after));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::container_push_front));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::container_pop_front));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::container_push_back));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::container_pop_back));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::container_push));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::container_pop));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::container_swap));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::container_merge));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::container_extract));

    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::container_splice_after));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::container_splice));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::container_remove));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::container_reverse));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::container_unique));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::container_sort));

    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::container_count));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::container_find));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::container_lower_bound));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::container_upper_bound));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::container_equal_range));

    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::variant_index));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::variant_valueless_by_exception));

    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::optional_has_value));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::optional_value));
    RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_VIEW(Meta::optional_reset));
}

Name NameIDRegistry::Nameof(name_id ID) const {
    auto view = Viewof(ID);
    if (view.empty()) return {};
    return {view, ID};
}

TypeIDRegistry::TypeIDRegistry() { RegisterUnmanaged(NEKO_DYREFL_META_GET_ID_NAME(Meta::global)); }

Type TypeIDRegistry::Typeof(TypeID ID) const {
    auto view = Viewof(ID);
    if (view.empty()) return {};
    return {view, ID};
}

//
// Type Computation
/////////////////////

Type TypeIDRegistry::RegisterAddConst(Type type) {
    std::string_view name = type.get_name();
    if (name.empty()) return {};

    TypeID ref_ID{type_name_add_const_hash(name)};
    if (auto ref_name = Viewof(ref_ID); !ref_name.empty()) return {ref_name, ref_ID};

    std::string_view rst_name;
    {
        std::lock_guard wlock{smutex};  // write resource
        rst_name = type_name_add_const(name, get_allocator());
    }

    RegisterUnmanaged(ref_ID, rst_name);
    return {rst_name, ref_ID};
}

Type TypeIDRegistry::RegisterAddLValueReference(Type type) {
    std::string_view name = type.get_name();
    if (name.empty()) return {};

    TypeID ref_ID{type_name_add_lvalue_reference_hash(name)};
    if (auto ref_name = Viewof(ref_ID); !ref_name.empty()) return {ref_name, ref_ID};

    std::string_view rst_name;
    {
        std::lock_guard wlock{smutex};  // write resource
        rst_name = type_name_add_lvalue_reference(name, get_allocator());
    }

    RegisterUnmanaged(ref_ID, rst_name);
    return {rst_name, ref_ID};
}

Type TypeIDRegistry::RegisterAddLValueReferenceWeak(Type type) {
    std::string_view name = type.get_name();
    if (name.empty()) return {};

    TypeID ref_ID{type_name_add_lvalue_reference_weak_hash(name)};
    if (auto ref_name = Viewof(ref_ID); !ref_name.empty()) return {ref_name, ref_ID};

    std::string_view rst_name;
    {
        std::lock_guard wlock{smutex};  // write resource
        rst_name = type_name_add_lvalue_reference_weak(name, get_allocator());
    }

    RegisterUnmanaged(ref_ID, rst_name);

    return {rst_name, ref_ID};
}

Type TypeIDRegistry::RegisterAddRValueReference(Type type) {
    std::string_view name = type.get_name();
    if (name.empty()) return {};

    TypeID ref_ID{type_name_add_rvalue_reference_hash(name)};
    if (auto ref_name = Viewof(ref_ID); !ref_name.empty()) return {ref_name, ref_ID};

    std::string_view rst_name;
    {
        std::lock_guard wlock{smutex};  // write resource
        rst_name = type_name_add_rvalue_reference(name, get_allocator());
    }

    RegisterUnmanaged(ref_ID, rst_name);

    return {rst_name, ref_ID};
}

Type TypeIDRegistry::RegisterAddConstLValueReference(Type type) {
    std::string_view name = type.get_name();
    if (name.empty()) return {};

    TypeID ref_ID{type_name_add_const_lvalue_reference_hash(name)};
    if (auto ref_name = Viewof(ref_ID); !ref_name.empty()) return {ref_name, ref_ID};

    std::string_view rst_name;
    {
        std::lock_guard wlock{smutex};  // write resource
        rst_name = type_name_add_const_lvalue_reference(name, get_allocator());
    }

    RegisterUnmanaged(ref_ID, rst_name);

    return {rst_name, ref_ID};
}

Type TypeIDRegistry::RegisterAddConstRValueReference(Type type) {
    std::string_view name = type.get_name();
    if (name.empty()) return {};

    TypeID ref_ID{type_name_add_const_rvalue_reference_hash(name)};
    if (auto ref_name = Viewof(ref_ID); !ref_name.empty()) return {ref_name, ref_ID};

    std::string_view rst_name;
    {
        std::lock_guard wlock{smutex};  // write resource
        rst_name = type_name_add_const_rvalue_reference(name, get_allocator());
    }

    RegisterUnmanaged(ref_ID, rst_name);
    return {rst_name, ref_ID};
}

#undef NEKO_DYREFL_META_GET_ID_VIEW
#undef NEKO_DYREFL_META_GET_ID_NAME

MethodPtr::MethodPtr(Func func, MethodFlag flag, Type result_type, ParamList paramList) : func{std::move(func)}, flag{flag}, result_type{result_type}, paramList{std::move(paramList)} {
    neko_assert(enum_single(flag));
}

bool MethodPtr::IsMatch(std::span<const Type> argTypes) const noexcept {
    const std::size_t n = paramList.size();
    if (argTypes.size() != n) return false;

    for (std::size_t i = 0; i < n; i++) {
        if (paramList[i] == argTypes[i] || paramList[i].Is<ObjectView>()) continue;
    }

    return true;
}

void MethodPtr::Invoke(void* obj, void* result_buffer, ArgsView args) const {
    neko_assert(IsMatch(args.Types()));
    func(obj, result_buffer, args);
};

void MethodPtr::Invoke(const void* obj, void* result_buffer, ArgsView args) const {
    neko_assert(IsMatch(args.Types()));
    if (flag == MethodFlag::Variable) return;
    func(const_cast<void*>(obj), result_buffer, args);
};

void MethodPtr::Invoke(void* result_buffer, ArgsView args) const {
    neko_assert(IsMatch(args.Types()));
    if (flag != MethodFlag::Static) return;
    func(nullptr, result_buffer, args);
};

MethodRange::iterator::iterator(ObjectTree::iterator typeiter, MethodFlag flag) : typeiter{std::move(typeiter)}, flag{flag}, mode{typeiter.Valid() ? 0 : -1} {
    if (typeiter.Valid()) update();
}

void MethodRange::iterator::update() {
    switch (mode) {
        case 0:
            goto mode_0;
        case 1:
            goto mode_1;
        default:
            neko_assert(false);
            return;
    }

mode_0:
    mode = 1;
    while (typeiter.Valid()) {
        if (!std::get<TypeInfo*>(*typeiter)) {
            ++typeiter;
            continue;
        }

        curmethod = std::get<TypeInfo*>(*typeiter)->methodinfos.begin();
        while (curmethod != std::get<TypeInfo*>(*typeiter)->methodinfos.end()) {
            if (enum_contain_any(flag, curmethod->second.methodptr.GetMethodFlag())) {
                return;  // yield
            mode_1:;
            }
            ++curmethod;
        }

        ++typeiter;
    }

    mode = -1;
    return;  // stop
}

MethodRange::iterator& MethodRange::iterator::operator++() {
    update();
    return *this;
}

MethodRange::iterator MethodRange::iterator::operator++(int) {
    MethodRange::iterator iter = *this;
    (void)operator++();
    return iter;
}

namespace neko::cpp {
bool operator==(const MethodRange::iterator& lhs, const MethodRange::iterator& rhs) {
    neko_assert(lhs.flag == rhs.flag);
    if (lhs.Valid()) {
        if (rhs.Valid()) {
            if (lhs.typeiter == rhs.typeiter)
                return lhs.curmethod == rhs.curmethod;
            else
                return false;
        } else
            return false;
    } else if (rhs.Valid())
        return false;
    else
        return lhs.typeiter == rhs.typeiter;
}

bool operator!=(const MethodRange::iterator& lhs, const MethodRange::iterator& rhs) { return !(lhs == rhs); }
}  // namespace neko::cpp

FieldRange::iterator::iterator(ObjectTree::iterator typeiter, FieldFlag flag) : typeiter{std::move(typeiter)}, flag{flag}, mode{typeiter.Valid() ? 0 : -1} {
    if (typeiter.Valid()) update();
}

void FieldRange::iterator::update() {
    switch (mode) {
        case 0:
            goto mode_0;
        case 1:
            goto mode_1;
        default:
            neko_assert(false);
            return;
    }

mode_0:
    mode = 1;
    while (typeiter.Valid()) {
        if (!std::get<TypeInfo*>(*typeiter)) {
            ++typeiter;
            continue;
        }

        curfield = std::get<TypeInfo*>(*typeiter)->fieldinfos.begin();
        while (curfield != std::get<TypeInfo*>(*typeiter)->fieldinfos.end()) {
            if (enum_contain_any(flag, curfield->second.fieldptr.GetFieldFlag())) {
                return;  // yield
            mode_1:;
            }
            ++curfield;
        }

        ++typeiter;
    }

    mode = -1;
    return;  // stop
}

FieldRange::iterator& FieldRange::iterator::operator++() {
    update();
    return *this;
}

FieldRange::iterator FieldRange::iterator::operator++(int) {
    FieldRange::iterator iter = *this;
    (void)operator++();
    return iter;
}

namespace neko::cpp {
bool operator==(const FieldRange::iterator& lhs, const FieldRange::iterator& rhs) {
    neko_assert(lhs.flag == rhs.flag);
    if (lhs.Valid()) {
        if (rhs.Valid()) {
            if (lhs.typeiter == rhs.typeiter)
                return lhs.curfield == rhs.curfield;
            else
                return false;
        } else
            return false;
    } else if (rhs.Valid())
        return false;
    else
        return lhs.typeiter == rhs.typeiter;
}

bool operator!=(const FieldRange::iterator& lhs, const FieldRange::iterator& rhs) { return !(lhs == rhs); }
}  // namespace neko::cpp

void ObjectTree::iterator::update() {
    switch (mode) {
        case 0:
            goto mode_0;
        case 1:
            goto mode_1;
        default:
            neko_assert(false);
            return;
    }

mode_0:
    mode = 1;

    if (!std::get<TypeInfo*>(value)) {
        mode = -1;
        return;  // stop
    }

    deriveds.push_back({.obj = std::get<ObjectView>(value), .typeinfo = std::get<TypeInfo*>(value)});

    curbase_valid = false;

    while (!deriveds.empty()) {
        // update curbase

        if (curbase_valid)
            ++deriveds.back().curbase;
        else {
            deriveds.back().curbase = deriveds.back().typeinfo->baseinfos.begin();
            curbase_valid = true;
        }

        // get not visited base

        while (deriveds.back().curbase != deriveds.back().typeinfo->baseinfos.end()) {
            if (!deriveds.back().curbase->second.IsVirtual()) break;

            if (std::find(visitedVBs.begin(), visitedVBs.end(), deriveds.back().curbase->first) == visitedVBs.end()) {
                visitedVBs.push_back(deriveds.back().curbase->first);
                break;
            }

            ++deriveds.back().curbase;
        }

        // check base

        if (deriveds.back().curbase == deriveds.back().typeinfo->baseinfos.end()) {
            deriveds.pop_back();
            continue;
        }

        // get result

        std::get<ObjectView>(value) = {deriveds.back().curbase->first, deriveds.back().obj.GetPtr() ? deriveds.back().curbase->second.StaticCast_DerivedToBase(deriveds.back().obj.GetPtr()) : nullptr};

        if (auto target = neko_refl_instance.typeinfos.find(std::get<ObjectView>(value).GetType()); target != neko_refl_instance.typeinfos.end())
            std::get<TypeInfo*>(value) = &target->second;
        else
            std::get<TypeInfo*>(value) = nullptr;

        return;  // yield

    mode_1:
        // push derived
        if (std::get<TypeInfo*>(value)) {
            deriveds.push_back({.obj = std::get<ObjectView>(value), .typeinfo = std::get<TypeInfo*>(value)});
            curbase_valid = false;
        }
    }

    mode = -1;
    return;  // stop
}

ObjectTree::iterator::iterator(ObjectView obj, bool begin_or_end) : mode{begin_or_end ? 0 : -1}, curbase_valid{true} {
    if (begin_or_end) {
        auto target = neko_refl_instance.typeinfos.find(obj.GetType());
        value = {target == neko_refl_instance.typeinfos.end() ? nullptr : &target->second, obj};
    }
}

ObjectTree::iterator& ObjectTree::iterator::operator++() {
    update();
    return *this;
}

ObjectTree::iterator ObjectTree::iterator::operator++(int) {
    ObjectTree::iterator iter = *this;
    (void)operator++();
    return iter;
}

namespace neko::cpp {
bool operator==(const ObjectTree::iterator& lhs, const ObjectTree::iterator& rhs) { return lhs.deriveds == rhs.deriveds && lhs.mode == rhs.mode; }

bool operator!=(const ObjectTree::iterator& lhs, const ObjectTree::iterator& rhs) { return !(lhs == rhs); }
}  // namespace neko::cpp

VarRange::iterator::iterator(ObjectTree::iterator typeiter, CVRefMode cvref_mode, FieldFlag flag) : typeiter{typeiter}, cvref_mode{cvref_mode}, flag{flag}, mode{typeiter.Valid() ? 0 : -1} {
    neko_assert(!enum_contain_any(cvref_mode, CVRefMode::Volatile));
    if (typeiter.Valid()) update();
}

void VarRange::iterator::update() {
    switch (mode) {
        case 0:
            goto mode_0;
        case 1:
            goto mode_1;
        default:
            neko_assert(false);
            return;
    }

mode_0:
    mode = 1;
    while (typeiter.Valid()) {
        if (!std::get<TypeInfo*>(*typeiter)) {
            ++typeiter;
            continue;
        }

        curfield = std::get<TypeInfo*>(*typeiter)->fieldinfos.begin();
        while (curfield != std::get<TypeInfo*>(*typeiter)->fieldinfos.end()) {
            if (enum_contain_any(flag, curfield->second.fieldptr.GetFieldFlag())) {
                std::get<Name>(value) = curfield->first;
                {  // set var
                    ObjectView var = curfield->second.fieldptr.Var(std::get<ObjectView>(*typeiter).GetPtr());
                    switch (cvref_mode) {
                        case neko::cpp::CVRefMode::Left:
                            std::get<ObjectView>(value) = var.AddLValueReference();
                            break;
                        case neko::cpp::CVRefMode::Right:
                            std::get<ObjectView>(value) = var.AddRValueReference();
                            break;
                        case neko::cpp::CVRefMode::Const:
                            std::get<ObjectView>(value) = var.AddConst();
                            break;
                        case neko::cpp::CVRefMode::ConstLeft:
                            std::get<ObjectView>(value) = var.AddConstLValueReference();
                            break;
                        case neko::cpp::CVRefMode::ConstRight:
                            std::get<ObjectView>(value) = var.AddConstRValueReference();
                            break;
                        default:
                            std::get<ObjectView>(value) = var;
                            break;
                    }
                }
                return;  // yield
            mode_1:;
            }
            ++curfield;
        }

        ++typeiter;
    }

    mode = -1;
    return;  // stop
}

VarRange::iterator& VarRange::iterator::operator++() {
    update();
    return *this;
}

VarRange::iterator VarRange::iterator::operator++(int) {
    VarRange::iterator iter = *this;
    (void)operator++();
    return iter;
}

namespace neko::cpp {
bool operator==(const VarRange::iterator& lhs, const VarRange::iterator& rhs) {
    neko_assert(lhs.flag == rhs.flag);
    neko_assert(lhs.cvref_mode == rhs.cvref_mode);
    if (lhs.Valid()) {
        if (rhs.Valid()) {
            if (lhs.typeiter == rhs.typeiter)
                return lhs.curfield == rhs.curfield;
            else
                return false;
        } else
            return false;
    } else if (rhs.Valid())
        return false;
    else
        return lhs.typeiter == rhs.typeiter;
}

bool operator!=(const VarRange::iterator& lhs, const VarRange::iterator& rhs) { return !(lhs == rhs); }
}  // namespace neko::cpp

std::pmr::memory_resource* neko::cpp::ReflMngr_GetTemporaryResource() { return neko_refl_instance.GetTemporaryResource(); }

ObjectView::operator bool() const noexcept {
    if (ptr && type) {
        if (type.Is<bool>())
            return As<bool>();
        else {
            if (auto rst = IsInvocable(NameIDRegistry::Meta::operator_bool, MethodFlag::Const)) {
                neko_assert(rst.Is<bool>());
                return Invoke<bool>(NameIDRegistry::Meta::operator_bool, ArgsView{}, MethodFlag::Const);
            } else
                return true;
        }
    } else
        return false;
}

ObjectView ObjectView::Var(Name field_name, FieldFlag flag) const { return neko_refl_instance.Var(*this, field_name, flag); }

ObjectView ObjectView::Var(Type base, Name field_name, FieldFlag flag) const { return neko_refl_instance.Var(*this, base, field_name, flag); }

Type ObjectView::BInvoke(Name method_name, void* result_buffer, ArgsView args, MethodFlag flag, std::pmr::memory_resource* temp_args_rsrc) const {
    return neko_refl_instance.BInvoke(*this, method_name, result_buffer, args, flag, temp_args_rsrc);
}

SharedObject ObjectView::MInvoke(Name method_name, std::pmr::memory_resource* rst_rsrc, ArgsView args, MethodFlag flag, std::pmr::memory_resource* temp_args_rsrc) const {
    return neko_refl_instance.MInvoke(*this, method_name, rst_rsrc, args, flag, temp_args_rsrc);
}

SharedObject ObjectView::Invoke(Name method_name, ArgsView args, MethodFlag flag, std::pmr::memory_resource* temp_args_rsrc) const {
    return neko_refl_instance.Invoke(*this, method_name, args, flag, temp_args_rsrc);
}

ObjectView ObjectView::AddConst() const { return {neko_refl_instance.tregistry.RegisterAddConst(type), ptr}; }

ObjectView ObjectView::AddConstLValueReference() const { return {neko_refl_instance.tregistry.RegisterAddConstLValueReference(type), ptr}; }

ObjectView ObjectView::AddConstRValueReference() const { return {neko_refl_instance.tregistry.RegisterAddConstRValueReference(type), ptr}; }

ObjectView ObjectView::AddLValueReference() const { return {neko_refl_instance.tregistry.RegisterAddLValueReference(type), ptr}; }

ObjectView ObjectView::AddLValueReferenceWeak() const { return {neko_refl_instance.tregistry.RegisterAddLValueReferenceWeak(type), ptr}; }

ObjectView ObjectView::AddRValueReference() const { return {neko_refl_instance.tregistry.RegisterAddRValueReference(type), ptr}; }

Type ObjectView::IsInvocable(Name method_name, std::span<const Type> argTypes, MethodFlag flag) const { return neko_refl_instance.IsInvocable(type, method_name, argTypes, flag); }

ObjectView ObjectView::StaticCast_DerivedToBase(Type type) const { return neko_refl_instance.StaticCast_DerivedToBase(*this, type); }

ObjectView ObjectView::StaticCast_BaseToDerived(Type type) const { return neko_refl_instance.StaticCast_BaseToDerived(*this, type); }

ObjectView ObjectView::DynamicCast_BaseToDerived(Type type) const { return neko_refl_instance.DynamicCast_BaseToDerived(*this, type); }

ObjectView ObjectView::StaticCast(Type type) const { return neko_refl_instance.StaticCast(*this, type); }

ObjectView ObjectView::DynamicCast(Type type) const { return neko_refl_instance.DynamicCast(*this, type); }

ObjectTree ObjectView::GetObjectTree() const { return ObjectTree{*this}; }

MethodRange ObjectView::GetMethods(MethodFlag flag) const { return {*this, flag}; }

FieldRange ObjectView::GetFields(FieldFlag flag) const { return {*this, flag}; }

VarRange ObjectView::GetVars(FieldFlag flag) const { return {*this, flag}; }

ContainerType ObjectView::get_container_type() const {
    auto* typeinfo = neko_refl_instance.GetTypeInfo(type);
    if (!typeinfo) return ContainerType::None;

    auto target = typeinfo->attrs.find(Type_of<ContainerType>);
    if (target == typeinfo->attrs.end()) return ContainerType::None;

    return target->As<ContainerType>();
}

SharedObject SharedObject::StaticCast_DerivedToBase(Type base) const {
    auto b = ObjectView::StaticCast_DerivedToBase(base);
    if (!b.GetType().valid()) return {};

    return {b.GetType(), SharedBuffer{buffer, b.GetPtr()}};
}

SharedObject SharedObject::StaticCast_BaseToDerived(Type derived) const {
    auto d = ObjectView::StaticCast_BaseToDerived(derived);
    if (!d.GetType().valid()) return {};

    return {d.GetType(), SharedBuffer{buffer, d.GetPtr()}};
}

SharedObject SharedObject::DynamicCast_BaseToDerived(Type derived) const {
    auto d = ObjectView::StaticCast_BaseToDerived(derived);
    if (!d.GetType().valid()) return {};

    return {d.GetType(), SharedBuffer{buffer, d.GetPtr()}};
}

SharedObject SharedObject::StaticCast(Type type) const {
    auto t = ObjectView::StaticCast(type);
    if (!t.GetType().valid()) return {};

    return {t.GetType(), SharedBuffer{buffer, t.GetPtr()}};
}

SharedObject SharedObject::DynamicCast(Type type) const {
    auto t = ObjectView::DynamicCast(type);
    if (!t.GetType().valid()) return {};

    return {t.GetType(), SharedBuffer{buffer, t.GetPtr()}};
}

bool details::IsPriorityCompatible(std::span<const Type> params, std::span<const Type> argTypes) {
    if (params.size() != argTypes.size()) return false;

    for (size_t i = 0; i < params.size(); i++) {
        if (params[i] == argTypes[i]) continue;

        const auto& lhs = params[i];
        const auto& rhs = argTypes[i];

        if (lhs.IsRValueReference()) {                                // &&{T} | &&{const{T}}
            const auto unref_lhs = lhs.Name_RemoveRValueReference();  // T | const{T}
            neko_assert(!type_name_is_volatile(unref_lhs));
            if (!type_name_is_const(unref_lhs) && rhs.Is(unref_lhs)) continue;  // &&{T} <- T
        } else if (lhs.IsLValueReference()) {                                   // &{T} | &{const{T}}
            const auto unref_lhs = lhs.Name_RemoveLValueReference();            // T | const{T}
            neko_assert(!type_name_is_volatile(unref_lhs));
            if (type_name_is_const(unref_lhs) && rhs.Is(unref_lhs)) continue;  // &{const{T}} <- const{T}
        } else {
            if (lhs.Is(rhs.Name_RemoveRValueReference())) continue;  // T <- &&{T}
        }

        return false;
    }

    return true;
}

bool details::IsRefCompatible(std::span<const Type> paramTypes, std::span<const Type> argTypes) {
    if (paramTypes.size() != argTypes.size()) return false;

    for (size_t i = 0; i < paramTypes.size(); i++) {
        if (!is_ref_compatible(paramTypes[i], argTypes[i])) return false;
    }

    return true;
}

bool details::IsRefConstructible(Type paramType, std::span<const Type> argTypes) {
    auto target = neko_refl_instance.typeinfos.find(paramType);
    if (target == neko_refl_instance.typeinfos.end()) return false;
    const auto& typeinfo = target->second;

    if (typeinfo.is_trivial && (argTypes.empty()                                                        // default ctor
                                || argTypes.size() == 1 && argTypes.front().RemoveCVRef() == paramType  // const/ref ctor
                                )) {
        return true;
    }

    auto [begin_iter, end_iter] = typeinfo.methodinfos.equal_range(NameIDRegistry::Meta::ctor);
    for (auto iter = begin_iter; iter != end_iter; ++iter) {
        if (IsRefCompatible(iter->second.methodptr.GetParamList(), argTypes)) return true;
    }
    return false;
}

bool details::RefConstruct(ObjectView obj, ArgsView args) {
    auto target = neko_refl_instance.typeinfos.find(obj.GetType());
    if (target == neko_refl_instance.typeinfos.end()) return false;

    const auto& typeinfo = target->second;

    if (typeinfo.is_trivial) {
        if (args.Types().empty()) return true;
        if (args.Types().size() == 1 && args.Types().front().RemoveCVRef() == obj.GetType()) {
            std::memcpy(obj.GetPtr(), args.Buffer()[0], typeinfo.size);
            return true;
        }
    }

    auto [begin_iter, end_iter] = typeinfo.methodinfos.equal_range(NameIDRegistry::Meta::ctor);
    for (auto iter = begin_iter; iter != end_iter; ++iter) {
        if (iter->second.methodptr.GetMethodFlag() == MethodFlag::Variable && IsRefCompatible(iter->second.methodptr.GetParamList(), args.Types())) {
            iter->second.methodptr.Invoke(obj.GetPtr(), nullptr, {args.Buffer(), iter->second.methodptr.GetParamList()});
            return true;
        }
    }
    return false;
}

details::NewArgsGuard::NewArgsGuard(bool is_priority, std::pmr::memory_resource* rsrc, std::span<const Type> paramTypes, ArgsView args) {
    auto argTypes = args.Types();
    auto orig_argptr_buffer = args.Buffer();

    if (argTypes.size() != paramTypes.size()) return;

    if (is_priority) {
        is_compatible = IsPriorityCompatible(paramTypes, argTypes);
        new_args = {orig_argptr_buffer, paramTypes};
        return;
    }

    // 1. is compatible ? (collect infos)

    const std::uint8_t num_args = static_cast<std::uint8_t>(argTypes.size());

    ArgInfo info_copiedargs[MaxArgNum + 1];
    std::uint8_t num_copiedargs = 0;
    std::uint32_t size_copiedargs = 0;
    std::uint8_t num_copied_nonptr_args = 0;
    bool contains_objview = false;
    for (std::uint8_t i = 0; i < argTypes.size(); i++) {
        if (paramTypes[i] == argTypes[i]) continue;

        if (paramTypes[i] == Type_of<ObjectView>) {
            contains_objview = true;
            continue;
        }

        const auto& lhs = paramTypes[i];
        const auto& rhs = argTypes[i];

        if (lhs.IsLValueReference()) {                                        // &{T} | &{const{T}}
            const auto unref_lhs = lhs.Name_RemoveLValueReference();          // T | const{T}
            if (type_name_is_const(unref_lhs)) {                              // &{const{T}}
                if (unref_lhs == rhs.Name_RemoveRValueReference()) continue;  // &{const{T}} <- &&{const{T}} || const{T}

                const auto raw_lhs = type_name_remove_const(unref_lhs);                  // T
                if (rhs.Is(raw_lhs) || raw_lhs == rhs.Name_RemoveReference()) continue;  // &{const{T}} <- T | &{T} | &&{T}

                Type raw_lhs_type{raw_lhs};
                if (IsRefConstructible(raw_lhs_type, std::span<const Type>{&rhs, 1}) && neko_refl_instance.IsDestructible(raw_lhs_type)) {
                    auto& info = info_copiedargs[num_copiedargs++];
                    neko_assert(num_copiedargs <= MaxArgNum);

                    info.idx = i;
                    info.is_pointer_or_array = false;
                    info.name = raw_lhs_type.get_name().data();
                    info.name_size = static_cast<std::uint16_t>(raw_lhs_type.get_name().size());
                    info.name_hash = raw_lhs_type.get_id().GetValue();

                    continue;  // &{const{T}} <- T{arg}
                }
            }
        } else if (lhs.IsRValueReference()) {                         // &&{T} | &&{const{T}}
            const auto unref_lhs = lhs.Name_RemoveRValueReference();  // T | const{T}
            if (type_name_is_const(unref_lhs)) {                      // &&{const{T}}
                if (rhs.Is(unref_lhs)) continue;                      // &&{const{T}} <- const{T}

                const auto raw_lhs = type_name_remove_const(unref_lhs);  // T

                if (rhs.Is(raw_lhs)) continue;  // &&{const{T}} <- T

                if (raw_lhs == rhs.Name_RemoveRValueReference()) continue;  // &&{const{T}} <- &&{T}

                Type raw_lhs_type{raw_lhs};
                if (IsRefConstructible(raw_lhs_type, std::span<const Type>{&rhs, 1}) && neko_refl_instance.IsDestructible(raw_lhs_type)) {
                    auto& info = info_copiedargs[num_copiedargs++];
                    neko_assert(num_copiedargs <= MaxArgNum);

                    info.idx = i;
                    info.is_pointer_or_array = false;
                    info.name = raw_lhs_type.get_name().data();
                    info.name_size = static_cast<std::uint16_t>(raw_lhs_type.get_name().size());
                    info.name_hash = raw_lhs_type.get_id().GetValue();

                    continue;  // &&{const{T}} <- T{arg}
                }
            } else {                              // &&{T}
                if (rhs.Is(unref_lhs)) continue;  // &&{T} <- T

                Type raw_lhs_type{unref_lhs};
                if (IsRefConstructible(raw_lhs_type, std::span<const Type>{&rhs, 1}) && neko_refl_instance.IsDestructible(raw_lhs_type)) {
                    auto& info = info_copiedargs[num_copiedargs++];
                    neko_assert(num_copiedargs <= MaxArgNum);

                    info.idx = i;
                    info.is_pointer_or_array = false;
                    info.name = raw_lhs_type.get_name().data();
                    info.name_size = static_cast<std::uint16_t>(raw_lhs_type.get_name().size());
                    info.name_hash = raw_lhs_type.get_id().GetValue();

                    continue;  // &&{T} <- T{arg}
                }
            }
        } else {                                                     // T
            if (lhs.Is(rhs.Name_RemoveRValueReference())) continue;  // T <- &&{T}

            if (IsRefConstructible(lhs, std::span<const Type>{&rhs, 1}) && neko_refl_instance.IsDestructible(lhs)) {
                auto& info = info_copiedargs[num_copiedargs++];
                neko_assert(num_copiedargs <= MaxArgNum);

                info.idx = i;
                info.is_pointer_or_array = false;
                info.name = lhs.get_name().data();
                info.name_size = static_cast<std::uint16_t>(lhs.get_name().size());
                info.name_hash = lhs.get_id().GetValue();

                continue;  // T <- T{arg}
            }
        }

        if (is_pointer_array_compatible(lhs, rhs)) {
            auto raw_lhs = lhs.Name_RemoveCVRef();
            auto& info = info_copiedargs[num_copiedargs++];
            neko_assert(num_copiedargs <= MaxArgNum);
            info.idx = i;
            info.is_pointer_or_array = true;
            info.name = raw_lhs.data();
            info.name_size = static_cast<std::uint16_t>(raw_lhs.size());
            info.name_hash = TypeID{raw_lhs}.GetValue();
            continue;
        }

        return;  // not compatible
    }

    is_compatible = true;

    std::span<const Type> correct_types;
    if (contains_objview) {
        new (&type_buffer) BufferGuard{rsrc, num_args * sizeof(Type), alignof(Type)};
        auto* types = (Type*)type_buffer.Get();
        for (std::uint8_t i = 0; i < num_args; i++) types[i] = paramTypes[i].Is<ObjectView>() ? argTypes[i] : paramTypes[i];
        correct_types = {types, num_args};
    } else
        correct_types = paramTypes;

    if (num_copiedargs == 0) {
        new_args = {orig_argptr_buffer, correct_types};
        return;
    }

    // 2. compute offset and alignment

    std::uint32_t max_alignment = 1;
    for (std::uint8_t k = 0; k < num_copiedargs; ++k) {
        std::uint32_t size, alignment;
        if (info_copiedargs[k].is_pointer_or_array) {
            size = static_cast<std::uint32_t>(sizeof(void*));
            alignment = static_cast<std::uint32_t>(alignof(void*));
        } else {
            ++num_copied_nonptr_args;
            const auto& typeinfo = neko_refl_instance.typeinfos.at(info_copiedargs[k].GetType());
            size = static_cast<std::uint32_t>(typeinfo.size);
            alignment = static_cast<std::uint32_t>(typeinfo.alignment);
        }

        std::uint32_t offset = (size_copiedargs + (alignment - 1)) & ~(alignment - 1);
        info_copiedargs[k].offset = offset;
        size_copiedargs = offset + size;

        if (alignment > max_alignment) max_alignment = alignment;
    }

    // 3. fill buffer

    // buffer = copied args buffer + argptr buffer + non-ptr arg info buffer

    std::uint32_t offset_new_arg_buffer = 0;
    std::uint32_t offset_new_argptr_buffer = (size_copiedargs + alignof(void*) - 1) & ~(alignof(void*) - 1);
    std::uint32_t offset_new_nonptr_arg_info_buffer = offset_new_argptr_buffer + num_args * sizeof(void*);

    std::uint32_t buffer_size = offset_new_nonptr_arg_info_buffer + num_copied_nonptr_args * sizeof(ArgInfo);

    new (&buffer) BufferGuard{rsrc, buffer_size, max_alignment};

    auto new_arg_buffer = forward_offset(buffer, offset_new_arg_buffer);
    auto new_argptr_buffer = reinterpret_cast<void**>(forward_offset(buffer, offset_new_argptr_buffer));
    auto new_nonptr_arg_info_buffer = reinterpret_cast<ArgInfo*>(forward_offset(buffer, offset_new_nonptr_arg_info_buffer));

    nonptr_arg_infos = {new_nonptr_arg_info_buffer, num_copied_nonptr_args};

    info_copiedargs[num_copiedargs].idx = static_cast<std::uint8_t>(-1);  // guard
    std::uint8_t idx_copiedargs = 0, idx_nonptr_args = 0;
    for (std::uint8_t i = 0; i < num_args; i++) {
        const auto& info = info_copiedargs[idx_copiedargs];
        if (i < info.idx) {
            new_argptr_buffer[i] = orig_argptr_buffer[i];
            continue;
        }
        neko_assert(i == info.idx);

        void* arg_buffer = forward_offset(new_arg_buffer, info.offset);
        new_argptr_buffer[i] = arg_buffer;

        // copy
        if (info.is_pointer_or_array)
            buffer_as<void*>(arg_buffer) = orig_argptr_buffer[i];
        else {
            bool success = RefConstruct(ObjectView{info.GetType(), arg_buffer}, ArgsView{&orig_argptr_buffer[i], std::span<const Type>{&argTypes[i], 1}});
            neko_assert(success);
            nonptr_arg_infos[idx_nonptr_args++] = info;
        }

        ++idx_copiedargs;
    }
    neko_assert(idx_copiedargs == num_copiedargs);
    neko_assert(idx_nonptr_args == num_copied_nonptr_args);

    new_args = {new_argptr_buffer, correct_types};
}

details::NewArgsGuard::~NewArgsGuard() {
    if (buffer.Get()) {
        for (const auto& info : nonptr_arg_infos) neko_refl_instance.Destruct({info.GetType(), new_args[info.idx].GetPtr()});
    }
}
