
#ifndef NEKO_HOTLOAD_H
#define NEKO_HOTLOAD_H

// https://fungos.github.io/cr-simple-c-hot-reload/

#include "engine/neko.h"
#include "engine/neko_platform.h"

#if defined(NEKO_PLATFORM_LINUX)

#else

#define NEKO_HOTLOAD_WINDOWS
#define NEKO_HOTLOAD_MODULE(name) "" name ".dll"

#if defined(_MSC_VER)
#if defined(__cplusplus)
#define NEKO_HOTLOAD_EXPORT extern "C" __declspec(dllexport)
#define NEKO_HOTLOAD_IMPORT extern "C" __declspec(dllimport)
#else
#define NEKO_HOTLOAD_EXPORT __declspec(dllexport)
#define NEKO_HOTLOAD_IMPORT __declspec(dllimport)
#endif
#endif

enum neko_hotload_mode { NEKO_HOTLOAD_SAFEST = 0, NEKO_HOTLOAD_SAFE = 1, NEKO_HOTLOAD_UNSAFE = 2, NEKO_HOTLOAD_DISABLE = 3 };

enum neko_hotload_op {
    NEKO_HOTLOAD_LOAD = 0,
    NEKO_HOTLOAD_STEP = 1,
    NEKO_HOTLOAD_UNLOAD = 2,
    NEKO_HOTLOAD_CLOSE = 3,
};

enum neko_hotload_failure {
    NEKO_HOTLOAD_NONE,
    NEKO_HOTLOAD_BOUNDS,
    NEKO_HOTLOAD_STACKOVERFLOW,
    NEKO_HOTLOAD_INITIAL_FAILURE,
    NEKO_HOTLOAD_ABORT,
    NEKO_HOTLOAD_SEGFAULT,
    NEKO_HOTLOAD_ILLEGAL,
    NEKO_HOTLOAD_MISALIGN,
    NEKO_HOTLOAD_OTHER,
    NEKO_HOTLOAD_STATE_INVALIDATED,
    NEKO_HOTLOAD_BAD_IMAGE,
    NEKO_HOTLOAD_USER = 0x100,
};

struct neko_hotload_module;

typedef int (*neko_hotload_module_main_func)(struct neko_hotload_module *ctx, enum neko_hotload_op operation);

struct neko_hotload_module {
    void *p;
    void *userdata;
    u32 version;
    enum neko_hotload_failure failure;
    u32 next_version;
    u32 last_working_version;
};

#ifndef NEKO_HOTLOAD_HOST

#if defined(_MSC_VER)
#pragma section(".state", read, write)
#define NEKO_HOTLOAD_STATE __declspec(allocate(".state"))
#elif defined(__GNUC__)
#define NEKO_HOTLOAD_STATE __attribute__((section(".state")))
#endif

#else

#ifndef NEKO_HOTLOAD_MAIN_FUNC
#define NEKO_HOTLOAD_MAIN_FUNC "neko_hotload_main"
#endif

#define NEKO_HOTLOAD_OP_MODE NEKO_HOTLOAD_HOST

#include <algorithm>
#include <chrono>
#include <cstring>
#include <string>
#include <thread>

#if defined(NEKO_HOTLOAD_WINDOWS)
#define NEKO_HOTLOAD_PATH_SEPARATOR '\\'
#define NEKO_HOTLOAD_PATH_SEPARATOR_INVALID '/'
#else
#define NEKO_HOTLOAD_PATH_SEPARATOR '/'
#define NEKO_HOTLOAD_PATH_SEPARATOR_INVALID '\\'
#endif

static void neko_hotload_split_path(std::string path, std::string &parent_dir, std::string &base_name, std::string &ext) {
    std::replace(path.begin(), path.end(), NEKO_HOTLOAD_PATH_SEPARATOR_INVALID, NEKO_HOTLOAD_PATH_SEPARATOR);
    auto sep_pos = path.rfind(NEKO_HOTLOAD_PATH_SEPARATOR);
    auto dot_pos = path.rfind('.');

    if (sep_pos == std::string::npos) {
        parent_dir = "";
        if (dot_pos == std::string::npos) {
            ext = "";
            base_name = path;
        } else {
            ext = path.substr(dot_pos);
            base_name = path.substr(0, dot_pos);
        }
    } else {
        parent_dir = path.substr(0, sep_pos + 1);
        if (dot_pos == std::string::npos || sep_pos > dot_pos) {
            ext = "";
            base_name = path.substr(sep_pos + 1);
        } else {
            ext = path.substr(dot_pos);
            base_name = path.substr(sep_pos + 1, dot_pos - sep_pos - 1);
        }
    }
}

static std::string neko_hotload_version_path(const std::string &basepath, unsigned version, const std::string &temppath) {
    std::string folder, fname, ext;
    neko_hotload_split_path(basepath, folder, fname, ext);
    std::string ver = std::to_string(version);
#if defined(_MSC_VER)

    if (ver.size() > folder.size()) {
        fname = fname.substr(0, fname.size() - (ver.size() - folder.size() - 1));
    }
#endif
    if (!temppath.empty()) {
        folder = temppath;
    }
    return folder + fname + ver + ext;
}

namespace neko_hotload_module_section_type {
enum e { state, bss, count };
}

namespace neko_hotload_module_section_version {
enum e { backup, current, count };
}

struct neko_hotload_module_section {
    neko_hotload_module_section_type::e type = {};
    intptr_t base = 0;
    char *ptr = 0;
    s64 size = 0;
    void *data = nullptr;
};

struct neko_hotload_module_segment {
    char *ptr = 0;
    s64 size = 0;
};

struct neko_hotload_internal {
    std::string fullname = {};
    std::string temppath = {};
    time_t timestamp = {};
    void *handle = nullptr;
    neko_hotload_module_main_func main = nullptr;
    neko_hotload_module_segment seg = {};
    neko_hotload_module_section data[neko_hotload_module_section_type::count][neko_hotload_module_section_version::count] = {};
    neko_hotload_mode mode = NEKO_HOTLOAD_SAFEST;
};

static bool neko_hotload_module_section_validate(neko_hotload_module &ctx, neko_hotload_module_section_type::e type, intptr_t vaddr, intptr_t ptr, s64 size);
static void neko_hotload_module_sections_reload(neko_hotload_module &ctx, neko_hotload_module_section_version::e version);
static void neko_hotload_module_sections_store(neko_hotload_module &ctx);
static void neko_hotload_module_sections_backup(neko_hotload_module &ctx);
static void neko_hotload_module_reload(neko_hotload_module &ctx);
static int neko_hotload_module_unload(neko_hotload_module &ctx, bool rollback, bool close);
static bool neko_hotload_module_changed(neko_hotload_module &ctx);
static bool neko_hotload_module_rollback(neko_hotload_module &ctx);
static int neko_hotload_module_main(neko_hotload_module &ctx, neko_hotload_op operation);

void neko_hotload_set_temporary_path(neko_hotload_module &ctx, const std::string &path) {
    auto pimpl = (neko_hotload_internal *)ctx.p;
    pimpl->temppath = path;
}

#if defined(NEKO_HOTLOAD_WINDOWS)

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <dbghelp.h>
#include <windows.h>

// #if defined(_MSC_VER)
// #pragma comment(lib, "dbghelp.lib")
// #endif

static time_t neko_hotload_last_write_time(const std::string &path) {
    neko_unicode_convert_path(_path, path);
    WIN32_FILE_ATTRIBUTE_DATA fad;
    if (!GetFileAttributesEx(_path.c_str(), GetFileExInfoStandard, &fad)) {
        return -1;
    }

    if (fad.nFileSizeHigh == 0 && fad.nFileSizeLow == 0) {
        return -1;
    }

    LARGE_INTEGER time;
    time.HighPart = fad.ftLastWriteTime.dwHighDateTime;
    time.LowPart = fad.ftLastWriteTime.dwLowDateTime;

    return static_cast<time_t>(time.QuadPart / 10000000 - 11644473600LL);
}

static bool neko_hotload_exists(const std::string &path) {
    neko_unicode_convert_path(_path, path);
    return GetFileAttributes(_path.c_str()) != INVALID_FILE_ATTRIBUTES;
}

static bool neko_hotload_copy(const std::string &from, const std::string &to) {
    neko_unicode_convert_path(_from, from);
    neko_unicode_convert_path(_to, to);
    return CopyFile(_from.c_str(), _to.c_str(), FALSE) ? true : false;
}

static void neko_hotload_del(const std::string &path) {
    neko_unicode_convert_path(_path, path);
    DeleteFile(_path.c_str());
}

#if defined(_MSC_VER)
#include <crtdbg.h>
#include <limits.h>
#include <stdio.h>
#include <tchar.h>

static std::string neko_hotload_replace_extension(const std::string &filepath, const std::string &ext) {
    std::string folder, filename, old_ext;
    neko_hotload_split_path(filepath, folder, filename, old_ext);
    return folder + filename + ext;
}

template <class T>
static T struct_cast(void *ptr, LONG offset = 0) {
    return reinterpret_cast<T>(reinterpret_cast<intptr_t>(ptr) + offset);
}

using DebugInfoSignature = DWORD;
#define NEKO_HOTLOAD_RSDS_SIGNATURE 'SDSR'
struct neko_hotload_rsds_hdr {
    DebugInfoSignature signature;
    GUID guid;
    long version;
    char filename[1];
};

static bool neko_hotload_pe_debugdir_rva(PIMAGE_OPTIONAL_HEADER optionalHeader, DWORD &debugDirRva, DWORD &debugDirSize) {
    if (optionalHeader->Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
        auto optionalHeader64 = struct_cast<PIMAGE_OPTIONAL_HEADER64>(optionalHeader);
        debugDirRva = optionalHeader64->DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress;
        debugDirSize = optionalHeader64->DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size;
    } else {
        auto optionalHeader32 = struct_cast<PIMAGE_OPTIONAL_HEADER32>(optionalHeader);
        debugDirRva = optionalHeader32->DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress;
        debugDirSize = optionalHeader32->DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size;
    }

    if (debugDirRva == 0 && debugDirSize == 0) {
        return true;
    } else if (debugDirRva == 0 || debugDirSize == 0) {
        return false;
    }

    return true;
}

static bool neko_hotload_pe_fileoffset_rva(PIMAGE_NT_HEADERS ntHeaders, DWORD rva, DWORD &fileOffset) {
    bool found = false;
    auto *sectionHeader = IMAGE_FIRST_SECTION(ntHeaders);
    for (int i = 0; i < ntHeaders->FileHeader.NumberOfSections; i++, sectionHeader++) {
        auto sectionSize = sectionHeader->Misc.VirtualSize;
        if ((rva >= sectionHeader->VirtualAddress) && (rva < sectionHeader->VirtualAddress + sectionSize)) {
            found = true;
            break;
        }
    }

    if (!found) {
        return false;
    }

    const int diff = static_cast<int>(sectionHeader->VirtualAddress - sectionHeader->PointerToRawData);
    fileOffset = rva - diff;
    return true;
}

static char *neko_hotload_pdb_find(LPBYTE imageBase, PIMAGE_DEBUG_DIRECTORY debugDir) {
    neko_assert(debugDir && imageBase);
    LPBYTE debugInfo = imageBase + debugDir->PointerToRawData;
    const auto debugInfoSize = debugDir->SizeOfData;
    if (debugInfo == 0 || debugInfoSize == 0) {
        return nullptr;
    }

    if (IsBadReadPtr(debugInfo, debugInfoSize)) {
        return nullptr;
    }

    if (debugInfoSize < sizeof(DebugInfoSignature)) {
        return nullptr;
    }

    if (debugDir->Type == IMAGE_DEBUG_TYPE_CODEVIEW) {
        auto signature = *(DWORD *)debugInfo;
        if (signature == NEKO_HOTLOAD_RSDS_SIGNATURE) {
            auto *info = (neko_hotload_rsds_hdr *)(debugInfo);
            if (IsBadReadPtr(debugInfo, sizeof(neko_hotload_rsds_hdr))) {
                return nullptr;
            }

            if (IsBadStringPtrA((const char *)info->filename, UINT_MAX)) {
                return nullptr;
            }

            return info->filename;
        }
    }

    return nullptr;
}

static bool neko_hotload_pdb_replace(const std::string &filename, const std::string &pdbname, std::string &orig_pdb) {
    neko_unicode_convert_path(_filename, filename);

    HANDLE fp = nullptr;
    HANDLE filemap = nullptr;
    LPVOID mem = 0;
    bool result = false;
    do {
        fp = CreateFile(_filename.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if ((fp == INVALID_HANDLE_VALUE) || (fp == nullptr)) {
            break;
        }

        filemap = CreateFileMapping(fp, nullptr, PAGE_READWRITE, 0, 0, nullptr);
        if (filemap == nullptr) {
            break;
        }

        mem = MapViewOfFile(filemap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
        if (mem == nullptr) {
            break;
        }

        auto dosHeader = struct_cast<PIMAGE_DOS_HEADER>(mem);
        if (dosHeader == 0) {
            break;
        }

        if (IsBadReadPtr(dosHeader, sizeof(IMAGE_DOS_HEADER))) {
            break;
        }

        if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
            break;
        }

        auto ntHeaders = struct_cast<PIMAGE_NT_HEADERS>(dosHeader, dosHeader->e_lfanew);
        if (ntHeaders == 0) {
            break;
        }

        if (IsBadReadPtr(ntHeaders, sizeof(ntHeaders->Signature))) {
            break;
        }

        if (ntHeaders->Signature != IMAGE_NT_SIGNATURE) {
            break;
        }

        if (IsBadReadPtr(&ntHeaders->FileHeader, sizeof(IMAGE_FILE_HEADER))) {
            break;
        }

        if (IsBadReadPtr(&ntHeaders->OptionalHeader, ntHeaders->FileHeader.SizeOfOptionalHeader)) {
            break;
        }

        if (ntHeaders->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC && ntHeaders->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
            break;
        }

        auto sectionHeaders = IMAGE_FIRST_SECTION(ntHeaders);
        if (IsBadReadPtr(sectionHeaders, ntHeaders->FileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER))) {
            break;
        }

        DWORD debugDirRva = 0;
        DWORD debugDirSize = 0;
        if (!neko_hotload_pe_debugdir_rva(&ntHeaders->OptionalHeader, debugDirRva, debugDirSize)) {
            break;
        }

        if (debugDirRva == 0 || debugDirSize == 0) {
            break;
        }

        DWORD debugDirOffset = 0;
        if (!neko_hotload_pe_fileoffset_rva(ntHeaders, debugDirRva, debugDirOffset)) {
            break;
        }

        auto debugDir = struct_cast<PIMAGE_DEBUG_DIRECTORY>(mem, debugDirOffset);
        if (debugDir == 0) {
            break;
        }

        if (IsBadReadPtr(debugDir, debugDirSize)) {
            break;
        }

        if (debugDirSize < sizeof(IMAGE_DEBUG_DIRECTORY)) {
            break;
        }

        int numEntries = debugDirSize / sizeof(IMAGE_DEBUG_DIRECTORY);
        if (numEntries == 0) {
            break;
        }

        for (int i = 1; i <= numEntries; i++, debugDir++) {
            char *pdb = neko_hotload_pdb_find((LPBYTE)mem, debugDir);
            if (pdb) {
                auto len = strlen(pdb);
                if (len >= strlen(pdbname.c_str())) {
                    orig_pdb = pdb;
                    memcpy_s(pdb, len, pdbname.c_str(), pdbname.length());
                    pdb[pdbname.length()] = 0;
                    result = true;
                }
            }
        }
    } while (0);

    if (mem != nullptr) {
        UnmapViewOfFile(mem);
    }

    if (filemap != nullptr) {
        CloseHandle(filemap);
    }

    if ((fp != nullptr) && (fp != INVALID_HANDLE_VALUE)) {
        CloseHandle(fp);
    }

    return result;
}

bool static neko_hotload_pdb_process(const std::string &desination) {
    std::string folder, fname, ext, orig_pdb;
    neko_hotload_split_path(desination, folder, fname, ext);
    bool result = neko_hotload_pdb_replace(desination, fname + ".pdb", orig_pdb);
    result &= neko_hotload_copy(orig_pdb, neko_hotload_replace_extension(desination, ".pdb"));
    return result;
}
#endif

static void neko_hotload_pe_section_save(neko_hotload_module &ctx, neko_hotload_module_section_type::e type, s64 vaddr, s64 base, IMAGE_SECTION_HEADER &shdr) {
    const auto version = neko_hotload_module_section_version::current;
    auto p = (neko_hotload_internal *)ctx.p;
    auto data = &p->data[type][version];
    const size_t old_size = data->size;
    data->base = base;
    data->ptr = (char *)vaddr;
    data->size = shdr.SizeOfRawData;
    data->data = neko_safe_realloc(data->data, shdr.SizeOfRawData);
    if (old_size < shdr.SizeOfRawData) {
        memset((char *)data->data + old_size, '\0', shdr.SizeOfRawData - old_size);
    }
}

static bool neko_hotload_module_validate_sections(neko_hotload_module &ctx, HMODULE handle, const std::string &imagefile, bool rollback) {
    (void)imagefile;
    neko_assert(handle);
    auto p = (neko_hotload_internal *)ctx.p;
    if (p->mode == NEKO_HOTLOAD_DISABLE) {
        return true;
    }
    auto ntHeaders = ImageNtHeader(handle);
    auto base = ntHeaders->OptionalHeader.ImageBase;
    auto sectionHeaders = (IMAGE_SECTION_HEADER *)(ntHeaders + 1);
    bool result = true;
    for (int i = 0; i < ntHeaders->FileHeader.NumberOfSections; ++i) {
        auto sectionHeader = sectionHeaders[i];
        const s64 size = sectionHeader.SizeOfRawData;
        if (!strcmp((const char *)sectionHeader.Name, ".state")) {
            if (ctx.version || rollback) {
                result &= neko_hotload_module_section_validate(ctx, neko_hotload_module_section_type::state, base + sectionHeader.VirtualAddress, base, size);
            }
            if (result) {
                auto sec = neko_hotload_module_section_type::state;
                neko_hotload_pe_section_save(ctx, sec, base + sectionHeader.VirtualAddress, base, sectionHeader);
            }
        } else if (!strcmp((const char *)sectionHeader.Name, ".bss")) {
            if (ctx.version || rollback) {
                result &= neko_hotload_module_section_validate(ctx, neko_hotload_module_section_type::bss, base + sectionHeader.VirtualAddress, base, size);
            }
            if (result) {
                auto sec = neko_hotload_module_section_type::bss;
                neko_hotload_pe_section_save(ctx, sec, base + sectionHeader.VirtualAddress, base, sectionHeader);
            }
        }
    }
    return result;
}

static void neko_hotload_so_unload(neko_hotload_module &ctx) {
    auto p = (neko_hotload_internal *)ctx.p;
    neko_assert(p->handle);
    FreeLibrary((HMODULE)p->handle);
}

static HMODULE neko_hotload_so_load(const std::string &filename) {
    neko_unicode_convert_path(_filename, filename);
    auto new_dll = LoadLibrary(_filename.c_str());
    if (!new_dll) {
        neko_log_error("Couldn't load module: %d", GetLastError());
    }
    return new_dll;
}

static neko_hotload_module_main_func neko_hotload_so_symbol(HMODULE handle) {
    neko_assert(handle);
    auto new_main = (neko_hotload_module_main_func)GetProcAddress(handle, NEKO_HOTLOAD_MAIN_FUNC);
    if (!new_main) {
        neko_log_error("Couldn't find module entry point: %d", GetLastError());
    }
    return new_main;
}

static void neko_hotload_plat_init() {}

static int neko_hotload_seh_filter(neko_hotload_module &ctx, unsigned long seh) {
    if (ctx.version == 1) {
        return EXCEPTION_CONTINUE_SEARCH;
    }

    ctx.version = ctx.last_working_version;
    switch (seh) {
        case EXCEPTION_ACCESS_VIOLATION:
            ctx.failure = NEKO_HOTLOAD_SEGFAULT;
            return EXCEPTION_EXECUTE_HANDLER;
        case EXCEPTION_ILLEGAL_INSTRUCTION:
            ctx.failure = NEKO_HOTLOAD_ILLEGAL;
            return EXCEPTION_EXECUTE_HANDLER;
        case EXCEPTION_DATATYPE_MISALIGNMENT:
            ctx.failure = NEKO_HOTLOAD_MISALIGN;
            return EXCEPTION_EXECUTE_HANDLER;
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
            ctx.failure = NEKO_HOTLOAD_BOUNDS;
            return EXCEPTION_EXECUTE_HANDLER;
        case EXCEPTION_STACK_OVERFLOW:
            ctx.failure = NEKO_HOTLOAD_STACKOVERFLOW;
            return EXCEPTION_EXECUTE_HANDLER;
        default:
            break;
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

static int neko_hotload_module_main(neko_hotload_module &ctx, neko_hotload_op operation) {
    auto p = (neko_hotload_internal *)ctx.p;

#ifdef _MSC_VER
    __try {
        if (p->main) {
            return p->main(&ctx, operation);
        }
    } __except (neko_hotload_seh_filter(ctx, GetExceptionCode())) {
        return -1;
    }
#else
#error "Only msvc"
#endif

    return -1;
}

#endif

static bool neko_hotload_module_load_internal(neko_hotload_module &ctx, bool rollback) {

    auto p = (neko_hotload_internal *)ctx.p;
    const auto file = p->fullname;
    if (neko_hotload_exists(file) || rollback) {
        const auto old_file = neko_hotload_version_path(file, ctx.version, p->temppath);
        neko_log_info("unload '%s' with rollback: %d", old_file.c_str(), rollback);
        int r = neko_hotload_module_unload(ctx, rollback, false);
        if (r < 0) {
            return false;
        }

        auto new_version = rollback ? ctx.version : ctx.next_version;
        auto new_file = neko_hotload_version_path(file, new_version, p->temppath);
        if (rollback) {
            if (ctx.version == 0) {
                ctx.failure = NEKO_HOTLOAD_INITIAL_FAILURE;
                return false;
            }

            ctx.last_working_version = ctx.version > 0 ? ctx.version - 1 : 0;
        } else {

            ctx.last_working_version = ctx.version;
            neko_hotload_copy(file, new_file);

            ctx.next_version = new_version + 1;

#if defined(_MSC_VER)
            if (!neko_hotload_pdb_process(new_file)) {
                neko_log_error("%s", "[NekoHotLoad] failed to load pdb");  // 无法处理 PDB 调试可能会受到影响或重新加载可能会失败
            }
#endif
        }

        auto new_dll = neko_hotload_so_load(new_file);
        if (!new_dll) {
            ctx.failure = NEKO_HOTLOAD_BAD_IMAGE;
            return false;
        }

        if (!neko_hotload_module_validate_sections(ctx, new_dll, new_file, rollback)) {
            return false;
        }

        if (rollback) {
            neko_hotload_module_sections_reload(ctx, neko_hotload_module_section_version::backup);
        } else if (ctx.version) {
            neko_hotload_module_sections_reload(ctx, neko_hotload_module_section_version::current);
        }

        auto new_main = neko_hotload_so_symbol(new_dll);
        if (!new_main) {
            return false;
        }

        auto p2 = (neko_hotload_internal *)ctx.p;
        p2->handle = new_dll;
        p2->main = new_main;
        if (ctx.failure != NEKO_HOTLOAD_BAD_IMAGE) {
            p2->timestamp = neko_hotload_last_write_time(file);
        }
        ctx.version = new_version;
        neko_log_info("loaded: %s (version: %d)", new_file.c_str(), ctx.version);
    } else {
        neko_log_error("%s", "error loading hotload module.");
        return false;
    }
    return true;
}

static bool neko_hotload_module_section_validate(neko_hotload_module &ctx, neko_hotload_module_section_type::e type, intptr_t ptr, intptr_t base, s64 size) {
    (void)ptr;
    auto p = (neko_hotload_internal *)ctx.p;
    switch (p->mode) {
        case NEKO_HOTLOAD_SAFE:
            return (p->data[type][0].size == size);
        case NEKO_HOTLOAD_UNSAFE:
            return (p->data[type][0].size <= size);
        case NEKO_HOTLOAD_DISABLE:
            return true;
        default:
            break;
    }

    return (p->data[type][0].base == base && p->data[type][0].size == size);
}

static void neko_hotload_module_sections_backup(neko_hotload_module &ctx) {
    auto p = (neko_hotload_internal *)ctx.p;
    if (p->mode == NEKO_HOTLOAD_DISABLE) {
        return;
    }

    for (int i = 0; i < neko_hotload_module_section_type::count; ++i) {
        auto cur = &p->data[i][neko_hotload_module_section_version::current];
        if (cur->ptr) {
            auto bkp = &p->data[i][neko_hotload_module_section_version::backup];
            bkp->data = neko_safe_realloc(bkp->data, cur->size);
            bkp->ptr = cur->ptr;
            bkp->size = cur->size;
            bkp->base = cur->base;

            if (bkp->data) {
                std::memcpy(bkp->data, cur->data, bkp->size);
            }
        }
    }
}

static void neko_hotload_module_sections_store(neko_hotload_module &ctx) {
    auto p = (neko_hotload_internal *)ctx.p;
    if (p->mode == NEKO_HOTLOAD_DISABLE) {
        return;
    }

    auto version = neko_hotload_module_section_version::current;
    for (int i = 0; i < neko_hotload_module_section_type::count; ++i) {
        if (p->data[i][version].ptr && p->data[i][version].data) {
            const char *ptr = p->data[i][version].ptr;
            const s64 len = p->data[i][version].size;
            std::memcpy(p->data[i][version].data, ptr, len);
        }
    }

    neko_hotload_module_sections_backup(ctx);
}

static void neko_hotload_module_sections_reload(neko_hotload_module &ctx, neko_hotload_module_section_version::e version) {
    neko_assert(version < neko_hotload_module_section_version::count);
    auto p = (neko_hotload_internal *)ctx.p;
    if (p->mode == NEKO_HOTLOAD_DISABLE) {
        return;
    }

    for (int i = 0; i < neko_hotload_module_section_type::count; ++i) {
        if (p->data[i][version].data) {
            const s64 len = p->data[i][version].size;

            const auto current = neko_hotload_module_section_version::current;
            auto dest = (void *)p->data[i][current].ptr;
            if (dest) {
                std::memcpy(dest, p->data[i][version].data, len);
            }
        }
    }
}

static void neko_hotload_so_sections_free(neko_hotload_module &ctx) {

    auto p = (neko_hotload_internal *)ctx.p;
    for (int i = 0; i < neko_hotload_module_section_type::count; ++i) {
        for (int v = 0; v < neko_hotload_module_section_version::count; ++v) {
            if (p->data[i][v].data) {
                neko_safe_free(p->data[i][v].data);
            }
            p->data[i][v].data = nullptr;
        }
    }
}

static bool neko_hotload_module_changed(neko_hotload_module &ctx) {
    auto p = (neko_hotload_internal *)ctx.p;
    const auto src = neko_hotload_last_write_time(p->fullname);
    const auto cur = p->timestamp;
    return src > cur;
}

static int neko_hotload_module_unload(neko_hotload_module &ctx, bool rollback, bool close) {

    auto p = (neko_hotload_internal *)ctx.p;
    int r = 0;
    if (p->handle) {
        if (!rollback) {
            r = neko_hotload_module_main(ctx, close ? NEKO_HOTLOAD_CLOSE : NEKO_HOTLOAD_UNLOAD);

            if (r < 0) {
                neko_log_info("[NekoHotLoad] failed: %d", r);
            } else {
                neko_hotload_module_sections_store(ctx);
            }
        }
        neko_hotload_so_unload(ctx);
        p->handle = nullptr;
        p->main = nullptr;
    }
    return r;
}

static bool neko_hotload_module_rollback(neko_hotload_module &ctx) {

    auto loaded = neko_hotload_module_load_internal(ctx, true);
    if (loaded) {
        loaded = neko_hotload_module_main(ctx, NEKO_HOTLOAD_LOAD) >= 0;
        if (loaded) {
            ctx.failure = NEKO_HOTLOAD_NONE;
        }
    }
    return loaded;
}

static void neko_hotload_module_reload(neko_hotload_module &ctx) {
    if (neko_hotload_module_changed(ctx)) {

        if (!neko_hotload_module_load_internal(ctx, false)) {
            return;
        }
        int r = neko_hotload_module_main(ctx, NEKO_HOTLOAD_LOAD);
        if (r < 0 && !ctx.failure) {
            neko_log_info("[NekoHotLoad] failed: %d", r);
            ctx.failure = NEKO_HOTLOAD_USER;
        }
    }
}

extern "C" int neko_hotload_module_update(neko_hotload_module &ctx, bool reloadCheck = true) {
    if (ctx.failure) {
        neko_log_info("[NekoHotLoad] rollback version was %d", ctx.version);
        neko_hotload_module_rollback(ctx);
        neko_log_info("[NekoHotLoad] rollback version is now %d", ctx.version);

        neko_hotload_plat_init();

    } else {
        if (reloadCheck) {
            neko_hotload_module_reload(ctx);
        }
    }

    if (ctx.failure) {
        neko_log_info("[NekoHotLoad] failed: -2");
        return -2;
    }

    int r = neko_hotload_module_main(ctx, NEKO_HOTLOAD_STEP);
    if (r < 0 && !ctx.failure) {
        neko_log_info("[NekoHotLoad] failed: NEKO_HOTLOAD_USER");
        ctx.failure = NEKO_HOTLOAD_USER;
    }
    return r;
}

extern "C" bool neko_hotload_module_open(neko_hotload_module &ctx, const char *fullpath) {

    neko_assert(fullpath);
    if (!neko_hotload_exists(fullpath)) {
        return false;
    }
    auto p = new (neko_safe_malloc(sizeof(neko_hotload_internal))) neko_hotload_internal;
    p->mode = NEKO_HOTLOAD_OP_MODE;
    p->fullname = fullpath;
    ctx.p = p;
    ctx.next_version = 1;
    ctx.last_working_version = 0;
    ctx.version = 0;
    ctx.failure = NEKO_HOTLOAD_NONE;
    neko_hotload_plat_init();
    return true;
}

extern "C" bool neko_hotload_module_load(neko_hotload_module &ctx, const char *fullpath) { return neko_hotload_module_open(ctx, fullpath); }

extern "C" void neko_hotload_module_close(neko_hotload_module &ctx) {

    const bool rollback = false;
    const bool close = true;
    neko_hotload_module_unload(ctx, rollback, close);
    neko_hotload_so_sections_free(ctx);
    auto p = (neko_hotload_internal *)ctx.p;

    const auto file = p->fullname;
    for (u32 i = 0; i < ctx.version; i++) {
        neko_hotload_del(neko_hotload_version_path(file, i, p->temppath));
#if defined(_MSC_VER)
        neko_hotload_del(neko_hotload_replace_extension(neko_hotload_version_path(file, i, p->temppath), ".pdb"));
#endif
    }

    p->~neko_hotload_internal();
    neko_safe_free(p);
    ctx.p = nullptr;
    ctx.version = 0;
}

#endif

#endif

#endif