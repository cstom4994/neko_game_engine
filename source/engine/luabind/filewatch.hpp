#ifndef NEKO_FILEWATCH_H
#define NEKO_FILEWATCH_H

#include <functional>
#include <optional>
#include <queue>
#include <string>

#include "engine/neko.h"

#if defined(_WIN32)
#include <list>
#elif defined(__APPLE__)
#include <CoreServices/CoreServices.h>

#include <mutex>
#include <set>
#elif defined(__linux__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
#include <map>
#else
#error unsupport platform
#endif

struct lua_State;

namespace neko::filewatch {
class task;

struct notify {
    enum class flag {
        modify,
        rename,
    };
    flag flags;
    std::string path;
    notify(const flag& flags, const std::string& path) noexcept : flags(flags), path(path) {}
};

class watch {
public:
#if defined(_WIN32)
    using string_type = std::wstring;
#else
    using string_type = std::string;
#endif
    using filter = std::function<bool(const char*)>;
    static inline filter DefaultFilter = [](const char*) { return true; };

    watch() noexcept;
    ~watch() noexcept;
    void stop() noexcept;
    void add(const string_type& path) noexcept;
    void set_recursive(bool enable) noexcept;
    bool set_follow_symlinks(bool enable) noexcept;
    bool set_filter(filter f = DefaultFilter) noexcept;
    std::optional<notify> select() noexcept;
#if defined(__APPLE__)
    void event_update(const char* paths[], const FSEventStreamEventFlags flags[], size_t n) noexcept;
#endif

private:
#if defined(_WIN32)
    bool event_update(task& task) noexcept;
#elif defined(__APPLE__)
    bool create_stream(CFArrayRef cf_paths) noexcept;
    void destroy_stream() noexcept;
    void update_stream() noexcept;
#elif defined(__linux__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
    void event_update(void* event) noexcept;
#endif

private:
    std::queue<notify> m_notify;
    bool m_recursive = true;
#if defined(_WIN32)
    std::list<task> m_tasks;
#elif defined(__APPLE__)
    std::mutex m_mutex;
    std::set<std::string> m_paths;
    FSEventStreamRef m_stream;
    dispatch_queue_t m_fsevent_queue;
#elif defined(__linux__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
    std::map<int, std::string> m_fd_path;
    int m_inotify_fd;
    bool m_follow_symlinks = false;
    filter m_filter = DefaultFilter;
#endif
};

}  // namespace neko::filewatch

#endif

#include "engine/neko.hpp"

#ifdef NEKO_PLATFORM_WIN

#include <Windows.h>

namespace neko::filewatch {
class task : public OVERLAPPED {
    static const size_t kBufSize = 16 * 1024;

public:
    task() noexcept;
    ~task() noexcept;

    enum class result {
        success,
        wait,
        failed,
        zero,
    };

    bool open(const std::wstring& path) noexcept;
    bool start(bool recursive) noexcept;
    void cancel() noexcept;
    result try_read() noexcept;
    const std::wstring& path() const noexcept;
    const std::byte* data() const noexcept;

private:
    std::wstring m_path;
    HANDLE m_directory;
    std::array<std::byte, kBufSize> m_buffer;
};

task::task() noexcept : m_path(), m_directory(INVALID_HANDLE_VALUE), m_buffer() {
    memset((OVERLAPPED*)this, 0, sizeof(OVERLAPPED));
    hEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
}

task::~task() noexcept { assert(m_directory == INVALID_HANDLE_VALUE); }

bool task::open(const std::wstring& path) noexcept {
    if (m_directory != INVALID_HANDLE_VALUE) {
        return true;
    }
    if (path.back() != L'/') {
        m_path = path + L"/";
    } else {
        m_path = path;
    }
    m_directory =
            ::CreateFileW(m_path.c_str(), FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);
    if (m_directory == INVALID_HANDLE_VALUE) {
        return false;
    }
    return true;
}

void task::cancel() noexcept {
    if (m_directory != INVALID_HANDLE_VALUE) {
        ::CancelIo(m_directory);
        ::CloseHandle(m_directory);
        m_directory = INVALID_HANDLE_VALUE;
    }
}

bool task::start(bool recursive) noexcept {
    if (m_directory == INVALID_HANDLE_VALUE) {
        return false;
    }
    if (!ResetEvent(hEvent)) {
        return false;
    }
    if (!::ReadDirectoryChangesW(m_directory, &m_buffer[0], static_cast<DWORD>(m_buffer.size()), recursive,
                                 FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION, NULL, this, NULL)) {
        ::CloseHandle(m_directory);
        m_directory = INVALID_HANDLE_VALUE;
        return false;
    }
    return true;
}

task::result task::try_read() noexcept {
    DWORD dwNumberOfBytesTransfered = 0;
    const bool ok = GetOverlappedResult(m_directory, this, &dwNumberOfBytesTransfered, FALSE);
    const DWORD dwErrorCode = ::GetLastError();
    if (!ok) {
        if (dwErrorCode == ERROR_IO_INCOMPLETE) {
            return result::wait;
        }
    }
    if (dwErrorCode != 0) {
        if (dwErrorCode == ERROR_NOTIFY_ENUM_DIR) {
            // TODO 通知溢出
            return result::zero;
        }
        cancel();
        return result::failed;
    }
    if (!dwNumberOfBytesTransfered) {
        return result::zero;
    }
    assert(dwNumberOfBytesTransfered >= offsetof(FILE_NOTIFY_INFORMATION, FileName) + sizeof(WCHAR));
    assert(dwNumberOfBytesTransfered <= m_buffer.size());
    return result::success;
}

const std::wstring& task::path() const noexcept { return m_path; }

const std::byte* task::data() const noexcept { return m_buffer.data(); }

watch::watch() noexcept : m_notify(), m_tasks() {}

watch::~watch() noexcept { stop(); }

void watch::stop() noexcept {
    if (m_tasks.empty()) {
        return;
    }
    for (auto& task : m_tasks) {
        task.cancel();
    }
    m_tasks.clear();
}

void watch::add(const string_type& path) noexcept {
    auto& t = m_tasks.emplace_back();
    if (t.open(path)) {
        if (t.start(m_recursive)) {
            return;
        }
    }
    m_tasks.pop_back();
}

void watch::set_recursive(bool enable) noexcept { m_recursive = enable; }

bool watch::set_follow_symlinks(bool enable) noexcept { return false; }

bool watch::set_filter(filter f) noexcept { return false; }

bool watch::event_update(task& task) noexcept {
    switch (task.try_read()) {
        case task::result::wait:
            return true;
        case task::result::failed:
            task.cancel();
            return false;
        case task::result::zero:
            return task.start(m_recursive);
        case task::result::success:
            break;
    }
    const std::byte* data = task.data();
    for (;;) {
        const FILE_NOTIFY_INFORMATION& fni = (const FILE_NOTIFY_INFORMATION&)*data;
        std::wstring path(fni.FileName, fni.FileNameLength / sizeof(wchar_t));
        path = task.path() + path;
        switch (fni.Action) {
            case FILE_ACTION_MODIFIED:
                m_notify.emplace(notify::flag::modify, wtf8::w2u(path));
                break;
            case FILE_ACTION_ADDED:
            case FILE_ACTION_REMOVED:
            case FILE_ACTION_RENAMED_OLD_NAME:
            case FILE_ACTION_RENAMED_NEW_NAME:
                m_notify.emplace(notify::flag::rename, wtf8::w2u(path));
                break;
            default:
                NEKO_ASSERT(0, "unreachable");
                break;
        }
        if (!fni.NextEntryOffset) {
            break;
        }
        data += fni.NextEntryOffset;
    }
    return task.start(m_recursive);
}

std::optional<notify> watch::select() noexcept {
    for (auto iter = m_tasks.begin(); iter != m_tasks.end();) {
        if (event_update(*iter)) {
            ++iter;
        } else {
            iter = m_tasks.erase(iter);
        }
    }
    if (m_notify.empty()) {
        return std::nullopt;
    }
    auto n = m_notify.front();
    m_notify.pop();
    return n;
}
}  // namespace neko::filewatch

#elif defined(NEKO_PLATFORM_LINUX)

#include <poll.h>
#include <sys/inotify.h>
#include <unistd.h>

#include <cassert>
#include <cstddef>
#include <functional>

#include "filewatch.h"

namespace neko::filewatch {
watch::watch() noexcept : m_notify(), m_fd_path(), m_inotify_fd(inotify_init1(IN_NONBLOCK | IN_CLOEXEC)) { assert(m_inotify_fd != -1); }

watch::~watch() noexcept { stop(); }

void watch::stop() noexcept {
    if (m_inotify_fd == -1) {
        return;
    }
    for (auto& [desc, _] : m_fd_path) {
        (void)_;
        inotify_rm_watch(m_inotify_fd, desc);
    }
    m_fd_path.clear();
    close(m_inotify_fd);
    m_inotify_fd = -1;
}

void watch::add(const string_type& str) noexcept {
    if (m_inotify_fd == -1) {
        return;
    }
    if (!m_filter(str.c_str())) {
        return;
    }
    fs::path path = str;
    if (m_follow_symlinks) {
        std::error_code ec;
        path = fs::canonical(path, ec);
        if (ec) {
            return;
        }
    }
    int desc = inotify_add_watch(m_inotify_fd, path.c_str(), IN_ALL_EVENTS);
    if (desc != -1) {
        const auto& emplace_result = m_fd_path.emplace(std::make_pair(desc, path.string()));
        if (!emplace_result.second) {
            return;
        }
    }
    if (!m_recursive) {
        return;
    }
    std::error_code ec;
    fs::directory_iterator iter{path, fs::directory_options::skip_permission_denied, ec};
    fs::directory_iterator end{};
    for (; !ec && iter != end; iter.increment(ec)) {
        std::error_code file_status_ec;
        if (fs::is_directory(m_follow_symlinks ? iter->status(file_status_ec) : iter->symlink_status(file_status_ec))) {
            add(iter->path());
        }
    }
}

void watch::set_recursive(bool enable) noexcept { m_recursive = enable; }

bool watch::set_follow_symlinks(bool enable) noexcept {
    m_follow_symlinks = enable;
    return true;
}

bool watch::set_filter(filter f) noexcept {
    m_filter = f;
    return true;
}

void watch::event_update(void* e) noexcept {
    inotify_event* event = (inotify_event*)e;
    if (event->mask & IN_Q_OVERFLOW) {
        // TODO?
    }

    auto filename = m_fd_path[event->wd];
    if (event->len > 1) {
        filename += "/";
        filename += std::string(event->name);
    }
    if (event->mask & (IN_CREATE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO)) {
        m_notify.emplace(notify::flag::rename, filename);
    } else if (event->mask & (IN_MOVE_SELF | IN_ATTRIB | IN_CLOSE_WRITE | IN_MODIFY)) {
        m_notify.emplace(notify::flag::modify, filename);
    }

    if (event->mask & (IN_IGNORED | IN_DELETE_SELF)) {
        m_fd_path.erase(event->wd);
    }
    if (event->mask & IN_MOVE_SELF) {
        inotify_rm_watch(m_inotify_fd, event->wd);
        m_fd_path.erase(event->wd);
    }
    if (m_recursive && (event->mask & IN_ISDIR) && (event->mask & IN_CREATE)) {
        add(filename);
    }
}

std::optional<notify> watch::select() noexcept {
    do {
        if (m_inotify_fd == -1) {
            break;
        }

        struct pollfd pfd_read;
        pfd_read.fd = m_inotify_fd;
        pfd_read.events = POLLIN;
        if (poll(&pfd_read, 1, 0) != 1) {
            break;
        }

        std::byte buf[4096];
        ssize_t n = read(m_inotify_fd, buf, sizeof buf);
        if (n == 0 || n == -1) {
            break;
        }
        for (std::byte* p = buf; p < buf + n;) {
            auto event = (struct inotify_event*)p;
            event_update(event);
            p += sizeof(*event) + event->len;
        }
    } while (false);

    if (m_notify.empty()) {
        return std::nullopt;
    }
    auto msg = m_notify.front();
    m_notify.pop();
    return msg;
}
}  // namespace neko::filewatch

#elif defined(NEKO_PLATFORM_APPLE)

namespace neko::filewatch {
static void event_cb(ConstFSEventStreamRef streamRef, void* info, size_t numEvents, void* eventPaths, const FSEventStreamEventFlags eventFlags[], const FSEventStreamEventId eventIds[]) noexcept {
    (void)streamRef;
    (void)eventIds;
    watch* self = (watch*)info;
    self->event_update((const char**)eventPaths, eventFlags, numEvents);
}

watch::watch() noexcept : m_notify(), m_paths(), m_stream(NULL) {}

watch::~watch() noexcept { stop(); }

void watch::stop() noexcept {
    destroy_stream();
    m_paths.clear();
}

bool watch::create_stream(CFArrayRef cf_paths) noexcept {
    if (m_stream) {
        return false;
    }
    FSEventStreamContext ctx = {0, this, NULL, NULL, NULL};

    FSEventStreamRef ref = FSEventStreamCreate(NULL, &event_cb, &ctx, cf_paths, kFSEventStreamEventIdSinceNow, 0.05, kFSEventStreamCreateFlagNoDefer | kFSEventStreamCreateFlagFileEvents);
    if (ref == NULL) {
        return false;
    }
    m_fsevent_queue = dispatch_queue_create("fsevent_queue", NULL);
    FSEventStreamSetDispatchQueue(ref, m_fsevent_queue);
    if (!FSEventStreamStart(ref)) {
        FSEventStreamInvalidate(ref);
        FSEventStreamRelease(ref);
        return false;
    }
    m_stream = ref;
    return true;
}

void watch::destroy_stream() noexcept {
    if (!m_stream) {
        return;
    }
    FSEventStreamStop(m_stream);
    FSEventStreamInvalidate(m_stream);
    FSEventStreamRelease(m_stream);
    dispatch_release(m_fsevent_queue);
    m_stream = NULL;
}

void watch::add(const string_type& path) noexcept {
    m_paths.emplace(path);
    update_stream();
}

void watch::set_recursive(bool enable) noexcept { m_recursive = enable; }

bool watch::set_follow_symlinks(bool enable) noexcept { return false; }

bool watch::set_filter(filter f) noexcept { return false; }

void watch::update_stream() noexcept {
    destroy_stream();
    if (m_paths.empty()) {
        return;
    }
    std::unique_ptr<CFStringRef[]> paths(new CFStringRef[m_paths.size()]);
    size_t i = 0;
    for (auto& path : m_paths) {
        paths[i] = CFStringCreateWithCString(NULL, path.c_str(), kCFStringEncodingUTF8);
        if (paths[i] == NULL) {
            while (i != 0) {
                CFRelease(paths[--i]);
            }
            return;
        }
        i++;
    }
    CFArrayRef cf_paths = CFArrayCreate(NULL, (const void**)&paths[0], m_paths.size(), NULL);
    if (create_stream(cf_paths)) {
        return;
    }
    CFRelease(cf_paths);
}

void watch::event_update(const char* paths[], const FSEventStreamEventFlags flags[], size_t n) noexcept {
    std::unique_lock<std::mutex> lock(m_mutex);
    for (size_t i = 0; i < n; ++i) {
        const char* path = paths[i];
        if (!m_recursive && path[0] != '\0' && strchr(path + 1, '/') != NULL) {
            continue;
        }
        if (flags[i] & (kFSEventStreamEventFlagItemCreated | kFSEventStreamEventFlagItemRemoved | kFSEventStreamEventFlagItemRenamed)) {
            m_notify.emplace(notify::flag::rename, path);
        } else if (flags[i] & (kFSEventStreamEventFlagItemFinderInfoMod | kFSEventStreamEventFlagItemModified | kFSEventStreamEventFlagItemInodeMetaMod | kFSEventStreamEventFlagItemChangeOwner |
                               kFSEventStreamEventFlagItemXattrMod)) {
            m_notify.emplace(notify::flag::modify, path);
        }
    }
}

std::optional<notify> watch::select() noexcept {
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_notify.empty()) {
        return std::nullopt;
    }
    auto n = m_notify.front();
    m_notify.pop();
    return n;
}
}  // namespace neko::filewatch

#else

#endif

#include <filesystem>
#include <format>
#include <string_view>

#include "engine/neko_lua.hpp"
#include "engine/neko_luabind.hpp"

namespace neko::lua_filewatch {
static filewatch::watch& to(lua_State* L, int idx) { return lua::checkudata<filewatch::watch>(L, idx); }

static lua_State* get_thread(lua_State* L) {
    lua_getiuservalue(L, 1, 1);
    lua_State* thread = lua_tothread(L, -1);
    lua_pop(L, 1);
    return thread;
}

static int add(lua_State* L) {
    auto& self = to(L, 1);
    auto pathstr = lua::checkstrview(L, 2);
#if defined(_WIN32)
    std::filesystem::path path{wtf8::u2w(pathstr)};
#else
    std::filesystem::path path{std::string{pathstr.data(), pathstr.size()}};
#endif
    std::error_code ec;
    std::filesystem::path abspath = std::filesystem::absolute(path, ec);
    if (ec) {
        lua_pushstring(L, std::format("error fs::absolute {0}", ec.value()).c_str());
        lua_error(L);
        return 0;
    }
    self.add(abspath.lexically_normal().generic_string<filewatch::watch::string_type::value_type>());
    return 0;
}

static int set_recursive(lua_State* L) {
    auto& self = to(L, 1);
    bool enable = lua_toboolean(L, 2);
    self.set_recursive(enable);
    lua_pushboolean(L, 1);
    return 1;
}

static int set_follow_symlinks(lua_State* L) {
    auto& self = to(L, 1);
    bool enable = lua_toboolean(L, 2);
    bool ok = self.set_follow_symlinks(enable);
    lua_pushboolean(L, ok);
    return 1;
}

static int set_filter(lua_State* L) {
    auto& self = to(L, 1);
    if (lua_isnoneornil(L, 2)) {
        bool ok = self.set_filter();
        lua_pushboolean(L, ok);
        return 1;
    }
    lua_State* thread = get_thread(L);
    lua_settop(L, 2);
    lua_xmove(L, thread, 1);
    if (lua_gettop(thread) > 1) {
        lua_replace(thread, 1);
    }
    bool ok = self.set_filter([=](const char* path) {
        lua_pushvalue(thread, 1);
        lua_pushstring(thread, path);
        if (LUA_OK != lua_pcall(thread, 1, 1, 0)) {
            lua_pop(thread, 1);
            return true;
        }
        bool r = lua_toboolean(thread, -1);
        lua_pop(thread, 1);
        return r;
    });
    lua_pushboolean(L, ok);
    return 1;
}

static int select(lua_State* L) {
    auto& self = to(L, 1);
    auto notify = self.select();
    if (!notify) {
        return 0;
    }
    switch (notify->flags) {
        case filewatch::notify::flag::modify:
            lua_pushstring(L, "modify");
            break;
        case filewatch::notify::flag::rename:
            lua_pushstring(L, "rename");
            break;
        default:
            // std::unreachable();
            NEKO_ASSERT(0, "unreachable");
    }
    lua_pushlstring(L, notify->path.data(), notify->path.size());
    return 2;
}

static int mt_close(lua_State* L) {
    auto& self = to(L, 1);
    self.stop();
    return 0;
}

static void metatable(lua_State* L) {
    static luaL_Reg lib[] = {{"add", add}, {"set_recursive", set_recursive}, {"set_follow_symlinks", set_follow_symlinks}, {"set_filter", set_filter}, {"select", select}, {NULL, NULL}};
    luaL_newlibtable(L, lib);
    luaL_setfuncs(L, lib, 0);
    lua_setfield(L, -2, "__index");
    static luaL_Reg mt[] = {{"__close", mt_close}, {NULL, NULL}};
    luaL_setfuncs(L, mt, 0);
}

static int create(lua_State* L) {
    lua::newudata<filewatch::watch>(L);
    lua_newthread(L);
    lua_setiuservalue(L, -2, 1);
    return 1;
}

static int luaopen(lua_State* L) {
    static luaL_Reg lib[] = {{"create", create}, {NULL, NULL}};
    luaL_newlibtable(L, lib);
    luaL_setfuncs(L, lib, 0);
    return 1;
}
}  // namespace neko::lua_filewatch

DEFINE_LUAOPEN(filewatch)

namespace neko::lua {
template <>
struct udata<filewatch::watch> {
    static inline int nupvalue = 1;
    static inline auto metatable = neko::lua_filewatch::metatable;
};
}  // namespace neko::lua
