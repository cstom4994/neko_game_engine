
#ifndef NEKO_LUA_REG_H
#define NEKO_LUA_REG_H

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#define SPRINTF_F _snprintf_s

struct strtoll_tool_t {
    static long do_strtoll(const char* s, const char*, int) { return atol(s); }
};
#define strtoll strtoll_tool_t::do_strtoll
#define strtoull (unsigned long)strtoll_tool_t::do_strtoll

namespace neko {

#define INHERIT_TABLE "inherit_table"

struct cpp_void_t {};

struct lua_string_tool_t {
    inline static const char* c_str(const std::string& s_) { return s_.c_str(); }
    inline static const char* c_str(const char* s_) { return s_; }
};

class lua_exception_t : public std::exception {
public:
    explicit lua_exception_t(const char* err_) : m_err(err_) {}
    explicit lua_exception_t(const std::string& err_) : m_err(err_) {}
    ~lua_exception_t() throw() {}

    const char* what() const throw() { return m_err.c_str(); }

private:
    std::string m_err;
};

class neko_lua_tool_t {
public:
    static void dump_stack(lua_State* ls_) {
        int i;
        int top = lua_gettop(ls_);

        for (i = 1; i <= top; i++) {
            int t = lua_type(ls_, i);
            switch (t) {
                case LUA_TSTRING: {
                    printf("`%s'", lua_tostring(ls_, i));
                } break;
                case LUA_TBOOLEAN: {
                    printf(lua_toboolean(ls_, i) ? "true" : "false");
                } break;
                case LUA_TNUMBER: {
                    printf("`%g`", lua_tonumber(ls_, i));
                } break;
                case LUA_TTABLE: {
                    printf("table end\n");
                    lua_pushnil(ls_);
                    while (lua_next(ls_, i) != 0) {
                        printf(" %s - %s\n", lua_typename(ls_, lua_type(ls_, -2)), lua_typename(ls_, lua_type(ls_, -1)));
                        lua_pop(ls_, 1);
                    }
                    printf("table end");
                } break;
                default: {
                    printf("`%s`", lua_typename(ls_, t));
                } break;
            }
            printf(" ");
        }
        printf("\n");
    }
    static std::string dump_error(lua_State* ls_, const char* fmt, ...) {
        std::string ret;
        char buff[4096];

        va_list argp;
        va_start(argp, fmt);
#ifndef _WIN32
        vsnprintf(buff, sizeof(buff), fmt, argp);
#else
        vsnprintf_s(buff, sizeof(buff), sizeof(buff), fmt, argp);
#endif
        va_end(argp);

        ret = buff;
        SPRINTF_F(buff, sizeof(buff), " tracback:%s", lua_tostring(ls_, -1));
        ret += buff;

        return ret;
    }
};

typedef int (*lua_function_t)(lua_State* L);

class lua_nil_t {};

template <typename T>
struct userdata_for_object_t {
    userdata_for_object_t(T* p_ = NULL) : obj(p_) {}
    T* obj;
};

template <typename T>
struct lua_type_info_t {
    static void set_name(const std::string& name_, std::string inherit_name_ = "") {
        size_t n = name_.length() > sizeof(name) - 1 ? sizeof(name) - 1 : name_.length();
#ifndef _WIN32
        ::strncpy(name, name_.c_str(), n);
#else
        ::strncpy_s(name, name_.c_str(), n);
#endif
        if (false == inherit_name_.empty()) {
            n = inherit_name_.length() > sizeof(inherit_name) - 1 ? sizeof(inherit_name) - 1 : inherit_name_.length();
#ifndef _WIN32
            ::strncpy(inherit_name, inherit_name_.c_str(), n);
#else
            ::strncpy_s(inherit_name, inherit_name_.c_str(), n);
#endif
        }
    }
    inline static const char* get_name() { return name; }
    inline static const char* get_inherit_name() { return inherit_name; }
    inline static bool is_registed() { return name[0] != '\0'; }
    inline static bool is_inherit() { return inherit_name[0] != '\0'; }
    static char name[128];
    static char inherit_name[128];
};
template <typename T>
char lua_type_info_t<T>::name[128] = {0};
template <typename T>
char lua_type_info_t<T>::inherit_name[128] = {0};

template <typename ARG_TYPE>
struct basetype_ptr_traits_t;
template <>
struct basetype_ptr_traits_t<const std::string&> {
    typedef std::string arg_type_t;
};
template <>
struct basetype_ptr_traits_t<std::string&> {
    typedef std::string arg_type_t;
};
template <>
struct basetype_ptr_traits_t<std::string> {
    typedef std::string arg_type_t;
};
template <>
struct basetype_ptr_traits_t<const char*> {
    typedef char* arg_type_t;
};
template <>
struct basetype_ptr_traits_t<char*> {
    typedef char* arg_type_t;
};
template <>
struct basetype_ptr_traits_t<char> {
    typedef char arg_type_t;
};
template <>
struct basetype_ptr_traits_t<unsigned char> {
    typedef unsigned char arg_type_t;
};
template <>
struct basetype_ptr_traits_t<short> {
    typedef short arg_type_t;
};
template <>
struct basetype_ptr_traits_t<unsigned short> {
    typedef unsigned short arg_type_t;
};
template <>
struct basetype_ptr_traits_t<int> {
    typedef int arg_type_t;
};
template <>
struct basetype_ptr_traits_t<unsigned int> {
    typedef unsigned int arg_type_t;
};
template <>
struct basetype_ptr_traits_t<long> {
    typedef long arg_type_t;
};
template <>
struct basetype_ptr_traits_t<unsigned long> {
    typedef unsigned long arg_type_t;
};
template <>
struct basetype_ptr_traits_t<long long> {
    typedef long long arg_type_t;
};
template <>
struct basetype_ptr_traits_t<unsigned long long> {
    typedef unsigned long long arg_type_t;
};

template <>
struct basetype_ptr_traits_t<float> {
    typedef float arg_type_t;
};
template <>
struct basetype_ptr_traits_t<bool> {
    typedef bool arg_type_t;
};

template <>
struct basetype_ptr_traits_t<double> {
    typedef double arg_type_t;
};
template <typename T>
struct basetype_ptr_traits_t<const T&> {
    typedef T* arg_type_t;
};
template <typename T>
struct basetype_ptr_traits_t<T&> {
    typedef T* arg_type_t;
};
template <typename T>
struct basetype_ptr_traits_t<T*> {
    typedef T* arg_type_t;
};
template <typename T>
struct basetype_ptr_traits_t<const T*> {
    typedef T* arg_type_t;
};
template <typename T>
struct basetype_ptr_traits_t<std::vector<T> > {
    typedef std::vector<T> arg_type_t;
};
template <typename T>
struct basetype_ptr_traits_t<std::list<T> > {
    typedef std::list<T> arg_type_t;
};
template <typename T>
struct basetype_ptr_traits_t<std::set<T> > {
    typedef std::set<T> arg_type_t;
};
template <typename K, typename V>
struct basetype_ptr_traits_t<std::map<K, V> > {
    typedef std::map<K, V> arg_type_t;
};
template <typename T>
struct basetype_ptr_traits_t<std::vector<T>&> {
    typedef std::vector<T> arg_type_t;
};
template <typename T>
struct basetype_ptr_traits_t<std::list<T>&> {
    typedef std::list<T> arg_type_t;
};
template <typename T>
struct basetype_ptr_traits_t<std::set<T>&> {
    typedef std::set<T> arg_type_t;
};
template <typename K, typename V>
struct basetype_ptr_traits_t<std::map<K, V>&> {
    typedef std::map<K, V> arg_type_t;
};
template <typename T>
struct basetype_ptr_traits_t<const std::vector<T>&> {
    typedef std::vector<T> arg_type_t;
};
template <typename T>
struct basetype_ptr_traits_t<const std::list<T>&> {
    typedef std::list<T> arg_type_t;
};
template <typename T>
struct basetype_ptr_traits_t<const std::set<T>&> {
    typedef std::set<T> arg_type_t;
};
template <typename K, typename V>
struct basetype_ptr_traits_t<const std::map<K, V>&> {
    typedef std::map<K, V> arg_type_t;
};

//!-------------------------------------------------------------------------------------------------------------------------
template <typename ARG_TYPE>
struct p_t;

template <typename ARG_TYPE>
struct p_t {
    static ARG_TYPE r(ARG_TYPE a) { return a; }
    static ARG_TYPE& r(ARG_TYPE* a) { return *a; }
};
template <typename ARG_TYPE>
struct p_t<ARG_TYPE&> {
    static ARG_TYPE& r(ARG_TYPE& a) { return a; }
    static ARG_TYPE& r(ARG_TYPE* a) { return *a; }
};
//! #########################################################################################################################
template <typename ARG_TYPE>
struct reference_traits_t;

template <typename ARG_TYPE>
struct reference_traits_t {
    typedef ARG_TYPE arg_type_t;
};

template <>
struct reference_traits_t<const std::string&> {
    typedef std::string arg_type_t;
};

template <>
struct reference_traits_t<std::string&> {
    typedef std::string arg_type_t;
};

template <typename T>
struct reference_traits_t<const T*> {
    typedef T* arg_type_t;
};
template <typename T>
struct reference_traits_t<const T&> {
    typedef T arg_type_t;
};

template <>
struct reference_traits_t<const char*> {
    typedef char* arg_type_t;
};

template <typename T>
struct init_value_traits_t;

template <typename T>
struct init_value_traits_t {
    inline static T value() { return T(); }
};

template <typename T>
struct init_value_traits_t<const T*> {
    inline static T* value() { return NULL; }
};

template <typename T>
struct init_value_traits_t<const T&> {
    inline static T value() { return T(); }
};

template <>
struct init_value_traits_t<std::string> {
    inline static const char* value() { return ""; }
};

template <>
struct init_value_traits_t<const std::string&> {
    inline static const char* value() { return ""; }
};

template <typename T>
struct lua_op_t {
    static void push_stack(lua_State* ls_, const char* arg_) { lua_pushstring(ls_, arg_); }
    /*
    static int lua_to_value(lua_State* ls_, int pos_, char*& param_)
    {
        const char* str = luaL_checkstring(ls_, pos_);
        param_ = (char*)str;
        return 0;
    }*/
};

template <>
struct lua_op_t<const char*> {
    static void push_stack(lua_State* ls_, const char* arg_) { lua_pushstring(ls_, arg_); }
    static int lua_to_value(lua_State* ls_, int pos_, char*& param_) {
        const char* str = luaL_checkstring(ls_, pos_);
        param_ = (char*)str;
        return 0;
    }
};
template <>
struct lua_op_t<char*> {
    static void push_stack(lua_State* ls_, const char* arg_) { lua_pushstring(ls_, arg_); }
    static int lua_to_value(lua_State* ls_, int pos_, char*& param_) {
        const char* str = luaL_checkstring(ls_, pos_);
        param_ = (char*)str;
        return 0;
    }
};

template <>
struct lua_op_t<lua_nil_t> {
    static void push_stack(lua_State* ls_, const lua_nil_t& arg_) { lua_pushnil(ls_); }
};

template <>
struct lua_op_t<cpp_void_t> {
    static int get_ret_value(lua_State* ls_, int pos_, cpp_void_t& param_) { return 0; }
};

template <>
struct lua_op_t<int64_t> {
    static void push_stack(lua_State* ls_, int64_t arg_) {
#if LUA_VERSION_NUM >= 503
        lua_pushinteger(ls_, arg_);
#else
        stringstream ss;
        ss << arg_;
        string str = ss.str();
        lua_pushlstring(ls_, str.c_str(), str.length());
#endif
    }

    static int get_ret_value(lua_State* ls_, int pos_, int64_t& param_) {
#if LUA_VERSION_NUM >= 503
        if (!lua_isinteger(ls_, pos_)) {
            return -1;
        }
        param_ = lua_tointeger(ls_, pos_);
#else
        if (!lua_isstring(ls_, pos_)) {
            return -1;
        }

        size_t len = 0;
        const char* src = lua_tolstring(ls_, pos_, &len);
        param_ = (int64_t)strtoll(src, NULL, 10);
#endif
        return 0;
    }

    static int lua_to_value(lua_State* ls_, int pos_, int64_t& param_) {
#if LUA_VERSION_NUM >= 503
        param_ = luaL_checkinteger(ls_, pos_);
#else
        size_t len = 0;
        const char* str = luaL_checklstring(ls_, pos_, &len);
        param_ = (int64_t)strtoll(str, NULL, 10);
#endif
        return 0;
    }
};

template <>
struct lua_op_t<uint64_t> {
    static void push_stack(lua_State* ls_, uint64_t arg_) {
        std::stringstream ss;
        ss << arg_;
        std::string str = ss.str();
        lua_pushlstring(ls_, str.c_str(), str.length());
    }

    static int get_ret_value(lua_State* ls_, int pos_, uint64_t& param_) {
        if (!lua_isstring(ls_, pos_)) {
            return -1;
        }

        size_t len = 0;
        const char* src = lua_tolstring(ls_, pos_, &len);
        param_ = (uint64_t)strtoull(src, NULL, 10);
        return 0;
    }

    static int lua_to_value(lua_State* ls_, int pos_, uint64_t& param_) {
        size_t len = 0;
        const char* str = luaL_checklstring(ls_, pos_, &len);
        param_ = (uint64_t)strtoull(str, NULL, 10);
        return 0;
    }
};

template <>
struct lua_op_t<int8_t> {

    static void push_stack(lua_State* ls_, int8_t arg_) { lua_pushnumber(ls_, (lua_Number)arg_); }
    static int get_ret_value(lua_State* ls_, int pos_, int8_t& param_) {
        if (!lua_isnumber(ls_, pos_)) {
            return -1;
        }
        param_ = (int8_t)lua_tonumber(ls_, pos_);
        return 0;
    }
    static int lua_to_value(lua_State* ls_, int pos_, int8_t& param_) {
        param_ = (int8_t)luaL_checknumber(ls_, pos_);
        return 0;
    }
};

template <>
struct lua_op_t<uint8_t> {
    static void push_stack(lua_State* ls_, uint8_t arg_) { lua_pushnumber(ls_, (lua_Number)arg_); }
    static int get_ret_value(lua_State* ls_, int pos_, uint8_t& param_) {
        if (!lua_isnumber(ls_, pos_)) {
            return -1;
        }
        param_ = (uint8_t)lua_tonumber(ls_, pos_);
        return 0;
    }
    static int lua_to_value(lua_State* ls_, int pos_, uint8_t& param_) {
        param_ = (uint8_t)luaL_checknumber(ls_, pos_);
        return 0;
    }
};
#ifdef _WIN32

template <>
struct lua_op_t<char> {

    static void push_stack(lua_State* ls_, char arg_) { lua_pushnumber(ls_, (lua_Number)arg_); }
    static int get_ret_value(lua_State* ls_, int pos_, char& param_) {
        if (!lua_isnumber(ls_, pos_)) {
            return -1;
        }
        param_ = (char)lua_tonumber(ls_, pos_);
        return 0;
    }
    static int lua_to_value(lua_State* ls_, int pos_, char& param_) {
        param_ = (char)luaL_checknumber(ls_, pos_);
        return 0;
    }
};

#endif
template <>
struct lua_op_t<int16_t> {
    static void push_stack(lua_State* ls_, int16_t arg_) { lua_pushnumber(ls_, (lua_Number)arg_); }
    static int get_ret_value(lua_State* ls_, int pos_, int16_t& param_) {
        if (!lua_isnumber(ls_, pos_)) {
            return -1;
        }
        param_ = (int16_t)lua_tonumber(ls_, pos_);
        return 0;
    }
    static int lua_to_value(lua_State* ls_, int pos_, int16_t& param_) {
        param_ = (int16_t)luaL_checknumber(ls_, pos_);
        return 0;
    }
};
template <>
struct lua_op_t<uint16_t> {

    static void push_stack(lua_State* ls_, uint16_t arg_) { lua_pushnumber(ls_, (lua_Number)arg_); }
    static int get_ret_value(lua_State* ls_, int pos_, uint16_t& param_) {
        if (!lua_isnumber(ls_, pos_)) {
            return -1;
        }
        param_ = (uint16_t)lua_tonumber(ls_, pos_);
        return 0;
    }
    static int lua_to_value(lua_State* ls_, int pos_, uint16_t& param_) {
        param_ = (uint16_t)luaL_checknumber(ls_, pos_);
        return 0;
    }
};
template <>
struct lua_op_t<int32_t> {
    static void push_stack(lua_State* ls_, int32_t arg_) { lua_pushnumber(ls_, (lua_Number)arg_); }
    static int get_ret_value(lua_State* ls_, int pos_, int32_t& param_) {
        if (!lua_isnumber(ls_, pos_)) {
            return -1;
        }
        param_ = (int32_t)lua_tonumber(ls_, pos_);
        return 0;
    }
    static int lua_to_value(lua_State* ls_, int pos_, int32_t& param_) {
        param_ = (int32_t)luaL_checknumber(ls_, pos_);
        return 0;
    }
};
template <>
struct lua_op_t<uint32_t> {

    static void push_stack(lua_State* ls_, uint32_t arg_) { lua_pushnumber(ls_, (lua_Number)arg_); }
    static int get_ret_value(lua_State* ls_, int pos_, uint32_t& param_) {
        if (!lua_isnumber(ls_, pos_)) {
            return -1;
        }
        param_ = (uint32_t)lua_tonumber(ls_, pos_);
        return 0;
    }
    static int lua_to_value(lua_State* ls_, int pos_, uint32_t& param_) {
        param_ = (uint32_t)luaL_checknumber(ls_, pos_);
        return 0;
    }
};

template <>
struct lua_op_t<bool> {
    static void push_stack(lua_State* ls_, bool arg_) { lua_pushboolean(ls_, arg_); }

    static int get_ret_value(lua_State* ls_, int pos_, bool& param_) {
        //! nil 自动转换为false
        if (lua_isnil(ls_, pos_)) {
            param_ = false;
            return 0;
        }
        if (!lua_isboolean(ls_, pos_)) {
            return -1;
        }

        param_ = (0 != lua_toboolean(ls_, pos_));
        return 0;
    }
    static int lua_to_value(lua_State* ls_, int pos_, bool& param_) {
        luaL_checktype(ls_, pos_, LUA_TBOOLEAN);
        param_ = (0 != lua_toboolean(ls_, pos_));
        return 0;
    }
};

template <>
struct lua_op_t<std::string> {

    static void push_stack(lua_State* ls_, const std::string& arg_) { lua_pushlstring(ls_, arg_.c_str(), arg_.length()); }

    static int get_ret_value(lua_State* ls_, int pos_, std::string& param_) {
        if (!lua_isstring(ls_, pos_)) {
            return -1;
        }

        lua_pushvalue(ls_, pos_);
        size_t len = 0;
        const char* src = lua_tolstring(ls_, -1, &len);
        param_.assign(src, len);
        lua_pop(ls_, 1);

        return 0;
    }
    static int lua_to_value(lua_State* ls_, int pos_, std::string& param_) {
        size_t len = 0;
        const char* str = luaL_checklstring(ls_, pos_, &len);
        param_.assign(str, len);
        return 0;
    }
};

template <>
struct lua_op_t<const std::string&> {
    static void push_stack(lua_State* ls_, const std::string& arg_) { lua_pushlstring(ls_, arg_.c_str(), arg_.length()); }

    static int get_ret_value(lua_State* ls_, int pos_, std::string& param_) {
        if (!lua_isstring(ls_, pos_)) {
            return -1;
        }

        lua_pushvalue(ls_, pos_);
        size_t len = 0;
        const char* src = lua_tolstring(ls_, -1, &len);
        param_.assign(src, len);
        lua_pop(ls_, 1);

        return 0;
    }
    static int lua_to_value(lua_State* ls_, int pos_, std::string& param_) {
        size_t len = 0;
        const char* str = luaL_checklstring(ls_, pos_, &len);
        param_.assign(str, len);
        return 0;
    }
};
template <>
struct lua_op_t<float> {
    static void push_stack(lua_State* ls_, float arg_) { lua_pushnumber(ls_, (lua_Number)arg_); }
    static int get_ret_value(lua_State* ls_, int pos_, float& param_) {
        if (!lua_isnumber(ls_, pos_)) {
            return -1;
        }
        param_ = (float)lua_tonumber(ls_, pos_);
        return 0;
    }
    static int lua_to_value(lua_State* ls_, int pos_, float& param_) {
        param_ = (float)luaL_checknumber(ls_, pos_);
        return 0;
    }
};
template <>
struct lua_op_t<double> {
    static void push_stack(lua_State* ls_, double arg_) { lua_pushnumber(ls_, (lua_Number)arg_); }
    static int get_ret_value(lua_State* ls_, int pos_, double& param_) {
        if (!lua_isnumber(ls_, pos_)) {
            return -1;
        }
        param_ = (double)lua_tonumber(ls_, pos_);
        return 0;
    }
    static int lua_to_value(lua_State* ls_, int pos_, double& param_) {
        param_ = (double)luaL_checknumber(ls_, pos_);
        return 0;
    }
};
/*template<> struct lua_op_t<long>
{

    static void push_stack(lua_State* ls_, long arg_)
    {
        lua_pushnumber(ls_, (lua_Number)arg_);
    }
    static int get_ret_value(lua_State* ls_, int pos_, long& param_)
    {
        if (!lua_isnumber(ls_, pos_))
        {
            return -1;
        }
        param_ = (long)lua_tonumber(ls_, pos_);
        return 0;
    }
    static int lua_to_value(lua_State* ls_, int pos_, long& param_)
    {
        param_ = (long)luaL_checknumber(ls_, pos_);
        return 0;
    }
};*/
template <>
struct lua_op_t<void*> {
    static void push_stack(lua_State* ls_, void* arg_) { lua_pushlightuserdata(ls_, arg_); }

    static int get_ret_value(lua_State* ls_, int pos_, void*& param_) {
        if (!lua_isuserdata(ls_, pos_)) {
            char buff[128];
            SPRINTF_F(buff, sizeof(buff), "userdata param expected, but type<%s> provided", lua_typename(ls_, lua_type(ls_, pos_)));
            printf("%s\n", buff);
            return -1;
        }

        param_ = lua_touserdata(ls_, pos_);
        return 0;
    }

    static int lua_to_value(lua_State* ls_, int pos_, void*& param_) {
        if (!lua_isuserdata(ls_, pos_)) {
            luaL_argerror(ls_, 1, "userdata param expected");
            return -1;
        }
        param_ = lua_touserdata(ls_, pos_);
        return 0;
    }
};

template <typename T>
struct lua_op_t<T*> {
    static void push_stack(lua_State* ls_, T& arg_) {
        void* ptr = lua_newuserdata(ls_, sizeof(userdata_for_object_t<T>));
        new (ptr) userdata_for_object_t<T>(&arg_);

        luaL_getmetatable(ls_, lua_type_info_t<T>::get_name());
        lua_setmetatable(ls_, -2);
    }
    static void push_stack(lua_State* ls_, const T& arg_) {
        void* ptr = lua_newuserdata(ls_, sizeof(userdata_for_object_t<const T>));
        new (ptr) userdata_for_object_t<const T>(&arg_);

        luaL_getmetatable(ls_, lua_type_info_t<T>::get_name());
        lua_setmetatable(ls_, -2);
    }
    static void push_stack(lua_State* ls_, T* arg_) {
        void* ptr = lua_newuserdata(ls_, sizeof(userdata_for_object_t<T>));
        new (ptr) userdata_for_object_t<T>(arg_);

        luaL_getmetatable(ls_, lua_type_info_t<T>::get_name());
        lua_setmetatable(ls_, -2);
    }

    static int get_ret_value(lua_State* ls_, int pos_, T*& param_) {
        if (false == lua_type_info_t<T>::is_registed()) {
            luaL_argerror(ls_, pos_, "type not supported");
        }

        void* arg_data = lua_touserdata(ls_, pos_);

        if (NULL == arg_data) {
            printf("expect<%s> but <%s> NULL\n", lua_type_info_t<T>::get_name(), lua_typename(ls_, lua_type(ls_, pos_)));
            return -1;
        }

        if (0 == lua_getmetatable(ls_, pos_)) {
            return -1;
        }

        luaL_getmetatable(ls_, lua_type_info_t<T>::get_name());
        if (0 == lua_rawequal(ls_, -1, -2)) {
            lua_getfield(ls_, -2, INHERIT_TABLE);
            if (0 == lua_rawequal(ls_, -1, -2)) {
                printf("expect<%s> but <%s> not equal\n", lua_type_info_t<T>::get_name(), lua_typename(ls_, lua_type(ls_, pos_)));
                lua_pop(ls_, 3);
                return -1;
            }
            lua_pop(ls_, 3);
        } else {
            lua_pop(ls_, 2);
        }
        T* ret_ptr = ((userdata_for_object_t<T>*)arg_data)->obj;
        if (NULL == ret_ptr) {
            return -1;
        }

        param_ = ret_ptr;
        return 0;
    }

    static int lua_to_value(lua_State* ls_, int pos_, T*& param_) {
        if (false == lua_type_info_t<T>::is_registed()) {
            luaL_argerror(ls_, pos_, "type not supported");
        }
        void* arg_data = lua_touserdata(ls_, pos_);

        if (NULL == arg_data || 0 == lua_getmetatable(ls_, pos_)) {
            char buff[128];
            SPRINTF_F(buff, sizeof(buff), "`%s` arg1 connot be null", lua_type_info_t<T>::get_name());
            luaL_argerror(ls_, pos_, buff);
        }

        luaL_getmetatable(ls_, lua_type_info_t<T>::get_name());
        if (0 == lua_rawequal(ls_, -1, -2)) {
            lua_getfield(ls_, -2, INHERIT_TABLE);
            if (0 == lua_rawequal(ls_, -1, -2)) {
                lua_pop(ls_, 3);
                char buff[128];
                SPRINTF_F(buff, sizeof(buff), "`%s` arg1 type not equal", lua_type_info_t<T>::get_name());
                luaL_argerror(ls_, pos_, buff);
            }
            lua_pop(ls_, 3);
        } else {
            lua_pop(ls_, 2);
        }

        T* ret_ptr = ((userdata_for_object_t<T>*)arg_data)->obj;
        if (NULL == ret_ptr) {
            char buff[128];
            SPRINTF_F(buff, sizeof(buff), "`%s` object ptr can't be null", lua_type_info_t<T>::get_name());
            luaL_argerror(ls_, pos_, buff);
        }

        param_ = ret_ptr;
        return 0;
    }
};

template <typename T>
struct lua_op_t<const T*> {
    static void push_stack(lua_State* ls_, const T* arg_) { lua_op_t<T*>::push_stack(ls_, (T*)arg_); }

    static int get_ret_value(lua_State* ls_, int pos_, T*& param_) { return lua_op_t<T*>::get_ret_value(ls_, pos_, param_); }

    static int lua_to_value(lua_State* ls_, int pos_, T*& param_) { return lua_op_t<T*>::lua_to_value(ls_, pos_, param_); }
};

template <typename T>
struct lua_op_t<std::vector<T> > {
    static void push_stack(lua_State* ls_, const std::vector<T>& arg_) {
        lua_newtable(ls_);
        typename vector<T>::const_iterator it = arg_.begin();
        for (int i = 1; it != arg_.end(); ++it, ++i) {
            lua_op_t<int>::push_stack(ls_, i);
            lua_op_t<T>::push_stack(ls_, *it);
            lua_settable(ls_, -3);
        }
    }

    static int get_ret_value(lua_State* ls_, int pos_, std::vector<T>& param_) {
        if (0 == lua_istable(ls_, pos_)) {
            return -1;
        }
        lua_pushnil(ls_);
        int real_pos = pos_;
        if (pos_ < 0) real_pos = real_pos - 1;

        while (lua_next(ls_, real_pos) != 0) {
            param_.push_back(T());
            if (lua_op_t<T>::get_ret_value(ls_, -1, param_[param_.size() - 1]) < 0) {
                return -1;
            }
            lua_pop(ls_, 1);
        }
        return 0;
    }

    static int lua_to_value(lua_State* ls_, int pos_, std::vector<T>& param_) {
        luaL_checktype(ls_, pos_, LUA_TTABLE);

        lua_pushnil(ls_);
        int real_pos = pos_;
        if (pos_ < 0) real_pos = real_pos - 1;
        while (lua_next(ls_, real_pos) != 0) {
            param_.push_back(T());
            if (lua_op_t<T>::lua_to_value(ls_, -1, param_[param_.size() - 1]) < 0) {
                luaL_argerror(ls_, pos_ > 0 ? pos_ : -pos_, "convert to vector failed");
            }
            lua_pop(ls_, 1);
        }
        return 0;
    }
};

template <typename T>
struct lua_op_t<std::list<T> > {
    static void push_stack(lua_State* ls_, const std::list<T>& arg_) {
        lua_newtable(ls_);
        typename list<T>::const_iterator it = arg_.begin();
        for (int i = 1; it != arg_.end(); ++it, ++i) {
            lua_op_t<int>::push_stack(ls_, i);
            lua_op_t<T>::push_stack(ls_, *it);
            lua_settable(ls_, -3);
        }
    }

    static int get_ret_value(lua_State* ls_, int pos_, std::list<T>& param_) {
        if (0 == lua_istable(ls_, pos_)) {
            return -1;
        }
        lua_pushnil(ls_);
        int real_pos = pos_;
        if (pos_ < 0) real_pos = real_pos - 1;

        while (lua_next(ls_, real_pos) != 0) {
            param_.push_back(T());
            if (lua_op_t<T>::get_ret_value(ls_, -1, (param_.back())) < 0) {
                return -1;
            }
            lua_pop(ls_, 1);
        }
        return 0;
    }

    static int lua_to_value(lua_State* ls_, int pos_, std::list<T>& param_) {
        luaL_checktype(ls_, pos_, LUA_TTABLE);

        lua_pushnil(ls_);
        int real_pos = pos_;
        if (pos_ < 0) real_pos = real_pos - 1;
        while (lua_next(ls_, real_pos) != 0) {
            param_.push_back(T());
            if (lua_op_t<T>::lua_to_value(ls_, -1, (param_.back())) < 0) {
                luaL_argerror(ls_, pos_ > 0 ? pos_ : -pos_, "convert to vector failed");
            }
            lua_pop(ls_, 1);
        }
        return 0;
    }
};

template <typename T>
struct lua_op_t<std::set<T> > {
    static void push_stack(lua_State* ls_, const std::set<T>& arg_) {
        lua_newtable(ls_);
        typename set<T>::const_iterator it = arg_.begin();
        for (int i = 1; it != arg_.end(); ++it, ++i) {
            lua_op_t<int>::push_stack(ls_, i);
            lua_op_t<T>::push_stack(ls_, *it);
            lua_settable(ls_, -3);
        }
    }

    static int get_ret_value(lua_State* ls_, int pos_, std::set<T>& param_) {
        if (0 == lua_istable(ls_, pos_)) {
            return -1;
        }
        lua_pushnil(ls_);
        int real_pos = pos_;
        if (pos_ < 0) real_pos = real_pos - 1;

        while (lua_next(ls_, real_pos) != 0) {
            T val = init_value_traits_t<T>::value();
            if (lua_op_t<T>::get_ret_value(ls_, -1, val) < 0) {
                return -1;
            }
            param_.insert(val);
            lua_pop(ls_, 1);
        }
        return 0;
    }

    static int lua_to_value(lua_State* ls_, int pos_, std::set<T>& param_) {
        luaL_checktype(ls_, pos_, LUA_TTABLE);

        lua_pushnil(ls_);
        int real_pos = pos_;
        if (pos_ < 0) real_pos = real_pos - 1;
        while (lua_next(ls_, real_pos) != 0) {
            T val = init_value_traits_t<T>::value();
            if (lua_op_t<T>::lua_to_value(ls_, -1, val) < 0) {
                luaL_argerror(ls_, pos_ > 0 ? pos_ : -pos_, "convert to vector failed");
            }
            param_.insert(val);
            lua_pop(ls_, 1);
        }
        return 0;
    }
};
template <typename K, typename V>
struct lua_op_t<std::map<K, V> > {
    static void push_stack(lua_State* ls_, const std::map<K, V>& arg_) {
        lua_newtable(ls_);
        typename map<K, V>::const_iterator it = arg_.begin();
        for (; it != arg_.end(); ++it) {
            lua_op_t<K>::push_stack(ls_, it->first);
            lua_op_t<V>::push_stack(ls_, it->second);
            lua_settable(ls_, -3);
        }
    }

    static int get_ret_value(lua_State* ls_, int pos_, std::map<K, V>& param_) {
        if (0 == lua_istable(ls_, pos_)) {
            return -1;
        }
        lua_pushnil(ls_);
        int real_pos = pos_;
        if (pos_ < 0) real_pos = real_pos - 1;

        while (lua_next(ls_, real_pos) != 0) {
            K key = init_value_traits_t<K>::value();
            V val = init_value_traits_t<V>::value();

            if (lua_op_t<K>::get_ret_value(ls_, -2, key) < 0 || lua_op_t<V>::get_ret_value(ls_, -1, val) < 0) {
                return -1;
            }
            param_.insert(make_pair(key, val));
            lua_pop(ls_, 1);
        }
        return 0;
    }

    static int lua_to_value(lua_State* ls_, int pos_, std::map<K, V>& param_) {
        luaL_checktype(ls_, pos_, LUA_TTABLE);

        lua_pushnil(ls_);
        int real_pos = pos_;
        if (pos_ < 0) real_pos = real_pos - 1;
        while (lua_next(ls_, real_pos) != 0) {
            K key = init_value_traits_t<K>::value();
            V val = init_value_traits_t<V>::value();
            if (lua_op_t<K>::get_ret_value(ls_, -2, key) < 0 || lua_op_t<V>::get_ret_value(ls_, -1, val) < 0) {
                luaL_argerror(ls_, pos_ > 0 ? pos_ : -pos_, "convert to vector failed");
            }
            param_.insert(make_pair(key, val));
            lua_pop(ls_, 1);
        }
        return 0;
    }
};

#define virtual_ctor int
#define void_ctor void
#define LUA_ARG_POS(x) (x)

typedef int (*mt_index_func_t)(lua_State*, void*, const char*);
typedef int (*mt_newindex_func_t)(lua_State*, void*, const char*, int);

#define TO_METATABLE_NAME(x) op_tool_t::to_metatable_name((x)).c_str()

struct op_tool_t {
    static std::string to_metatable_name(const std::string& name_) { return std::string("neko_lua.") + name_; }
};

template <typename T>
struct class_property_info_t {
    class_property_info_t() : property_pos(NULL) {}
    T property_pos;
};

//! 记录类中字段的指针
struct real_class_property_processor_t {
    real_class_property_processor_t() : index_impl_func(NULL), newindex_impl_func(NULL), property_pos(NULL) {}
    mt_index_func_t index_impl_func;
    mt_newindex_func_t newindex_impl_func;

    void* property_pos;
};

template <typename FUNC_TYPE>
struct userdata_for_function_t {
    userdata_for_function_t(FUNC_TYPE func_) : real_func(func_) {}
    FUNC_TYPE real_func;
};
template <typename PROPERTY_TYPE>
struct userdata_for_class_property_t : public real_class_property_processor_t {
    typedef class_property_info_t<PROPERTY_TYPE> real_class_property_info_t;
    real_class_property_info_t property_info;
};

//!  生成构造函数new,delete,index的实现类
template <typename CLASS_TYPE>
struct metatable_register_impl_t {
    static int mt_index_function(lua_State* ls_) {
        const char* key = luaL_checkstring(ls_, LUA_ARG_POS(2));

        luaL_getmetatable(ls_, lua_type_info_t<CLASS_TYPE>::get_name());
        int mt_index = lua_gettop(ls_);

        lua_getfield(ls_, -1, key);
        lua_remove(ls_, mt_index);

        //! 没有这个字段，查找基类
        if (lua_isnil(ls_, -1) && lua_type_info_t<CLASS_TYPE>::is_inherit()) {
            lua_pop(ls_, 1);
            luaL_getmetatable(ls_, lua_type_info_t<CLASS_TYPE>::get_inherit_name());
            mt_index = lua_gettop(ls_);
            lua_getfield(ls_, -1, key);
            lua_remove(ls_, mt_index);
        }

        if (lua_isuserdata(ls_, -1))  //! 获取属性
        {
            real_class_property_processor_t* p = (real_class_property_processor_t*)lua_touserdata(ls_, -1);
            lua_pop(ls_, 1);
            return (*(p->index_impl_func))(ls_, p->property_pos, key);
        } else {
            return 1;
        }
    }
    static int mt_newindex_function(lua_State* ls_) {
        const char* key = luaL_checkstring(ls_, LUA_ARG_POS(2));

        luaL_getmetatable(ls_, lua_type_info_t<CLASS_TYPE>::get_name());
        int mt_index = lua_gettop(ls_);

        lua_getfield(ls_, -1, key);
        lua_remove(ls_, mt_index);

        //! 没有这个字段，查找基类
        if (lua_isnil(ls_, -1) && lua_type_info_t<CLASS_TYPE>::is_inherit()) {
            lua_pop(ls_, 1);
            luaL_getmetatable(ls_, lua_type_info_t<CLASS_TYPE>::get_inherit_name());
            mt_index = lua_gettop(ls_);
            lua_getfield(ls_, -1, key);
            lua_remove(ls_, mt_index);
        }
        if (lua_isuserdata(ls_, -1)) {
            real_class_property_processor_t* p = (real_class_property_processor_t*)lua_touserdata(ls_, -1);
            lua_pop(ls_, 1);
            return (*(p->newindex_impl_func))(ls_, p->property_pos, key, LUA_ARG_POS(3));
        } else {
            return 1;
        }
    }

    static CLASS_TYPE** userdata_to_object_ptr_address(lua_State* ls_) {
        if (false == lua_type_info_t<CLASS_TYPE>::is_registed()) {
            luaL_argerror(ls_, 1, "arg 1 can't convert to class*, because the class has not registed to Lua");
        }

        void* arg_data = luaL_checkudata(ls_, 1, lua_type_info_t<CLASS_TYPE>::get_name());
        if (NULL == arg_data) {
            char buff[512];
            SPRINTF_F(buff, sizeof(buff), "`%s` expect arg 1, but arg == null", lua_type_info_t<CLASS_TYPE>::get_name());
            luaL_argerror(ls_, 1, buff);
        }

        CLASS_TYPE** ret_ptr = &(((userdata_for_object_t<CLASS_TYPE>*)arg_data)->obj);
        return ret_ptr;
    }
    static CLASS_TYPE* userdata_to_object(lua_State* ls_) {
        if (false == lua_type_info_t<CLASS_TYPE>::is_registed()) {
            luaL_argerror(ls_, 1, "arg 1 can't convert to class*, because the class has not registed to Lua");
        }
        void* arg_data = lua_touserdata(ls_, 1);
        if (NULL == arg_data) {
            luaL_argerror(ls_, 1, "arg 1 need userdsata(can't be null) param");
        }
        if (0 == lua_getmetatable(ls_, 1)) {
            luaL_argerror(ls_, 1, "arg 1 has no metatable, it is not cpp type");
        }

        luaL_getmetatable(ls_, lua_type_info_t<CLASS_TYPE>::get_name());
        if (0 == lua_rawequal(ls_, -1, -2)) {
            //! 查找基类
            lua_getfield(ls_, -2, INHERIT_TABLE);
            if (0 == lua_rawequal(ls_, -1, -2)) {
                lua_pop(ls_, 3);
                luaL_argerror(ls_, 1, "type convert failed");
            }
            lua_pop(ls_, 3);
        } else {
            lua_pop(ls_, 2);
        }
        CLASS_TYPE* ret_ptr = ((userdata_for_object_t<CLASS_TYPE>*)arg_data)->obj;
        if (NULL == ret_ptr) {
            char buff[128];
            SPRINTF_F(buff, sizeof(buff), "`%s` object ptr can't be null", lua_type_info_t<CLASS_TYPE>::get_name());
            luaL_argerror(ls_, 1, buff);
        }
        return ret_ptr;
    }
    static int get_pointer(lua_State* ls_) {
        CLASS_TYPE** obj_ptr = userdata_to_object_ptr_address(ls_);
        int64_t addr = int64_t(*obj_ptr);
        lua_op_t<int64_t>::push_stack(ls_, addr);
        return 1;
    }
};

template <typename CLASS_TYPE, typename FUNC_TYPE>
struct new_traits_t;

template <typename CLASS_TYPE>
struct delete_traits_t {
    static int lua_function(lua_State* ls_) {
        CLASS_TYPE** obj_ptr = metatable_register_impl_t<CLASS_TYPE>::userdata_to_object_ptr_address(ls_);

        delete *obj_ptr;
        *obj_ptr = NULL;
        return 0;
    }
};

template <typename FUNC_TYPE>
struct class_function_traits_t;
template <typename PROPERTY_TYPE, typename RET>
struct class_property_traits_t;
template <typename FUNC_TYPE>
struct function_traits_t;

//! CLASS_TYPE 为要注册的类型, CTOR_TYPE为构造函数类型
template <typename T>
struct neko_lua_register_router_t;
template <typename T>
struct neko_lua_register_router_t {
    template <typename REG_TYPE>
    static void call(REG_TYPE* reg_, T arg_, const std::string& s_) {
        reg_->def_class_property(arg_, s_);
    }
};
template <typename CLASS_TYPE = op_tool_t, typename CTOR_TYPE = void()>
class neko_lua_register_t {
public:
    neko_lua_register_t(lua_State* ls_) : m_ls(ls_) {}
    neko_lua_register_t(lua_State* ls_, const std::string& class_name_, std::string inherit_name_ = "");

    template <typename FUNC_TYPE>
    neko_lua_register_t& def(FUNC_TYPE func, const std::string& s_) {
        neko_lua_register_router_t<FUNC_TYPE>::call(this, func, s_);
        return *this;
    }
    template <typename FUNC_TYPE>
    neko_lua_register_t& def_class_func(FUNC_TYPE func_, const std::string& func_name_) {
        lua_function_t class_function = &class_function_traits_t<FUNC_TYPE>::lua_function;
        typedef typename class_function_traits_t<FUNC_TYPE>::userdata_for_function_info userdata_for_function_t;
        void* user_data_ptr = lua_newuserdata(m_ls, sizeof(userdata_for_function_t));
        new (user_data_ptr) userdata_for_function_t(func_);
        lua_pushcclosure(m_ls, class_function, 1);

        luaL_getmetatable(m_ls, TO_METATABLE_NAME(m_class_name));
        lua_pushstring(m_ls, func_name_.c_str());
        lua_pushvalue(m_ls, -3);
        lua_settable(m_ls, -3);

        lua_pop(m_ls, 2);
        return *this;
    }
    template <typename RET>
    neko_lua_register_t& def_class_property(RET CLASS_TYPE::*p_, const std::string& property_name_) {
        typedef typename class_property_traits_t<CLASS_TYPE, RET>::process_index_func_t process_index_func_t;
        typedef typename class_property_traits_t<CLASS_TYPE, RET>::process_newindex_func_t process_newindex_func_t;
        process_index_func_t process_index = &class_property_traits_t<CLASS_TYPE, RET>::process_index;
        process_newindex_func_t process_newindex = &class_property_traits_t<CLASS_TYPE, RET>::process_newindex;

        typedef userdata_for_class_property_t<RET CLASS_TYPE::*> udata_t;

        udata_t* pu = (udata_t*)lua_newuserdata(m_ls, sizeof(udata_t));
        pu->property_info.property_pos = p_;
        int udata_index = lua_gettop(m_ls);
        pu->index_impl_func = process_index;
        pu->newindex_impl_func = process_newindex;
        pu->property_pos = (void*)(&(pu->property_info));

        luaL_getmetatable(m_ls, lua_type_info_t<CLASS_TYPE>::get_name());
        lua_pushstring(m_ls, property_name_.c_str());
        lua_pushvalue(m_ls, udata_index);
        lua_settable(m_ls, -3);

        lua_pop(m_ls, 1);
        lua_remove(m_ls, udata_index);
        return *this;
    }
    template <typename FUNC>
    neko_lua_register_t& def_func(FUNC func_, const std::string& func_name_) {
        if (m_class_name.empty()) {
            lua_function_t lua_func = function_traits_t<FUNC>::lua_function;

            void* user_data_ptr = lua_newuserdata(m_ls, sizeof(func_));
            new (user_data_ptr) FUNC(func_);

            lua_pushcclosure(m_ls, lua_func, 1);
            lua_setglobal(m_ls, func_name_.c_str());
        } else {
            lua_function_t lua_func = function_traits_t<FUNC>::lua_function;

            void* user_data_ptr = lua_newuserdata(m_ls, sizeof(func_));
            new (user_data_ptr) FUNC(func_);

            lua_pushcclosure(m_ls, lua_func, 1);
            // lua_setglobal(m_ls, func_name_.c_str());

            lua_getglobal(m_ls, (m_class_name).c_str());
            lua_pushstring(m_ls, func_name_.c_str());
            lua_pushvalue(m_ls, -3);
            lua_settable(m_ls, -3);

            lua_pop(m_ls, 2);
        }
        return *this;
    }

private:
    lua_State* m_ls;
    std::string m_class_name;
};

template <typename CLASS_TYPE, typename CTOR_TYPE>
neko_lua_register_t<CLASS_TYPE, CTOR_TYPE>::neko_lua_register_t(lua_State* ls_, const std::string& class_name_, std::string inherit_name_) : m_ls(ls_), m_class_name(class_name_) {
    lua_type_info_t<CLASS_TYPE>::set_name(TO_METATABLE_NAME(class_name_), TO_METATABLE_NAME(inherit_name_));

    luaL_newmetatable(ls_, TO_METATABLE_NAME(class_name_));
    int metatable_index = lua_gettop(ls_);
    if (false == inherit_name_.empty())  //! 设置基类
    {
        luaL_getmetatable(ls_, TO_METATABLE_NAME(inherit_name_));
        if (lua_istable(ls_, -1)) {
            lua_setfield(ls_, metatable_index, INHERIT_TABLE);
        } else {
            lua_pop(ls_, 1);
        }
    }
    lua_pushstring(ls_, "__index");
    lua_function_t index_function = &metatable_register_impl_t<CLASS_TYPE>::mt_index_function;
    lua_pushcclosure(ls_, index_function, 0);
    lua_settable(ls_, -3);

    lua_pushstring(ls_, "get_pointer");
    lua_function_t pointer_function = &metatable_register_impl_t<CLASS_TYPE>::get_pointer;
    lua_pushcclosure(ls_, pointer_function, 0);
    lua_settable(ls_, -3);

    lua_pushstring(ls_, "__newindex");
    lua_function_t newindex_function = &metatable_register_impl_t<CLASS_TYPE>::mt_newindex_function;
    lua_pushcclosure(ls_, newindex_function, 0);
    lua_settable(ls_, -3);

    lua_function_t function_for_new = &new_traits_t<CLASS_TYPE, CTOR_TYPE>::lua_function;
    lua_pushcclosure(ls_, function_for_new, 0);

    lua_newtable(ls_);
    lua_pushstring(ls_, "new");
    lua_pushvalue(ls_, -3);
    lua_settable(ls_, -3);

    lua_setglobal(ls_, class_name_.c_str());

    lua_pop(ls_, 1);

    lua_function_t function_for_delete = &delete_traits_t<CLASS_TYPE>::lua_function;
    lua_pushcclosure(ls_, function_for_delete, 0);

    lua_pushstring(ls_, "delete");
    lua_pushvalue(ls_, -2);
    lua_settable(ls_, metatable_index);

    lua_pop(ls_, 2);
}

template <typename CLASS_TYPE>
struct new_traits_t<CLASS_TYPE, int()> {
    static int lua_function(lua_State* ls_) { return 0; }
};

template <typename CLASS_TYPE>
struct new_traits_t<CLASS_TYPE, void()> {
    static int lua_function(lua_State* ls_) {
        void* user_data_ptr = lua_newuserdata(ls_, sizeof(userdata_for_object_t<CLASS_TYPE>));
        luaL_getmetatable(ls_, lua_type_info_t<CLASS_TYPE>::get_name());
        lua_setmetatable(ls_, -2);

        new (user_data_ptr) userdata_for_object_t<CLASS_TYPE>(new CLASS_TYPE());
        return 1;
    }
};

template <typename CLASS_TYPE, typename ARG1>
struct new_traits_t<CLASS_TYPE, void(ARG1)> {
    static int lua_function(lua_State* ls_) {
        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg1);

        void* user_data_ptr = lua_newuserdata(ls_, sizeof(userdata_for_object_t<CLASS_TYPE>));
        luaL_getmetatable(ls_, lua_type_info_t<CLASS_TYPE>::get_name());
        lua_setmetatable(ls_, -2);

        new (user_data_ptr) userdata_for_object_t<CLASS_TYPE>(new CLASS_TYPE(arg1));
        return 1;
    }
};

template <typename CLASS_TYPE, typename ARG1, typename ARG2>
struct new_traits_t<CLASS_TYPE, void(ARG1, ARG2)> {
    static int lua_function(lua_State* ls_) {
        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG2>::arg_type_t arg2 = init_value_traits_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::value();
        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(3), arg2);

        void* user_data_ptr = lua_newuserdata(ls_, sizeof(userdata_for_object_t<CLASS_TYPE>));
        luaL_getmetatable(ls_, lua_type_info_t<CLASS_TYPE>::get_name());
        lua_setmetatable(ls_, -2);

        new (user_data_ptr) userdata_for_object_t<CLASS_TYPE>(new CLASS_TYPE(arg1, arg2));
        return 1;
    }
};

template <typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3>
struct new_traits_t<CLASS_TYPE, void(ARG1, ARG2, ARG3)> {
    static int lua_function(lua_State* ls_) {
        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG2>::arg_type_t arg2 = init_value_traits_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG3>::arg_type_t arg3 = init_value_traits_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::value();
        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(4), arg3);

        void* user_data_ptr = lua_newuserdata(ls_, sizeof(userdata_for_object_t<CLASS_TYPE>));

        luaL_getmetatable(ls_, lua_type_info_t<CLASS_TYPE>::get_name());
        lua_setmetatable(ls_, -2);

        new (user_data_ptr) userdata_for_object_t<CLASS_TYPE>(new CLASS_TYPE(arg1, arg2, arg3));
        return 1;
    }
};

template <typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4>
struct new_traits_t<CLASS_TYPE, void(ARG1, ARG2, ARG3, ARG4)> {
    static int lua_function(lua_State* ls_) {
        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG2>::arg_type_t arg2 = init_value_traits_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG3>::arg_type_t arg3 = init_value_traits_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG4>::arg_type_t arg4 = init_value_traits_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::value();
        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(5), arg4);

        void* user_data_ptr = lua_newuserdata(ls_, sizeof(userdata_for_object_t<CLASS_TYPE>));
        luaL_getmetatable(ls_, lua_type_info_t<CLASS_TYPE>::get_name());
        lua_setmetatable(ls_, -2);

        new (user_data_ptr) userdata_for_object_t<CLASS_TYPE>(new CLASS_TYPE(arg1, arg2, arg3, arg4));
        return 1;
    }
};

template <typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5>
struct new_traits_t<CLASS_TYPE, void(ARG1, ARG2, ARG3, ARG4, ARG5)> {
    static int lua_function(lua_State* ls_) {
        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG2>::arg_type_t arg2 = init_value_traits_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG3>::arg_type_t arg3 = init_value_traits_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG4>::arg_type_t arg4 = init_value_traits_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG5>::arg_type_t arg5 = init_value_traits_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::value();
        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(5), arg4);
        lua_op_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(6), arg5);

        void* user_data_ptr = lua_newuserdata(ls_, sizeof(userdata_for_object_t<CLASS_TYPE>));
        luaL_getmetatable(ls_, lua_type_info_t<CLASS_TYPE>::get_name());
        lua_setmetatable(ls_, -2);

        new (user_data_ptr) userdata_for_object_t<CLASS_TYPE>(new CLASS_TYPE(arg1, arg2, arg3, arg4, arg5));
        return 1;
    }
};

template <typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6>
struct new_traits_t<CLASS_TYPE, void(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6)> {
    static int lua_function(lua_State* ls_) {
        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG2>::arg_type_t arg2 = init_value_traits_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG3>::arg_type_t arg3 = init_value_traits_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG4>::arg_type_t arg4 = init_value_traits_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG5>::arg_type_t arg5 = init_value_traits_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG6>::arg_type_t arg6 = init_value_traits_t<typename basetype_ptr_traits_t<ARG6>::arg_type_t>::value();
        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(5), arg4);
        lua_op_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(6), arg5);
        lua_op_t<typename basetype_ptr_traits_t<ARG6>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(7), arg6);

        void* user_data_ptr = lua_newuserdata(ls_, sizeof(userdata_for_object_t<CLASS_TYPE>));
        luaL_getmetatable(ls_, lua_type_info_t<CLASS_TYPE>::get_name());
        lua_setmetatable(ls_, -2);

        new (user_data_ptr) userdata_for_object_t<CLASS_TYPE>(new CLASS_TYPE(arg1, arg2, arg3, arg4, arg5, arg6));
        return 1;
    }
};

template <typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7>
struct new_traits_t<CLASS_TYPE, void(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7)> {
    static int lua_function(lua_State* ls_) {
        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG2>::arg_type_t arg2 = init_value_traits_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG3>::arg_type_t arg3 = init_value_traits_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG4>::arg_type_t arg4 = init_value_traits_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG5>::arg_type_t arg5 = init_value_traits_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG6>::arg_type_t arg6 = init_value_traits_t<typename basetype_ptr_traits_t<ARG6>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG7>::arg_type_t arg7 = init_value_traits_t<typename basetype_ptr_traits_t<ARG7>::arg_type_t>::value();
        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(5), arg4);
        lua_op_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(6), arg5);
        lua_op_t<typename basetype_ptr_traits_t<ARG6>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(7), arg6);
        lua_op_t<typename basetype_ptr_traits_t<ARG7>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(8), arg7);

        void* user_data_ptr = lua_newuserdata(ls_, sizeof(userdata_for_object_t<CLASS_TYPE>));

        luaL_getmetatable(ls_, lua_type_info_t<CLASS_TYPE>::get_name());
        lua_setmetatable(ls_, -2);

        new (user_data_ptr) userdata_for_object_t<CLASS_TYPE>(new CLASS_TYPE(arg1, arg2, arg3, arg4, arg5, arg6, arg7));
        return 1;
    }
};

template <typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7, typename ARG8>
struct new_traits_t<CLASS_TYPE, void(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8)> {
    static int lua_function(lua_State* ls_) {
        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG2>::arg_type_t arg2 = init_value_traits_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG3>::arg_type_t arg3 = init_value_traits_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG4>::arg_type_t arg4 = init_value_traits_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG5>::arg_type_t arg5 = init_value_traits_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG6>::arg_type_t arg6 = init_value_traits_t<typename basetype_ptr_traits_t<ARG6>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG7>::arg_type_t arg7 = init_value_traits_t<typename basetype_ptr_traits_t<ARG7>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG8>::arg_type_t arg8 = init_value_traits_t<typename basetype_ptr_traits_t<ARG8>::arg_type_t>::value();
        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(5), arg4);
        lua_op_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(6), arg5);
        lua_op_t<typename basetype_ptr_traits_t<ARG6>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(7), arg6);
        lua_op_t<typename basetype_ptr_traits_t<ARG7>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(8), arg7);
        lua_op_t<typename basetype_ptr_traits_t<ARG8>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(9), arg8);

        void* user_data_ptr = lua_newuserdata(ls_, sizeof(userdata_for_object_t<CLASS_TYPE>));
        luaL_getmetatable(ls_, lua_type_info_t<CLASS_TYPE>::get_name());
        lua_setmetatable(ls_, -2);

        new (user_data_ptr) userdata_for_object_t<CLASS_TYPE>(new CLASS_TYPE(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8));
        return 1;
    }
};

template <typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7, typename ARG8, typename ARG9>
struct new_traits_t<CLASS_TYPE, void(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9)> {
    static int lua_function(lua_State* ls_) {
        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG2>::arg_type_t arg2 = init_value_traits_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG3>::arg_type_t arg3 = init_value_traits_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG4>::arg_type_t arg4 = init_value_traits_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG5>::arg_type_t arg5 = init_value_traits_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG6>::arg_type_t arg6 = init_value_traits_t<typename basetype_ptr_traits_t<ARG6>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG7>::arg_type_t arg7 = init_value_traits_t<typename basetype_ptr_traits_t<ARG7>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG8>::arg_type_t arg8 = init_value_traits_t<typename basetype_ptr_traits_t<ARG8>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG9>::arg_type_t arg9 = init_value_traits_t<typename basetype_ptr_traits_t<ARG9>::arg_type_t>::value();
        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(5), arg4);
        lua_op_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(6), arg5);
        lua_op_t<typename basetype_ptr_traits_t<ARG6>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(7), arg6);
        lua_op_t<typename basetype_ptr_traits_t<ARG7>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(8), arg7);
        lua_op_t<typename basetype_ptr_traits_t<ARG8>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(9), arg8);
        lua_op_t<typename basetype_ptr_traits_t<ARG9>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(10), arg9);

        void* user_data_ptr = lua_newuserdata(ls_, sizeof(userdata_for_object_t<CLASS_TYPE>));
        luaL_getmetatable(ls_, lua_type_info_t<CLASS_TYPE>::get_name());
        lua_setmetatable(ls_, -2);

        new (user_data_ptr) userdata_for_object_t<CLASS_TYPE>(new CLASS_TYPE(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9));

        return 1;
    }
};

template <typename FUNC_CLASS_TYPE>
struct class_function_traits_t<void (FUNC_CLASS_TYPE::*)()> {
    typedef void (FUNC_CLASS_TYPE::*dest_func_t)();
    typedef userdata_for_function_t<dest_func_t> userdata_for_function_info;

    static int lua_function(lua_State* ls_) {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);
        (obj_ptr->*(registed_data.real_func))();
        return 0;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1>
struct class_function_traits_t<void (FUNC_CLASS_TYPE::*)(ARG1)> {
    typedef void (FUNC_CLASS_TYPE::*dest_func_t)(ARG1);
    typedef userdata_for_function_t<dest_func_t> userdata_for_function_info;

    static int lua_function(lua_State* ls_) {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg1);

        (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1));
        return 0;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2>
struct class_function_traits_t<void (FUNC_CLASS_TYPE::*)(ARG1, ARG2)> {
    typedef void (FUNC_CLASS_TYPE::*dest_func_t)(ARG1, ARG2);
    typedef userdata_for_function_t<dest_func_t> userdata_for_function_info;

    static int lua_function(lua_State* ls_) {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG2>::arg_type_t arg2 = init_value_traits_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::value();
        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(3), arg2);

        (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2));
        return 0;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3>
struct class_function_traits_t<void (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3)> {
    typedef void (FUNC_CLASS_TYPE::*dest_func_t)(ARG1, ARG2, ARG3);
    typedef userdata_for_function_t<dest_func_t> userdata_for_function_info;

    static int lua_function(lua_State* ls_) {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG2>::arg_type_t arg2 = init_value_traits_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG3>::arg_type_t arg3 = init_value_traits_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::value();
        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(4), arg3);
        (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2), p_t<ARG3>::r(arg3));
        return 0;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4>
struct class_function_traits_t<void (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4)> {
    typedef void (FUNC_CLASS_TYPE::*dest_func_t)(ARG1, ARG2, ARG3, ARG4);
    typedef userdata_for_function_t<dest_func_t> userdata_for_function_info;

    static int lua_function(lua_State* ls_) {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG2>::arg_type_t arg2 = init_value_traits_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG3>::arg_type_t arg3 = init_value_traits_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG4>::arg_type_t arg4 = init_value_traits_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::value();
        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(5), arg4);

        (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2), p_t<ARG3>::r(arg3), p_t<ARG4>::r(arg4));
        return 0;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5>
struct class_function_traits_t<void (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5)> {
    typedef void (FUNC_CLASS_TYPE::*dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5);
    typedef userdata_for_function_t<dest_func_t> userdata_for_function_info;

    static int lua_function(lua_State* ls_) {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG2>::arg_type_t arg2 = init_value_traits_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG3>::arg_type_t arg3 = init_value_traits_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG4>::arg_type_t arg4 = init_value_traits_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG5>::arg_type_t arg5 = init_value_traits_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::value();

        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(5), arg4);
        lua_op_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(6), arg5);

        (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2), p_t<ARG3>::r(arg3), p_t<ARG4>::r(arg4), p_t<ARG5>::r(arg5));
        return 0;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6>
struct class_function_traits_t<void (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6)> {
    typedef void (FUNC_CLASS_TYPE::*dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6);
    typedef userdata_for_function_t<dest_func_t> userdata_for_function_info;

    static int lua_function(lua_State* ls_) {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG2>::arg_type_t arg2 = init_value_traits_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG3>::arg_type_t arg3 = init_value_traits_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG4>::arg_type_t arg4 = init_value_traits_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG5>::arg_type_t arg5 = init_value_traits_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG6>::arg_type_t arg6 = init_value_traits_t<typename basetype_ptr_traits_t<ARG6>::arg_type_t>::value();

        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(5), arg4);
        lua_op_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(6), arg5);
        lua_op_t<typename basetype_ptr_traits_t<ARG6>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(7), arg6);

        (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2), p_t<ARG3>::r(arg3), p_t<ARG4>::r(arg4), p_t<ARG5>::r(arg5), p_t<ARG6>::r(arg6));
        return 0;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7>
struct class_function_traits_t<void (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7)> {
    typedef void (FUNC_CLASS_TYPE::*dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7);
    typedef userdata_for_function_t<dest_func_t> userdata_for_function_info;

    static int lua_function(lua_State* ls_) {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG2>::arg_type_t arg2 = init_value_traits_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG3>::arg_type_t arg3 = init_value_traits_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG4>::arg_type_t arg4 = init_value_traits_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG5>::arg_type_t arg5 = init_value_traits_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG6>::arg_type_t arg6 = init_value_traits_t<typename basetype_ptr_traits_t<ARG6>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG7>::arg_type_t arg7 = init_value_traits_t<typename basetype_ptr_traits_t<ARG7>::arg_type_t>::value();

        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(5), arg4);
        lua_op_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(6), arg5);
        lua_op_t<typename basetype_ptr_traits_t<ARG6>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(7), arg6);
        lua_op_t<typename basetype_ptr_traits_t<ARG7>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(8), arg7);

        (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2), p_t<ARG3>::r(arg3), p_t<ARG4>::r(arg4), p_t<ARG5>::r(arg5), p_t<ARG6>::r(arg6), p_t<ARG7>::r(arg7));
        return 0;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7, typename ARG8>
struct class_function_traits_t<void (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8)> {
    typedef void (FUNC_CLASS_TYPE::*dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8);
    typedef userdata_for_function_t<dest_func_t> userdata_for_function_info;

    static int lua_function(lua_State* ls_) {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG2>::arg_type_t arg2 = init_value_traits_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG3>::arg_type_t arg3 = init_value_traits_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG4>::arg_type_t arg4 = init_value_traits_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG5>::arg_type_t arg5 = init_value_traits_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG6>::arg_type_t arg6 = init_value_traits_t<typename basetype_ptr_traits_t<ARG6>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG7>::arg_type_t arg7 = init_value_traits_t<typename basetype_ptr_traits_t<ARG7>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG8>::arg_type_t arg8 = init_value_traits_t<typename basetype_ptr_traits_t<ARG8>::arg_type_t>::value();

        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(5), arg4);
        lua_op_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(6), arg5);
        lua_op_t<typename basetype_ptr_traits_t<ARG6>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(7), arg6);
        lua_op_t<typename basetype_ptr_traits_t<ARG7>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(8), arg7);
        lua_op_t<typename basetype_ptr_traits_t<ARG8>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(9), arg8);

        (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2), p_t<ARG3>::r(arg3), p_t<ARG4>::r(arg4), p_t<ARG5>::r(arg5), p_t<ARG6>::r(arg6), p_t<ARG7>::r(arg7),
                                              p_t<ARG8>::r(arg8));
        return 0;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7, typename ARG8, typename ARG9>
struct class_function_traits_t<void (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9)> {
    typedef void (FUNC_CLASS_TYPE::*dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9);
    typedef userdata_for_function_t<dest_func_t> userdata_for_function_info;

    static int lua_function(lua_State* ls_) {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG2>::arg_type_t arg2 = init_value_traits_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG3>::arg_type_t arg3 = init_value_traits_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG4>::arg_type_t arg4 = init_value_traits_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG5>::arg_type_t arg5 = init_value_traits_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG6>::arg_type_t arg6 = init_value_traits_t<typename basetype_ptr_traits_t<ARG6>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG7>::arg_type_t arg7 = init_value_traits_t<typename basetype_ptr_traits_t<ARG7>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG8>::arg_type_t arg8 = init_value_traits_t<typename basetype_ptr_traits_t<ARG8>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG9>::arg_type_t arg9 = init_value_traits_t<typename basetype_ptr_traits_t<ARG9>::arg_type_t>::value();

        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(5), arg4);
        lua_op_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(6), arg5);
        lua_op_t<typename basetype_ptr_traits_t<ARG6>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(7), arg6);
        lua_op_t<typename basetype_ptr_traits_t<ARG7>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(8), arg7);
        lua_op_t<typename basetype_ptr_traits_t<ARG8>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(9), arg8);
        lua_op_t<typename basetype_ptr_traits_t<ARG9>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(10), arg9);

        (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2), p_t<ARG3>::r(arg3), p_t<ARG4>::r(arg4), p_t<ARG5>::r(arg5), p_t<ARG6>::r(arg6), p_t<ARG7>::r(arg7),
                                              p_t<ARG8>::r(arg8), p_t<ARG9>::r(arg9));
        return 0;
    }
};

template <typename FUNC_CLASS_TYPE>
struct class_function_traits_t<void (FUNC_CLASS_TYPE::*)() const> {
    typedef void (FUNC_CLASS_TYPE::*dest_func_t)() const;
    typedef userdata_for_function_t<dest_func_t> userdata_for_function_info;

    static int lua_function(lua_State* ls_) {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);
        (obj_ptr->*(registed_data.real_func))();
        return 0;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1>
struct class_function_traits_t<void (FUNC_CLASS_TYPE::*)(ARG1) const> {
    typedef void (FUNC_CLASS_TYPE::*dest_func_t)(ARG1) const;
    typedef userdata_for_function_t<dest_func_t> userdata_for_function_info;

    static int lua_function(lua_State* ls_) {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg1);

        (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1));
        return 0;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2>
struct class_function_traits_t<void (FUNC_CLASS_TYPE::*)(ARG1, ARG2) const> {
    typedef void (FUNC_CLASS_TYPE::*dest_func_t)(ARG1, ARG2) const;
    typedef userdata_for_function_t<dest_func_t> userdata_for_function_info;

    static int lua_function(lua_State* ls_) {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG2>::arg_type_t arg2 = init_value_traits_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::value();
        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(3), arg2);

        (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2));
        return 0;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3>
struct class_function_traits_t<void (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3) const> {
    typedef void (FUNC_CLASS_TYPE::*dest_func_t)(ARG1, ARG2, ARG3) const;
    typedef userdata_for_function_t<dest_func_t> userdata_for_function_info;

    static int lua_function(lua_State* ls_) {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG2>::arg_type_t arg2 = init_value_traits_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG3>::arg_type_t arg3 = init_value_traits_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::value();
        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(4), arg3);
        (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2), p_t<ARG3>::r(arg3));
        return 0;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4>
struct class_function_traits_t<void (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4) const> {
    typedef void (FUNC_CLASS_TYPE::*dest_func_t)(ARG1, ARG2, ARG3, ARG4) const;
    typedef userdata_for_function_t<dest_func_t> userdata_for_function_info;

    static int lua_function(lua_State* ls_) {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG2>::arg_type_t arg2 = init_value_traits_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG3>::arg_type_t arg3 = init_value_traits_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG4>::arg_type_t arg4 = init_value_traits_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::value();
        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(5), arg4);

        (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2), p_t<ARG3>::r(arg3), p_t<ARG4>::r(arg4));
        return 0;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5>
struct class_function_traits_t<void (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5) const> {
    typedef void (FUNC_CLASS_TYPE::*dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5) const;
    typedef userdata_for_function_t<dest_func_t> userdata_for_function_info;

    static int lua_function(lua_State* ls_) {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG2>::arg_type_t arg2 = init_value_traits_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG3>::arg_type_t arg3 = init_value_traits_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG4>::arg_type_t arg4 = init_value_traits_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG5>::arg_type_t arg5 = init_value_traits_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::value();

        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(5), arg4);
        lua_op_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(6), arg5);

        (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2), p_t<ARG3>::r(arg3), p_t<ARG4>::r(arg4), p_t<ARG5>::r(arg5));
        return 0;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6>
struct class_function_traits_t<void (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6) const> {
    typedef void (FUNC_CLASS_TYPE::*dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6) const;
    typedef userdata_for_function_t<dest_func_t> userdata_for_function_info;

    static int lua_function(lua_State* ls_) {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG2>::arg_type_t arg2 = init_value_traits_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG3>::arg_type_t arg3 = init_value_traits_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG4>::arg_type_t arg4 = init_value_traits_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG5>::arg_type_t arg5 = init_value_traits_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG6>::arg_type_t arg6 = init_value_traits_t<typename basetype_ptr_traits_t<ARG6>::arg_type_t>::value();

        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(5), arg4);
        lua_op_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(6), arg5);
        lua_op_t<typename basetype_ptr_traits_t<ARG6>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(7), arg6);

        (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2), p_t<ARG3>::r(arg3), p_t<ARG4>::r(arg4), p_t<ARG5>::r(arg5), p_t<ARG6>::r(arg6));
        return 0;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7>
struct class_function_traits_t<void (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7) const> {
    typedef void (FUNC_CLASS_TYPE::*dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7) const;
    typedef userdata_for_function_t<dest_func_t> userdata_for_function_info;

    static int lua_function(lua_State* ls_) {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG2>::arg_type_t arg2 = init_value_traits_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG3>::arg_type_t arg3 = init_value_traits_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG4>::arg_type_t arg4 = init_value_traits_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG5>::arg_type_t arg5 = init_value_traits_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG6>::arg_type_t arg6 = init_value_traits_t<typename basetype_ptr_traits_t<ARG6>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG7>::arg_type_t arg7 = init_value_traits_t<typename basetype_ptr_traits_t<ARG7>::arg_type_t>::value();

        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(5), arg4);
        lua_op_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(6), arg5);
        lua_op_t<typename basetype_ptr_traits_t<ARG6>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(7), arg6);
        lua_op_t<typename basetype_ptr_traits_t<ARG7>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(8), arg7);

        (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2), p_t<ARG3>::r(arg3), p_t<ARG4>::r(arg4), p_t<ARG5>::r(arg5), p_t<ARG6>::r(arg6), p_t<ARG7>::r(arg7));
        return 0;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7, typename ARG8>
struct class_function_traits_t<void (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8) const> {
    typedef void (FUNC_CLASS_TYPE::*dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8) const;
    typedef userdata_for_function_t<dest_func_t> userdata_for_function_info;

    static int lua_function(lua_State* ls_) {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG2>::arg_type_t arg2 = init_value_traits_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG3>::arg_type_t arg3 = init_value_traits_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG4>::arg_type_t arg4 = init_value_traits_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG5>::arg_type_t arg5 = init_value_traits_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG6>::arg_type_t arg6 = init_value_traits_t<typename basetype_ptr_traits_t<ARG6>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG7>::arg_type_t arg7 = init_value_traits_t<typename basetype_ptr_traits_t<ARG7>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG8>::arg_type_t arg8 = init_value_traits_t<typename basetype_ptr_traits_t<ARG8>::arg_type_t>::value();

        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(5), arg4);
        lua_op_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(6), arg5);
        lua_op_t<typename basetype_ptr_traits_t<ARG6>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(7), arg6);
        lua_op_t<typename basetype_ptr_traits_t<ARG7>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(8), arg7);
        lua_op_t<typename basetype_ptr_traits_t<ARG8>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(9), arg8);

        (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2), p_t<ARG3>::r(arg3), p_t<ARG4>::r(arg4), p_t<ARG5>::r(arg5), p_t<ARG6>::r(arg6), p_t<ARG7>::r(arg7),
                                              p_t<ARG8>::r(arg8));
        return 0;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7, typename ARG8, typename ARG9>
struct class_function_traits_t<void (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9) const> {
    typedef void (FUNC_CLASS_TYPE::*dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9) const;
    typedef userdata_for_function_t<dest_func_t> userdata_for_function_info;

    static int lua_function(lua_State* ls_) {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG2>::arg_type_t arg2 = init_value_traits_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG3>::arg_type_t arg3 = init_value_traits_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG4>::arg_type_t arg4 = init_value_traits_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG5>::arg_type_t arg5 = init_value_traits_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG6>::arg_type_t arg6 = init_value_traits_t<typename basetype_ptr_traits_t<ARG6>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG7>::arg_type_t arg7 = init_value_traits_t<typename basetype_ptr_traits_t<ARG7>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG8>::arg_type_t arg8 = init_value_traits_t<typename basetype_ptr_traits_t<ARG8>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG9>::arg_type_t arg9 = init_value_traits_t<typename basetype_ptr_traits_t<ARG9>::arg_type_t>::value();

        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(5), arg4);
        lua_op_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(6), arg5);
        lua_op_t<typename basetype_ptr_traits_t<ARG6>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(7), arg6);
        lua_op_t<typename basetype_ptr_traits_t<ARG7>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(8), arg7);
        lua_op_t<typename basetype_ptr_traits_t<ARG8>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(9), arg8);
        lua_op_t<typename basetype_ptr_traits_t<ARG9>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(10), arg9);

        (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2), p_t<ARG3>::r(arg3), p_t<ARG4>::r(arg4), p_t<ARG5>::r(arg5), p_t<ARG6>::r(arg6), p_t<ARG7>::r(arg7),
                                              p_t<ARG8>::r(arg8), p_t<ARG9>::r(arg9));
        return 0;
    }
};

template <typename FUNC_CLASS_TYPE, typename RET>
struct class_function_traits_t<RET (FUNC_CLASS_TYPE::*)()> {
    typedef RET (FUNC_CLASS_TYPE::*dest_func_t)();
    typedef userdata_for_function_t<dest_func_t> userdata_for_function_info;

    static int lua_function(lua_State* ls_) {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);
        RET ret = (obj_ptr->*(registed_data.real_func))();
        lua_op_t<typename basetype_ptr_traits_t<RET>::arg_type_t>::push_stack(ls_, ret);
        return 1;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename RET>
struct class_function_traits_t<RET (FUNC_CLASS_TYPE::*)(ARG1)> {
    typedef RET (FUNC_CLASS_TYPE::*dest_func_t)(ARG1);
    typedef userdata_for_function_t<dest_func_t> userdata_for_function_info;

    static int lua_function(lua_State* ls_) {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg1);

        RET ret = (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1));
        lua_op_t<typename basetype_ptr_traits_t<RET>::arg_type_t>::push_stack(ls_, ret);
        return 1;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename RET>
struct class_function_traits_t<RET (FUNC_CLASS_TYPE::*)(ARG1, ARG2)> {
    typedef RET (FUNC_CLASS_TYPE::*dest_func_t)(ARG1, ARG2);
    typedef userdata_for_function_t<dest_func_t> userdata_for_function_info;

    static int lua_function(lua_State* ls_) {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG2>::arg_type_t arg2 = init_value_traits_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::value();
        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(3), arg2);

        RET ret = (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2));
        lua_op_t<typename basetype_ptr_traits_t<RET>::arg_type_t>::push_stack(ls_, ret);
        return 1;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename RET>
struct class_function_traits_t<RET (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3)> {
    typedef RET (FUNC_CLASS_TYPE::*dest_func_t)(ARG1, ARG2, ARG3);
    typedef userdata_for_function_t<dest_func_t> userdata_for_function_info;

    static int lua_function(lua_State* ls_) {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG2>::arg_type_t arg2 = init_value_traits_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG3>::arg_type_t arg3 = init_value_traits_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::value();
        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(4), arg3);

        RET ret = (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2), p_t<ARG3>::r(arg3));
        lua_op_t<typename basetype_ptr_traits_t<RET>::arg_type_t>::push_stack(ls_, ret);
        return 1;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename RET>
struct class_function_traits_t<RET (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4)> {
    typedef RET (FUNC_CLASS_TYPE::*dest_func_t)(ARG1, ARG2, ARG3, ARG4);
    typedef userdata_for_function_t<dest_func_t> userdata_for_function_info;

    static int lua_function(lua_State* ls_) {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG2>::arg_type_t arg2 = init_value_traits_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG3>::arg_type_t arg3 = init_value_traits_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG4>::arg_type_t arg4 = init_value_traits_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::value();
        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(5), arg4);

        RET ret = (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2), p_t<ARG3>::r(arg3), p_t<ARG4>::r(arg4));
        lua_op_t<typename basetype_ptr_traits_t<RET>::arg_type_t>::push_stack(ls_, ret);
        return 1;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename RET>
struct class_function_traits_t<RET (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5)> {
    typedef RET (FUNC_CLASS_TYPE::*dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5);
    typedef userdata_for_function_t<dest_func_t> userdata_for_function_info;

    static int lua_function(lua_State* ls_) {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG2>::arg_type_t arg2 = init_value_traits_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG3>::arg_type_t arg3 = init_value_traits_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG4>::arg_type_t arg4 = init_value_traits_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG5>::arg_type_t arg5 = init_value_traits_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::value();

        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(5), arg4);
        lua_op_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(6), arg5);

        RET ret = (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2), p_t<ARG3>::r(arg3), p_t<ARG4>::r(arg4), p_t<ARG5>::r(arg5));
        lua_op_t<typename basetype_ptr_traits_t<RET>::arg_type_t>::push_stack(ls_, ret);
        return 1;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename RET>
struct class_function_traits_t<RET (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6)> {
    typedef RET (FUNC_CLASS_TYPE::*dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6);
    typedef userdata_for_function_t<dest_func_t> userdata_for_function_info;

    static int lua_function(lua_State* ls_) {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG2>::arg_type_t arg2 = init_value_traits_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG3>::arg_type_t arg3 = init_value_traits_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG4>::arg_type_t arg4 = init_value_traits_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG5>::arg_type_t arg5 = init_value_traits_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG6>::arg_type_t arg6 = init_value_traits_t<typename basetype_ptr_traits_t<ARG6>::arg_type_t>::value();

        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(5), arg4);
        lua_op_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(6), arg5);
        lua_op_t<typename basetype_ptr_traits_t<ARG6>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(7), arg6);

        RET ret = (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2), p_t<ARG3>::r(arg3), p_t<ARG4>::r(arg4), p_t<ARG5>::r(arg5), p_t<ARG6>::r(arg6));
        lua_op_t<typename basetype_ptr_traits_t<RET>::arg_type_t>::push_stack(ls_, ret);
        return 1;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7, typename RET>
struct class_function_traits_t<RET (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7)> {
    typedef RET (FUNC_CLASS_TYPE::*dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7);
    typedef userdata_for_function_t<dest_func_t> userdata_for_function_info;

    static int lua_function(lua_State* ls_) {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG2>::arg_type_t arg2 = init_value_traits_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG3>::arg_type_t arg3 = init_value_traits_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG4>::arg_type_t arg4 = init_value_traits_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG5>::arg_type_t arg5 = init_value_traits_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG6>::arg_type_t arg6 = init_value_traits_t<typename basetype_ptr_traits_t<ARG6>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG7>::arg_type_t arg7 = init_value_traits_t<typename basetype_ptr_traits_t<ARG7>::arg_type_t>::value();

        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(5), arg4);
        lua_op_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(6), arg5);
        lua_op_t<typename basetype_ptr_traits_t<ARG6>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(7), arg6);
        lua_op_t<typename basetype_ptr_traits_t<ARG7>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(8), arg7);

        RET ret = (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2), p_t<ARG3>::r(arg3), p_t<ARG4>::r(arg4), p_t<ARG5>::r(arg5), p_t<ARG6>::r(arg6), p_t<ARG7>::r(arg7));
        lua_op_t<typename basetype_ptr_traits_t<RET>::arg_type_t>::push_stack(ls_, ret);
        return 1;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7, typename ARG8, typename RET>
struct class_function_traits_t<RET (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8)> {
    typedef RET (FUNC_CLASS_TYPE::*dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8);
    typedef userdata_for_function_t<dest_func_t> userdata_for_function_info;

    static int lua_function(lua_State* ls_) {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG2>::arg_type_t arg2 = init_value_traits_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG3>::arg_type_t arg3 = init_value_traits_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG4>::arg_type_t arg4 = init_value_traits_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG5>::arg_type_t arg5 = init_value_traits_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG6>::arg_type_t arg6 = init_value_traits_t<typename basetype_ptr_traits_t<ARG6>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG7>::arg_type_t arg7 = init_value_traits_t<typename basetype_ptr_traits_t<ARG7>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG8>::arg_type_t arg8 = init_value_traits_t<typename basetype_ptr_traits_t<ARG8>::arg_type_t>::value();

        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(5), arg4);
        lua_op_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(6), arg5);
        lua_op_t<typename basetype_ptr_traits_t<ARG6>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(7), arg6);
        lua_op_t<typename basetype_ptr_traits_t<ARG7>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(8), arg7);
        lua_op_t<typename basetype_ptr_traits_t<ARG8>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(9), arg8);

        RET ret = (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2), p_t<ARG3>::r(arg3), p_t<ARG4>::r(arg4), p_t<ARG5>::r(arg5), p_t<ARG6>::r(arg6), p_t<ARG7>::r(arg7),
                                                        p_t<ARG8>::r(arg8));
        lua_op_t<typename basetype_ptr_traits_t<RET>::arg_type_t>::push_stack(ls_, ret);
        return 1;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7, typename ARG8, typename ARG9, typename RET>
struct class_function_traits_t<RET (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9)> {
    typedef RET (FUNC_CLASS_TYPE::*dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9);
    typedef userdata_for_function_t<dest_func_t> userdata_for_function_info;

    static int lua_function(lua_State* ls_) {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG2>::arg_type_t arg2 = init_value_traits_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG3>::arg_type_t arg3 = init_value_traits_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG4>::arg_type_t arg4 = init_value_traits_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG5>::arg_type_t arg5 = init_value_traits_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG6>::arg_type_t arg6 = init_value_traits_t<typename basetype_ptr_traits_t<ARG6>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG7>::arg_type_t arg7 = init_value_traits_t<typename basetype_ptr_traits_t<ARG7>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG8>::arg_type_t arg8 = init_value_traits_t<typename basetype_ptr_traits_t<ARG8>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG9>::arg_type_t arg9 = init_value_traits_t<typename basetype_ptr_traits_t<ARG9>::arg_type_t>::value();

        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(5), arg4);
        lua_op_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(6), arg5);
        lua_op_t<typename basetype_ptr_traits_t<ARG6>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(7), arg6);
        lua_op_t<typename basetype_ptr_traits_t<ARG7>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(8), arg7);
        lua_op_t<typename basetype_ptr_traits_t<ARG8>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(9), arg8);
        lua_op_t<typename basetype_ptr_traits_t<ARG9>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(10), arg9);

        RET ret = (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2), p_t<ARG3>::r(arg3), p_t<ARG4>::r(arg4), p_t<ARG5>::r(arg5), p_t<ARG6>::r(arg6), p_t<ARG7>::r(arg7),
                                                        p_t<ARG8>::r(arg8), p_t<ARG9>::r(arg9));
        lua_op_t<typename basetype_ptr_traits_t<RET>::arg_type_t>::push_stack(ls_, ret);
        return 1;
    }
};

template <typename FUNC_CLASS_TYPE, typename RET>
struct class_function_traits_t<RET (FUNC_CLASS_TYPE::*)() const> {
    typedef RET (FUNC_CLASS_TYPE::*dest_func_t)() const;
    typedef userdata_for_function_t<dest_func_t> userdata_for_function_info;

    static int lua_function(lua_State* ls_) {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);
        RET ret = (obj_ptr->*(registed_data.real_func))();
        lua_op_t<typename basetype_ptr_traits_t<RET>::arg_type_t>::push_stack(ls_, ret);
        return 1;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename RET>
struct class_function_traits_t<RET (FUNC_CLASS_TYPE::*)(ARG1) const> {
    typedef RET (FUNC_CLASS_TYPE::*dest_func_t)(ARG1) const;
    typedef userdata_for_function_t<dest_func_t> userdata_for_function_info;

    static int lua_function(lua_State* ls_) {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg1);

        RET ret = (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1));
        lua_op_t<typename basetype_ptr_traits_t<RET>::arg_type_t>::push_stack(ls_, ret);
        return 1;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename RET>
struct class_function_traits_t<RET (FUNC_CLASS_TYPE::*)(ARG1, ARG2) const> {
    typedef RET (FUNC_CLASS_TYPE::*dest_func_t)(ARG1, ARG2) const;
    typedef userdata_for_function_t<dest_func_t> userdata_for_function_info;

    static int lua_function(lua_State* ls_) {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG2>::arg_type_t arg2 = init_value_traits_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::value();
        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(3), arg2);

        RET ret = (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2));
        lua_op_t<typename basetype_ptr_traits_t<RET>::arg_type_t>::push_stack(ls_, ret);
        return 1;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename RET>
struct class_function_traits_t<RET (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3) const> {
    typedef RET (FUNC_CLASS_TYPE::*dest_func_t)(ARG1, ARG2, ARG3) const;
    typedef userdata_for_function_t<dest_func_t> userdata_for_function_info;

    static int lua_function(lua_State* ls_) {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG2>::arg_type_t arg2 = init_value_traits_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG3>::arg_type_t arg3 = init_value_traits_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::value();
        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(4), arg3);

        RET ret = (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2), p_t<ARG3>::r(arg3));
        lua_op_t<typename basetype_ptr_traits_t<RET>::arg_type_t>::push_stack(ls_, ret);
        return 1;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename RET>
struct class_function_traits_t<RET (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4) const> {
    typedef RET (FUNC_CLASS_TYPE::*dest_func_t)(ARG1, ARG2, ARG3, ARG4) const;
    typedef userdata_for_function_t<dest_func_t> userdata_for_function_info;

    static int lua_function(lua_State* ls_) {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG2>::arg_type_t arg2 = init_value_traits_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG3>::arg_type_t arg3 = init_value_traits_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG4>::arg_type_t arg4 = init_value_traits_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::value();
        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(5), arg4);

        RET ret = (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2), p_t<ARG3>::r(arg3), p_t<ARG4>::r(arg4));
        lua_op_t<typename basetype_ptr_traits_t<RET>::arg_type_t>::push_stack(ls_, ret);
        return 1;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename RET>
struct class_function_traits_t<RET (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5) const> {
    typedef RET (FUNC_CLASS_TYPE::*dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5) const;
    typedef userdata_for_function_t<dest_func_t> userdata_for_function_info;

    static int lua_function(lua_State* ls_) {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG2>::arg_type_t arg2 = init_value_traits_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG3>::arg_type_t arg3 = init_value_traits_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG4>::arg_type_t arg4 = init_value_traits_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG5>::arg_type_t arg5 = init_value_traits_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::value();

        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(5), arg4);
        lua_op_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(6), arg5);

        RET ret = (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2), p_t<ARG3>::r(arg3), p_t<ARG4>::r(arg4), p_t<ARG5>::r(arg5));
        lua_op_t<typename basetype_ptr_traits_t<RET>::arg_type_t>::push_stack(ls_, ret);
        return 1;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename RET>
struct class_function_traits_t<RET (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6) const> {
    typedef RET (FUNC_CLASS_TYPE::*dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6) const;
    typedef userdata_for_function_t<dest_func_t> userdata_for_function_info;

    static int lua_function(lua_State* ls_) {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG2>::arg_type_t arg2 = init_value_traits_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG3>::arg_type_t arg3 = init_value_traits_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG4>::arg_type_t arg4 = init_value_traits_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG5>::arg_type_t arg5 = init_value_traits_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG6>::arg_type_t arg6 = init_value_traits_t<typename basetype_ptr_traits_t<ARG6>::arg_type_t>::value();

        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(5), arg4);
        lua_op_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(6), arg5);
        lua_op_t<typename basetype_ptr_traits_t<ARG6>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(7), arg6);

        RET ret = (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2), p_t<ARG3>::r(arg3), p_t<ARG4>::r(arg4), p_t<ARG5>::r(arg5), p_t<ARG6>::r(arg6));
        lua_op_t<typename basetype_ptr_traits_t<RET>::arg_type_t>::push_stack(ls_, ret);
        return 1;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7, typename RET>
struct class_function_traits_t<RET (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7) const> {
    typedef RET (FUNC_CLASS_TYPE::*dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7) const;
    typedef userdata_for_function_t<dest_func_t> userdata_for_function_info;

    static int lua_function(lua_State* ls_) {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG2>::arg_type_t arg2 = init_value_traits_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG3>::arg_type_t arg3 = init_value_traits_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG4>::arg_type_t arg4 = init_value_traits_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG5>::arg_type_t arg5 = init_value_traits_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG6>::arg_type_t arg6 = init_value_traits_t<typename basetype_ptr_traits_t<ARG6>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG7>::arg_type_t arg7 = init_value_traits_t<typename basetype_ptr_traits_t<ARG7>::arg_type_t>::value();

        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(5), arg4);
        lua_op_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(6), arg5);
        lua_op_t<typename basetype_ptr_traits_t<ARG6>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(7), arg6);
        lua_op_t<typename basetype_ptr_traits_t<ARG7>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(8), arg7);

        RET ret = (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2), p_t<ARG3>::r(arg3), p_t<ARG4>::r(arg4), p_t<ARG5>::r(arg5), p_t<ARG6>::r(arg6), p_t<ARG7>::r(arg7));
        lua_op_t<typename basetype_ptr_traits_t<RET>::arg_type_t>::push_stack(ls_, ret);
        return 1;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7, typename ARG8, typename RET>
struct class_function_traits_t<RET (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8) const> {
    typedef RET (FUNC_CLASS_TYPE::*dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8) const;
    typedef userdata_for_function_t<dest_func_t> userdata_for_function_info;

    static int lua_function(lua_State* ls_) {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG2>::arg_type_t arg2 = init_value_traits_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG3>::arg_type_t arg3 = init_value_traits_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG4>::arg_type_t arg4 = init_value_traits_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG5>::arg_type_t arg5 = init_value_traits_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG6>::arg_type_t arg6 = init_value_traits_t<typename basetype_ptr_traits_t<ARG6>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG7>::arg_type_t arg7 = init_value_traits_t<typename basetype_ptr_traits_t<ARG7>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG8>::arg_type_t arg8 = init_value_traits_t<typename basetype_ptr_traits_t<ARG8>::arg_type_t>::value();

        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(5), arg4);
        lua_op_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(6), arg5);
        lua_op_t<typename basetype_ptr_traits_t<ARG6>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(7), arg6);
        lua_op_t<typename basetype_ptr_traits_t<ARG7>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(8), arg7);
        lua_op_t<typename basetype_ptr_traits_t<ARG8>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(9), arg8);

        RET ret = (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2), p_t<ARG3>::r(arg3), p_t<ARG4>::r(arg4), p_t<ARG5>::r(arg5), p_t<ARG6>::r(arg6), p_t<ARG7>::r(arg7),
                                                        p_t<ARG8>::r(arg8));
        lua_op_t<typename basetype_ptr_traits_t<RET>::arg_type_t>::push_stack(ls_, ret);
        return 1;
    }
};

template <typename FUNC_CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7, typename ARG8, typename ARG9, typename RET>
struct class_function_traits_t<RET (FUNC_CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9) const> {
    typedef RET (FUNC_CLASS_TYPE::*dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9) const;
    typedef userdata_for_function_t<dest_func_t> userdata_for_function_info;

    static int lua_function(lua_State* ls_) {
        void* dest_data = lua_touserdata(ls_, lua_upvalueindex(1));
        userdata_for_function_info& registed_data = *((userdata_for_function_info*)dest_data);

        FUNC_CLASS_TYPE* obj_ptr = metatable_register_impl_t<FUNC_CLASS_TYPE>::userdata_to_object(ls_);

        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG2>::arg_type_t arg2 = init_value_traits_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG3>::arg_type_t arg3 = init_value_traits_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG4>::arg_type_t arg4 = init_value_traits_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG5>::arg_type_t arg5 = init_value_traits_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG6>::arg_type_t arg6 = init_value_traits_t<typename basetype_ptr_traits_t<ARG6>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG7>::arg_type_t arg7 = init_value_traits_t<typename basetype_ptr_traits_t<ARG7>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG8>::arg_type_t arg8 = init_value_traits_t<typename basetype_ptr_traits_t<ARG8>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG9>::arg_type_t arg9 = init_value_traits_t<typename basetype_ptr_traits_t<ARG9>::arg_type_t>::value();

        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg1);
        lua_op_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(3), arg2);
        lua_op_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(4), arg3);
        lua_op_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(5), arg4);
        lua_op_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(6), arg5);
        lua_op_t<typename basetype_ptr_traits_t<ARG6>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(7), arg6);
        lua_op_t<typename basetype_ptr_traits_t<ARG7>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(8), arg7);
        lua_op_t<typename basetype_ptr_traits_t<ARG8>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(9), arg8);
        lua_op_t<typename basetype_ptr_traits_t<ARG9>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(10), arg9);

        RET ret = (obj_ptr->*(registed_data.real_func))(p_t<ARG1>::r(arg1), p_t<ARG2>::r(arg2), p_t<ARG3>::r(arg3), p_t<ARG4>::r(arg4), p_t<ARG5>::r(arg5), p_t<ARG6>::r(arg6), p_t<ARG7>::r(arg7),
                                                        p_t<ARG8>::r(arg8), p_t<ARG9>::r(arg9));
        lua_op_t<typename basetype_ptr_traits_t<RET>::arg_type_t>::push_stack(ls_, ret);
        return 1;
    }
};

template <typename CLASS_TYPE, typename RET>
struct class_property_traits_t {
    typedef int (*process_index_func_t)(lua_State*, void*, const char*);
    typedef int (*process_newindex_func_t)(lua_State*, void*, const char*, int);

    typedef RET property_t;
    typedef RET CLASS_TYPE::*property_ptr_t;
    static int process_index(lua_State* ls_, void* field_info_, const char* key_) {
        typedef class_property_info_t<property_ptr_t> class_property_info_t;
        CLASS_TYPE* obj_ptr = metatable_register_impl_t<CLASS_TYPE>::userdata_to_object(ls_);

        class_property_info_t* reg = (class_property_info_t*)field_info_;
        property_ptr_t ptr = reg->property_pos;

        if (ptr) {
            lua_op_t<property_t>::push_stack(ls_, (obj_ptr->*ptr));
            return 1;
        } else {
            printf("none this field<%s>\n", key_);
            return 0;
        }

        return 0;
    }

    static int process_newindex(lua_State* ls_, void* field_info_, const char* key_, int value_index_) {
        typedef class_property_info_t<property_ptr_t> class_property_info_t;
        CLASS_TYPE* obj_ptr = metatable_register_impl_t<CLASS_TYPE>::userdata_to_object(ls_);

        class_property_info_t* reg = (class_property_info_t*)field_info_;
        property_ptr_t ptr = reg->property_pos;

        if (ptr) {
            property_t value = init_value_traits_t<property_t>::value();
            lua_op_t<property_t>::lua_to_value(ls_, value_index_, value);
            (obj_ptr->*ptr) = value;
            return 0;
        } else {
            printf("none this field<%s>\n", key_);
            return 0;
        }

        return 0;
    }
};

template <>
struct function_traits_t<void (*)()> {
    typedef void (*dest_func_t)();
    static int lua_function(lua_State* ls_) {
        void* user_data = lua_touserdata(ls_, lua_upvalueindex(1));

        dest_func_t& registed_func = *((dest_func_t*)user_data);
        registed_func();
        return 0;
    }
};

template <typename ARG1>
struct function_traits_t<void (*)(ARG1)> {
    typedef void (*dest_func_t)(ARG1);
    static int lua_function(lua_State* ls_) {
        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();

        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(1), arg1);

        void* user_data = lua_touserdata(ls_, lua_upvalueindex(1));
        dest_func_t& registed_func = *((dest_func_t*)user_data);

        registed_func(arg1);
        return 0;
    }
};

template <typename ARG1, typename ARG2>
struct function_traits_t<void (*)(ARG1, ARG2)> {
    typedef void (*dest_func_t)(ARG1, ARG2);
    static int lua_function(lua_State* ls_) {
        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG2>::arg_type_t arg2 = init_value_traits_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::value();

        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(1), arg1);
        lua_op_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg2);

        void* user_data = lua_touserdata(ls_, lua_upvalueindex(1));
        dest_func_t& registed_func = *((dest_func_t*)user_data);

        registed_func(arg1, arg2);
        return 0;
    }
};

template <typename ARG1, typename ARG2, typename ARG3>
struct function_traits_t<void (*)(ARG1, ARG2, ARG3)> {
    typedef void (*dest_func_t)(ARG1, ARG2, ARG3);
    static int lua_function(lua_State* ls_) {
        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG2>::arg_type_t arg2 = init_value_traits_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG3>::arg_type_t arg3 = init_value_traits_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::value();

        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(1), arg1);
        lua_op_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg2);
        lua_op_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(3), arg3);

        void* user_data = lua_touserdata(ls_, lua_upvalueindex(1));
        dest_func_t& registed_func = *((dest_func_t*)user_data);

        registed_func(arg1, arg2, arg3);
        return 0;
    }
};

template <typename ARG1, typename ARG2, typename ARG3, typename ARG4>
struct function_traits_t<void (*)(ARG1, ARG2, ARG3, ARG4)> {
    typedef void (*dest_func_t)(ARG1, ARG2, ARG3, ARG4);
    static int lua_function(lua_State* ls_) {
        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG2>::arg_type_t arg2 = init_value_traits_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG3>::arg_type_t arg3 = init_value_traits_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG4>::arg_type_t arg4 = init_value_traits_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::value();

        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(1), arg1);
        lua_op_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg2);
        lua_op_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(3), arg3);
        lua_op_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(4), arg4);

        void* user_data = lua_touserdata(ls_, lua_upvalueindex(1));
        dest_func_t& registed_func = *((dest_func_t*)user_data);

        registed_func(arg1, arg2, arg3, arg4);
        return 0;
    }
};

template <typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5>
struct function_traits_t<void (*)(ARG1, ARG2, ARG3, ARG4, ARG5)> {
    typedef void (*dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5);
    static int lua_function(lua_State* ls_) {
        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG2>::arg_type_t arg2 = init_value_traits_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG3>::arg_type_t arg3 = init_value_traits_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG4>::arg_type_t arg4 = init_value_traits_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG5>::arg_type_t arg5 = init_value_traits_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::value();

        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(1), arg1);
        lua_op_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg2);
        lua_op_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(3), arg3);
        lua_op_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(4), arg4);
        lua_op_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(5), arg5);

        void* user_data = lua_touserdata(ls_, lua_upvalueindex(1));
        dest_func_t& registed_func = *((dest_func_t*)user_data);

        registed_func(arg1, arg2, arg3, arg4, arg5);
        return 0;
    }
};
template <typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6>
struct function_traits_t<void (*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6)> {
    typedef void (*dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6);
    static int lua_function(lua_State* ls_) {
        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG2>::arg_type_t arg2 = init_value_traits_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG3>::arg_type_t arg3 = init_value_traits_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG4>::arg_type_t arg4 = init_value_traits_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG5>::arg_type_t arg5 = init_value_traits_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG6>::arg_type_t arg6 = init_value_traits_t<typename basetype_ptr_traits_t<ARG6>::arg_type_t>::value();

        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(1), arg1);
        lua_op_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg2);
        lua_op_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(3), arg3);
        lua_op_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(4), arg4);
        lua_op_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(5), arg5);
        lua_op_t<typename basetype_ptr_traits_t<ARG6>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(6), arg6);

        void* user_data = lua_touserdata(ls_, lua_upvalueindex(1));
        dest_func_t& registed_func = *((dest_func_t*)user_data);

        registed_func(arg1, arg2, arg3, arg4, arg5, arg6);
        return 0;
    }
};

template <typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7>
struct function_traits_t<void (*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7)> {
    typedef void (*dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7);
    static int lua_function(lua_State* ls_) {
        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG2>::arg_type_t arg2 = init_value_traits_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG3>::arg_type_t arg3 = init_value_traits_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG4>::arg_type_t arg4 = init_value_traits_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG5>::arg_type_t arg5 = init_value_traits_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG6>::arg_type_t arg6 = init_value_traits_t<typename basetype_ptr_traits_t<ARG6>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG7>::arg_type_t arg7 = init_value_traits_t<typename basetype_ptr_traits_t<ARG7>::arg_type_t>::value();

        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(1), arg1);
        lua_op_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg2);
        lua_op_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(3), arg3);
        lua_op_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(4), arg4);
        lua_op_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(5), arg5);
        lua_op_t<typename basetype_ptr_traits_t<ARG6>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(6), arg6);
        lua_op_t<typename basetype_ptr_traits_t<ARG7>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(7), arg7);

        void* user_data = lua_touserdata(ls_, lua_upvalueindex(1));
        dest_func_t& registed_func = *((dest_func_t*)user_data);

        registed_func(arg1, arg2, arg3, arg4, arg5, arg6, arg7);
        return 0;
    }
};

template <typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7, typename ARG8>
struct function_traits_t<void (*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8)> {
    typedef void (*dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8);
    static int lua_function(lua_State* ls_) {
        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG2>::arg_type_t arg2 = init_value_traits_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG3>::arg_type_t arg3 = init_value_traits_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG4>::arg_type_t arg4 = init_value_traits_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG5>::arg_type_t arg5 = init_value_traits_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG6>::arg_type_t arg6 = init_value_traits_t<typename basetype_ptr_traits_t<ARG6>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG7>::arg_type_t arg7 = init_value_traits_t<typename basetype_ptr_traits_t<ARG7>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG8>::arg_type_t arg8 = init_value_traits_t<typename basetype_ptr_traits_t<ARG8>::arg_type_t>::value();

        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(1), arg1);
        lua_op_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg2);
        lua_op_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(3), arg3);
        lua_op_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(4), arg4);
        lua_op_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(5), arg5);
        lua_op_t<typename basetype_ptr_traits_t<ARG6>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(6), arg6);
        lua_op_t<typename basetype_ptr_traits_t<ARG7>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(7), arg7);
        lua_op_t<typename basetype_ptr_traits_t<ARG8>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(8), arg8);

        void* user_data = lua_touserdata(ls_, lua_upvalueindex(1));
        dest_func_t& registed_func = *((dest_func_t*)user_data);

        registed_func(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
        return 0;
    }
};

template <typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7, typename ARG8, typename ARG9>
struct function_traits_t<void (*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9)> {
    typedef void (*dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9);
    static int lua_function(lua_State* ls_) {
        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG2>::arg_type_t arg2 = init_value_traits_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG3>::arg_type_t arg3 = init_value_traits_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG4>::arg_type_t arg4 = init_value_traits_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG5>::arg_type_t arg5 = init_value_traits_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG6>::arg_type_t arg6 = init_value_traits_t<typename basetype_ptr_traits_t<ARG6>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG7>::arg_type_t arg7 = init_value_traits_t<typename basetype_ptr_traits_t<ARG7>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG8>::arg_type_t arg8 = init_value_traits_t<typename basetype_ptr_traits_t<ARG8>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG9>::arg_type_t arg9 = init_value_traits_t<typename basetype_ptr_traits_t<ARG9>::arg_type_t>::value();

        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(1), arg1);
        lua_op_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg2);
        lua_op_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(3), arg3);
        lua_op_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(4), arg4);
        lua_op_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(5), arg5);
        lua_op_t<typename basetype_ptr_traits_t<ARG6>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(6), arg6);
        lua_op_t<typename basetype_ptr_traits_t<ARG7>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(7), arg7);
        lua_op_t<typename basetype_ptr_traits_t<ARG8>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(8), arg8);
        lua_op_t<typename basetype_ptr_traits_t<ARG9>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(9), arg9);

        void* user_data = lua_touserdata(ls_, lua_upvalueindex(1));
        dest_func_t& registed_func = *((dest_func_t*)user_data);

        registed_func(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
        return 0;
    }
};

template <typename RET>
struct function_traits_t<RET (*)()> {
    typedef RET (*dest_func_t)();
    static int lua_function(lua_State* ls_) {
        void* user_data = lua_touserdata(ls_, lua_upvalueindex(1));
        dest_func_t& registed_func = *((dest_func_t*)user_data);

        RET ret = registed_func();
        lua_op_t<typename basetype_ptr_traits_t<RET>::arg_type_t>::push_stack(ls_, ret);

        return 1;
    }
};

template <typename RET, typename ARG1>
struct function_traits_t<RET (*)(ARG1)> {
    typedef RET (*dest_func_t)(ARG1);
    static int lua_function(lua_State* ls_) {
        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(1), arg1);

        void* user_data = lua_touserdata(ls_, lua_upvalueindex(1));
        dest_func_t& registed_func = *((dest_func_t*)user_data);

        RET ret = registed_func(arg1);
        lua_op_t<typename basetype_ptr_traits_t<RET>::arg_type_t>::push_stack(ls_, ret);
        return 1;
    }
};

template <typename RET, typename ARG1, typename ARG2>
struct function_traits_t<RET (*)(ARG1, ARG2)> {
    typedef RET (*dest_func_t)(ARG1, ARG2);
    static int lua_function(lua_State* ls_) {
        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG2>::arg_type_t arg2 = init_value_traits_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::value();
        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(1), arg1);
        lua_op_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg2);

        void* user_data = lua_touserdata(ls_, lua_upvalueindex(1));
        dest_func_t& registed_func = *((dest_func_t*)user_data);

        RET ret = registed_func(arg1, arg2);
        lua_op_t<typename basetype_ptr_traits_t<RET>::arg_type_t>::push_stack(ls_, ret);
        return 1;
    }
};

template <typename RET, typename ARG1, typename ARG2, typename ARG3>
struct function_traits_t<RET (*)(ARG1, ARG2, ARG3)> {
    typedef RET (*dest_func_t)(ARG1, ARG2, ARG3);
    static int lua_function(lua_State* ls_) {
        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG2>::arg_type_t arg2 = init_value_traits_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG3>::arg_type_t arg3 = init_value_traits_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::value();

        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(1), arg1);
        lua_op_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg2);
        lua_op_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(3), arg3);

        void* user_data = lua_touserdata(ls_, lua_upvalueindex(1));
        dest_func_t& registed_func = *((dest_func_t*)user_data);

        RET ret = registed_func(arg1, arg2, arg3);
        lua_op_t<typename basetype_ptr_traits_t<RET>::arg_type_t>::push_stack(ls_, ret);
        return 1;
    }
};

template <typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4>
struct function_traits_t<RET (*)(ARG1, ARG2, ARG3, ARG4)> {
    typedef RET (*dest_func_t)(ARG1, ARG2, ARG3, ARG4);
    static int lua_function(lua_State* ls_) {
        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG2>::arg_type_t arg2 = init_value_traits_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG3>::arg_type_t arg3 = init_value_traits_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG4>::arg_type_t arg4 = init_value_traits_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::value();

        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(1), arg1);
        lua_op_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg2);
        lua_op_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(3), arg3);
        lua_op_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(4), arg4);

        void* user_data = lua_touserdata(ls_, lua_upvalueindex(1));
        dest_func_t& registed_func = *((dest_func_t*)user_data);

        RET ret = registed_func(arg1, arg2, arg3, arg4);
        lua_op_t<typename basetype_ptr_traits_t<RET>::arg_type_t>::push_stack(ls_, ret);
        return 1;
    }
};

template <typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5>
struct function_traits_t<RET (*)(ARG1, ARG2, ARG3, ARG4, ARG5)> {
    typedef RET (*dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5);
    static int lua_function(lua_State* ls_) {
        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG2>::arg_type_t arg2 = init_value_traits_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG3>::arg_type_t arg3 = init_value_traits_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG4>::arg_type_t arg4 = init_value_traits_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG5>::arg_type_t arg5 = init_value_traits_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::value();

        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(1), arg1);
        lua_op_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg2);
        lua_op_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(3), arg3);
        lua_op_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(4), arg4);
        lua_op_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(5), arg5);

        void* user_data = lua_touserdata(ls_, lua_upvalueindex(1));
        dest_func_t& registed_func = *((dest_func_t*)user_data);

        RET ret = registed_func(arg1, arg2, arg3, arg4, arg5);
        lua_op_t<typename basetype_ptr_traits_t<RET>::arg_type_t>::push_stack(ls_, ret);
        return 1;
    }
};

template <typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6>
struct function_traits_t<RET (*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6)> {
    typedef RET (*dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6);
    static int lua_function(lua_State* ls_) {
        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG2>::arg_type_t arg2 = init_value_traits_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG3>::arg_type_t arg3 = init_value_traits_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG4>::arg_type_t arg4 = init_value_traits_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG5>::arg_type_t arg5 = init_value_traits_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG6>::arg_type_t arg6 = init_value_traits_t<typename basetype_ptr_traits_t<ARG6>::arg_type_t>::value();

        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(1), arg1);
        lua_op_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg2);
        lua_op_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(3), arg3);
        lua_op_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(4), arg4);
        lua_op_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(5), arg5);
        lua_op_t<typename basetype_ptr_traits_t<ARG6>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(6), arg6);

        void* user_data = lua_touserdata(ls_, lua_upvalueindex(1));
        dest_func_t& registed_func = *((dest_func_t*)user_data);

        RET ret = registed_func(arg1, arg2, arg3, arg4, arg5, arg6);
        lua_op_t<typename basetype_ptr_traits_t<RET>::arg_type_t>::push_stack(ls_, ret);
        return 1;
    }
};

template <typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7>
struct function_traits_t<RET (*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7)> {
    typedef RET (*dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7);
    static int lua_function(lua_State* ls_) {
        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG2>::arg_type_t arg2 = init_value_traits_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG3>::arg_type_t arg3 = init_value_traits_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG4>::arg_type_t arg4 = init_value_traits_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG5>::arg_type_t arg5 = init_value_traits_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG6>::arg_type_t arg6 = init_value_traits_t<typename basetype_ptr_traits_t<ARG6>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG7>::arg_type_t arg7 = init_value_traits_t<typename basetype_ptr_traits_t<ARG7>::arg_type_t>::value();

        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(1), arg1);
        lua_op_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg2);
        lua_op_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(3), arg3);
        lua_op_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(4), arg4);
        lua_op_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(5), arg5);
        lua_op_t<typename basetype_ptr_traits_t<ARG6>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(6), arg6);
        lua_op_t<typename basetype_ptr_traits_t<ARG7>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(7), arg7);

        void* user_data = lua_touserdata(ls_, lua_upvalueindex(1));
        dest_func_t& registed_func = *((dest_func_t*)user_data);

        RET ret = registed_func(arg1, arg2, arg3, arg4, arg5, arg6, arg7);
        lua_op_t<typename basetype_ptr_traits_t<RET>::arg_type_t>::push_stack(ls_, ret);
        return 1;
    }
};

template <typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7, typename ARG8>
struct function_traits_t<RET (*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8)> {
    typedef RET (*dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8);
    static int lua_function(lua_State* ls_) {
        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG2>::arg_type_t arg2 = init_value_traits_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG3>::arg_type_t arg3 = init_value_traits_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG4>::arg_type_t arg4 = init_value_traits_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG5>::arg_type_t arg5 = init_value_traits_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG6>::arg_type_t arg6 = init_value_traits_t<typename basetype_ptr_traits_t<ARG6>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG7>::arg_type_t arg7 = init_value_traits_t<typename basetype_ptr_traits_t<ARG7>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG8>::arg_type_t arg8 = init_value_traits_t<typename basetype_ptr_traits_t<ARG8>::arg_type_t>::value();

        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(1), arg1);
        lua_op_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg2);
        lua_op_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(3), arg3);
        lua_op_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(4), arg4);
        lua_op_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(5), arg5);
        lua_op_t<typename basetype_ptr_traits_t<ARG6>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(6), arg6);
        lua_op_t<typename basetype_ptr_traits_t<ARG7>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(7), arg7);
        lua_op_t<typename basetype_ptr_traits_t<ARG8>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(8), arg8);

        void* user_data = lua_touserdata(ls_, lua_upvalueindex(1));
        dest_func_t& registed_func = *((dest_func_t*)user_data);

        RET ret = registed_func(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
        lua_op_t<typename basetype_ptr_traits_t<RET>::arg_type_t>::push_stack(ls_, ret);
        return 1;
    }
};

template <typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7, typename ARG8, typename ARG9>
struct function_traits_t<RET (*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9)> {
    typedef RET (*dest_func_t)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9);
    static int lua_function(lua_State* ls_) {
        typename basetype_ptr_traits_t<ARG1>::arg_type_t arg1 = init_value_traits_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG2>::arg_type_t arg2 = init_value_traits_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG3>::arg_type_t arg3 = init_value_traits_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG4>::arg_type_t arg4 = init_value_traits_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG5>::arg_type_t arg5 = init_value_traits_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG6>::arg_type_t arg6 = init_value_traits_t<typename basetype_ptr_traits_t<ARG6>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG7>::arg_type_t arg7 = init_value_traits_t<typename basetype_ptr_traits_t<ARG7>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG8>::arg_type_t arg8 = init_value_traits_t<typename basetype_ptr_traits_t<ARG8>::arg_type_t>::value();
        typename basetype_ptr_traits_t<ARG9>::arg_type_t arg9 = init_value_traits_t<typename basetype_ptr_traits_t<ARG9>::arg_type_t>::value();

        lua_op_t<typename basetype_ptr_traits_t<ARG1>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(1), arg1);
        lua_op_t<typename basetype_ptr_traits_t<ARG2>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(2), arg2);
        lua_op_t<typename basetype_ptr_traits_t<ARG3>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(3), arg3);
        lua_op_t<typename basetype_ptr_traits_t<ARG4>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(4), arg4);
        lua_op_t<typename basetype_ptr_traits_t<ARG5>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(5), arg5);
        lua_op_t<typename basetype_ptr_traits_t<ARG6>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(6), arg6);
        lua_op_t<typename basetype_ptr_traits_t<ARG7>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(7), arg7);
        lua_op_t<typename basetype_ptr_traits_t<ARG8>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(8), arg8);
        lua_op_t<typename basetype_ptr_traits_t<ARG9>::arg_type_t>::lua_to_value(ls_, LUA_ARG_POS(9), arg9);

        void* user_data = lua_touserdata(ls_, lua_upvalueindex(1));
        dest_func_t& registed_func = *((dest_func_t*)user_data);

        RET ret = registed_func(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
        lua_op_t<typename basetype_ptr_traits_t<RET>::arg_type_t>::push_stack(ls_, ret);
        return 1;
    }
};

template <typename RET>
struct neko_lua_register_router_t<RET (*)()> {
    template <typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET (*arg_)(), const std::string& s_) {
        reg_->def_func(arg_, s_);
    }
};
template <typename RET, typename ARG1>
struct neko_lua_register_router_t<RET (*)(ARG1)> {
    template <typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET (*arg_)(ARG1), const std::string& s_) {
        reg_->def_func(arg_, s_);
    }
};
template <typename RET, typename ARG1, typename ARG2>
struct neko_lua_register_router_t<RET (*)(ARG1, ARG2)> {
    template <typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET (*arg_)(ARG1, ARG2), const std::string& s_) {
        reg_->def_func(arg_, s_);
    }
};
template <typename RET, typename ARG1, typename ARG2, typename ARG3>
struct neko_lua_register_router_t<RET (*)(ARG1, ARG2, ARG3)> {
    template <typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET (*arg_)(ARG1, ARG2, ARG3), const std::string& s_) {
        reg_->def_func(arg_, s_);
    }
};
template <typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4>
struct neko_lua_register_router_t<RET (*)(ARG1, ARG2, ARG3, ARG4)> {
    template <typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET (*arg_)(ARG1, ARG2, ARG3, ARG4), const std::string& s_) {
        reg_->def_func(arg_, s_);
    }
};
template <typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5>
struct neko_lua_register_router_t<RET (*)(ARG1, ARG2, ARG3, ARG4, ARG5)> {
    template <typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET (*arg_)(ARG1, ARG2, ARG3, ARG4, ARG5), const std::string& s_) {
        reg_->def_func(arg_, s_);
    }
};
template <typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6>
struct neko_lua_register_router_t<RET (*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6)> {
    template <typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET (*arg_)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6), const std::string& s_) {
        reg_->def_func(arg_, s_);
    }
};
template <typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7>
struct neko_lua_register_router_t<RET (*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7)> {
    template <typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET (*arg_)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7), const std::string& s_) {
        reg_->def_func(arg_, s_);
    }
};
template <typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7, typename ARG8>
struct neko_lua_register_router_t<RET (*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8)> {
    template <typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET (*arg_)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8), const std::string& s_) {
        reg_->def_func(arg_, s_);
    }
};
template <typename RET, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7, typename ARG8, typename ARG9>
struct neko_lua_register_router_t<RET (*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9)> {
    template <typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET (*arg_)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9), const std::string& s_) {
        reg_->def_func(arg_, s_);
    }
};

template <typename RET, typename CLASS_TYPE>
struct neko_lua_register_router_t<RET (CLASS_TYPE::*)()> {
    template <typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET (CLASS_TYPE::*arg_)(), const std::string& s_) {
        reg_->def_class_func(arg_, s_);
    }
};
template <typename RET, typename CLASS_TYPE, typename ARG1>
struct neko_lua_register_router_t<RET (CLASS_TYPE::*)(ARG1)> {
    template <typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET (CLASS_TYPE::*arg_)(ARG1), const std::string& s_) {
        reg_->def_class_func(arg_, s_);
    }
};
template <typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2>
struct neko_lua_register_router_t<RET (CLASS_TYPE::*)(ARG1, ARG2)> {
    template <typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET (CLASS_TYPE::*arg_)(ARG1, ARG2), const std::string& s_) {
        reg_->def_class_func(arg_, s_);
    }
};
template <typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3>
struct neko_lua_register_router_t<RET (CLASS_TYPE::*)(ARG1, ARG2, ARG3)> {
    template <typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET (CLASS_TYPE::*arg_)(ARG1, ARG2, ARG3), const std::string& s_) {
        reg_->def_class_func(arg_, s_);
    }
};
template <typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4>
struct neko_lua_register_router_t<RET (CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4)> {
    template <typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET (CLASS_TYPE::*arg_)(ARG1, ARG2, ARG3, ARG4), const std::string& s_) {
        reg_->def_class_func(arg_, s_);
    }
};
template <typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5>
struct neko_lua_register_router_t<RET (CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5)> {
    template <typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET (CLASS_TYPE::*arg_)(ARG1, ARG2, ARG3, ARG4, ARG5), const std::string& s_) {
        reg_->def_class_func(arg_, s_);
    }
};
template <typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6>
struct neko_lua_register_router_t<RET (CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6)> {
    template <typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET (CLASS_TYPE::*arg_)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6), const std::string& s_) {
        reg_->def_class_func(arg_, s_);
    }
};
template <typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7>
struct neko_lua_register_router_t<RET (CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7)> {
    template <typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET (CLASS_TYPE::*arg_)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7), const std::string& s_) {
        reg_->def_class_func(arg_, s_);
    }
};
template <typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7, typename ARG8>
struct neko_lua_register_router_t<RET (CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8)> {
    template <typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET (CLASS_TYPE::*arg_)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8), const std::string& s_) {
        reg_->def_class_func(arg_, s_);
    }
};
template <typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7, typename ARG8, typename ARG9>
struct neko_lua_register_router_t<RET (CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9)> {
    template <typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET (CLASS_TYPE::*arg_)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9), const std::string& s_) {
        reg_->def_class_func(arg_, s_);
    }
};

template <typename RET, typename CLASS_TYPE>
struct neko_lua_register_router_t<RET (CLASS_TYPE::*)() const> {
    template <typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET (CLASS_TYPE::*arg_)() const, const std::string& s_) {
        reg_->def_class_func(arg_, s_);
    }
};
template <typename RET, typename CLASS_TYPE, typename ARG1>
struct neko_lua_register_router_t<RET (CLASS_TYPE::*)(ARG1) const> {
    template <typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET (CLASS_TYPE::*arg_)(ARG1) const, const std::string& s_) {
        reg_->def_class_func(arg_, s_);
    }
};
template <typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2>
struct neko_lua_register_router_t<RET (CLASS_TYPE::*)(ARG1, ARG2) const> {
    template <typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET (CLASS_TYPE::*arg_)(ARG1, ARG2) const, const std::string& s_) {
        reg_->def_class_func(arg_, s_);
    }
};
template <typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3>
struct neko_lua_register_router_t<RET (CLASS_TYPE::*)(ARG1, ARG2, ARG3) const> {
    template <typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET (CLASS_TYPE::*arg_)(ARG1, ARG2, ARG3) const, const std::string& s_) {
        reg_->def_class_func(arg_, s_);
    }
};
template <typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4>
struct neko_lua_register_router_t<RET (CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4) const> {
    template <typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET (CLASS_TYPE::*arg_)(ARG1, ARG2, ARG3, ARG4) const, const std::string& s_) {
        reg_->def_class_func(arg_, s_);
    }
};
template <typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5>
struct neko_lua_register_router_t<RET (CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5) const> {
    template <typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET (CLASS_TYPE::*arg_)(ARG1, ARG2, ARG3, ARG4, ARG5) const, const std::string& s_) {
        reg_->def_class_func(arg_, s_);
    }
};
template <typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6>
struct neko_lua_register_router_t<RET (CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6) const> {
    template <typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET (CLASS_TYPE::*arg_)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6) const, const std::string& s_) {
        reg_->def_class_func(arg_, s_);
    }
};
template <typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7>
struct neko_lua_register_router_t<RET (CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7) const> {
    template <typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET (CLASS_TYPE::*arg_)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7) const, const std::string& s_) {
        reg_->def_class_func(arg_, s_);
    }
};
template <typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7, typename ARG8>
struct neko_lua_register_router_t<RET (CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8) const> {
    template <typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET (CLASS_TYPE::*arg_)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8) const, const std::string& s_) {
        reg_->def_class_func(arg_, s_);
    }
};
template <typename RET, typename CLASS_TYPE, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5, typename ARG6, typename ARG7, typename ARG8, typename ARG9>
struct neko_lua_register_router_t<RET (CLASS_TYPE::*)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9) const> {
    template <typename REG_TYPE>
    static void call(REG_TYPE* reg_, RET (CLASS_TYPE::*arg_)(ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9) const, const std::string& s_) {
        reg_->def_class_func(arg_, s_);
    }
};
}  // namespace neko

#endif
