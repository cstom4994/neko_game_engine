
#ifndef NEKO_DBGUI_HPP
#define NEKO_DBGUI_HPP

#include <map>
#include <ranges>

#include "engine/common/neko_containers.h"
#include "engine/common/neko_str.h"
#include "engine/common/neko_util.h"
#include "engine/gui/neko_imgui_utils.hpp"
#include "engine/utility/defer.hpp"
#include "engine/utility/enum.hpp"
#include "engine/utility/module.hpp"

namespace neko {

using neko_dbgui_func = neko_function<u8(u8)>;

ENUM_HPP_CLASS_DECL(neko_dbgui_flags, u32, (window = 1 << 0)(no_visible = 1 << 1));
ENUM_HPP_REGISTER_TRAITS(neko_dbgui_flags);

struct neko_dbgui_win {
    neko_string name;
    neko_dbgui_func func;
    cpp::bitflags::bitflags<neko_dbgui_flags> flags;
};

class dbgui : public module<dbgui> {
public:
public:
    dbgui() noexcept { __init(); }
    ~dbgui() noexcept { __end(); }

    neko_move_only(dbgui);

    void __init(){};
    void __end(){};

public:
    dbgui& create(const neko_string& name, const neko_dbgui_func& f, neko_dbgui_flags flags = neko_dbgui_flags::window) {
        // 创建 dbgui 实例
        dbgui_list.insert(std::make_pair(name, neko_dbgui_win{name, f, flags}));
        return *this;
    }

    dbgui& update(const neko_string& name, const neko_dbgui_func& f) {
        if (dbgui_list.count(name) != 1) neko_assert(0);
        auto origin = dbgui_list[name];
        dbgui_list[name] = neko_dbgui_win{origin.name, func_combine(f, origin.func), origin.flags};
        return *this;
    }

    cpp::bitflags::bitflags<neko_dbgui_flags>& flags(const neko_string& name) {
        if (dbgui_list.count(name) != 1) neko_assert(0);
        return dbgui_list[name].flags;
    }

    void draw() const {
        for (auto& d : dbgui_list) {
            cpp::bitflags::bitflags<neko_dbgui_flags> flags(d.second.flags);
            if ((flags & neko_dbgui_flags::no_visible) == neko_dbgui_flags::no_visible) return;
            if (ImGui::Begin(d.first.c_str())) {
                neko_defer([&] { d.second.func(0); });
            }
            ImGui::End();
        }
    }

private:
    std::map<neko_string, neko_dbgui_win> dbgui_list;
};

}  // namespace neko

#endif  // !NEKO_DBGUI_HPP
