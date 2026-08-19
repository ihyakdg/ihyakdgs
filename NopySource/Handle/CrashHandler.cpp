// CrashHandler.cpp (Windows / Linux support)
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#include <dbghelp.h>
#pragma comment(lib, "Dbghelp.lib")
#endif

#include <cstdio>
#include <cstdint>
#include <ctime>
#include <string>
#include <mutex>
#include <cstdarg>
#include <cstring>
#include <exception>
#include <cstdlib>

#ifdef _WIN32

static std::mutex g_crash_mtx;

static void ensure_logs_dir() {
    CreateDirectoryA("logs", nullptr);
}

static std::string now_str() {
    std::time_t t = std::time(nullptr);
    std::tm tm{};
    localtime_s(&tm, &t);
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
        tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
        tm.tm_hour, tm.tm_min, tm.tm_sec);
    return buf;
}

static void write_line(FILE* f, const char* s) {
    std::fputs(s, f);
    std::fputc('\n', f);
}

static void writef(FILE* f, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    std::vfprintf(f, fmt, ap);
    std::fputc('\n', f);
    va_end(ap);
}

static bool sym_init_once(HANDLE process) {
    static bool inited = false;
    static bool ok = false;
    static std::mutex m;
    std::lock_guard<std::mutex> lk(m);
    if (inited) return ok;

    DWORD opts = SymGetOptions();
    opts |= SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES;
    SymSetOptions(opts);

    char exePath[MAX_PATH]{};
    GetModuleFileNameA(nullptr, exePath, MAX_PATH);

    std::string exeDir = exePath;
    size_t pos = exeDir.find_last_of("\\/");
    if (pos != std::string::npos) exeDir = exeDir.substr(0, pos + 1);
    else exeDir = ".\\";

    std::string symPath = ".;";
    symPath += exeDir;

    ok = SymInitialize(process, symPath.c_str(), TRUE) ? true : false;
    inited = true;
    return ok;
}

static void dump_stack_to_file(FILE* f, CONTEXT* ctx) {
    HANDLE process = GetCurrentProcess();
    HANDLE thread = GetCurrentThread();

    if (!sym_init_once(process)) {
        write_line(f, "SymInitialize failed (DbgHelp). File:line may be unavailable.");
    }

    STACKFRAME64 frame{};
#if defined(_M_X64)
    DWORD machine = IMAGE_FILE_MACHINE_AMD64;
    frame.AddrPC.Offset = ctx->Rip;
    frame.AddrPC.Mode = AddrModeFlat;
    frame.AddrFrame.Offset = ctx->Rbp;
    frame.AddrFrame.Mode = AddrModeFlat;
    frame.AddrStack.Offset = ctx->Rsp;
    frame.AddrStack.Mode = AddrModeFlat;
#elif defined(_M_IX86)
    DWORD machine = IMAGE_FILE_MACHINE_I386;
    frame.AddrPC.Offset = ctx->Eip;
    frame.AddrPC.Mode = AddrModeFlat;
    frame.AddrFrame.Offset = ctx->Ebp;
    frame.AddrFrame.Mode = AddrModeFlat;
    frame.AddrStack.Offset = ctx->Esp;
    frame.AddrStack.Mode = AddrModeFlat;
#else
    write_line(f, "Unsupported architecture for StackWalk64.");
    return;
#endif

    write_line(f, "---- CALLSTACK ----");

    for (int i = 0; i < 128; i++) {
        BOOL ok = StackWalk64(
            machine, process, thread,
            &frame, ctx, nullptr,
            SymFunctionTableAccess64,
            SymGetModuleBase64,
            nullptr
        );
        if (!ok || frame.AddrPC.Offset == 0) break;

        DWORD64 addr = frame.AddrPC.Offset;

        char modName[MAX_PATH] = "?";
        DWORD64 modBase = SymGetModuleBase64(process, addr);
        if (modBase) {
            GetModuleFileNameA((HMODULE)modBase, modName, MAX_PATH);
            const char* base = std::strrchr(modName, '\\');
            if (!base) base = std::strrchr(modName, '/');
            if (base) {
                size_t len = std::strlen(base + 1);
                std::memmove(modName, base + 1, len);
                modName[len] = 0;
            }
        }

        char symBuf[sizeof(SYMBOL_INFO) + 256]{};
        SYMBOL_INFO* sym = (SYMBOL_INFO*)symBuf;
        sym->SizeOfStruct = sizeof(SYMBOL_INFO);
        sym->MaxNameLen = 255;

        DWORD64 dispSym = 0;
        const char* fnName = "?";
        if (SymFromAddr(process, addr, &dispSym, sym)) fnName = sym->Name;

        IMAGEHLP_LINE64 line{};
        line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
        DWORD dispLine = 0;

        if (SymGetLineFromAddr64(process, addr, &dispLine, &line)) {
            const char* file = line.FileName ? line.FileName : "";
            DWORD lineNo = line.LineNumber;
            writef(f, "#%02d %s!%s +0x%llx (%s:%lu)",
                i, modName, fnName,
                (unsigned long long)dispSym,
                file, (unsigned long)lineNo
            );
        }
        else {
            writef(f, "#%02d %s!%s +0x%llx (0x%llx)",
                i, modName, fnName,
                (unsigned long long)dispSym,
                (unsigned long long)addr
            );
        }
    }
}

static void write_crash_report(EXCEPTION_POINTERS* ep, CONTEXT* fallback_ctx, const char* origin) {
    std::lock_guard<std::mutex> lk(g_crash_mtx);
    ensure_logs_dir();

    FILE* f = nullptr;
    fopen_s(&f, "logs/callstack.txt", "wb");
    if (!f) return;

    writef(f, "Time: %s", now_str().c_str());
    writef(f, "Origin: %s", origin ? origin : "?");

    if (ep && ep->ExceptionRecord) {
        DWORD code = ep->ExceptionRecord->ExceptionCode;
        void* addr = ep->ExceptionRecord->ExceptionAddress;
        writef(f, "ExceptionCode: 0x%08X", (unsigned)code);
        writef(f, "ExceptionAddress: 0x%p", addr);

        switch (code) {
        case EXCEPTION_ACCESS_VIOLATION: write_line(f, "Hint: Access violation (NULL/dangling pointer, OOB, use-after-free)."); break;
        case EXCEPTION_INT_DIVIDE_BY_ZERO: write_line(f, "Hint: Divide by zero."); break;
        case EXCEPTION_STACK_OVERFLOW: write_line(f, "Hint: Stack overflow (deep recursion / huge stack alloc)."); break;
        default: write_line(f, "Hint: See exception code; root cause usually in top frames."); break;
        }
    }
    else {
        write_line(f, "No EXCEPTION_POINTERS (terminate/purecall/invalid-parameter/etc).");
    }

    CONTEXT* ctx = nullptr;
    if (ep && ep->ContextRecord) ctx = ep->ContextRecord;
    else ctx = fallback_ctx;

    if (ctx) dump_stack_to_file(f, ctx);
    else write_line(f, "No context available to dump stack.");

    std::fflush(f);
    std::fclose(f);
}

static LONG WINAPI UnhandledExceptionFilterImpl(EXCEPTION_POINTERS* ep) {
    write_crash_report(ep, nullptr, "UnhandledExceptionFilter");
    return EXCEPTION_EXECUTE_HANDLER;
}

static void capture_and_report(const char* origin, DWORD exitCode) {
    CONTEXT ctx{};
    ctx.ContextFlags = CONTEXT_ALL;
    RtlCaptureContext(&ctx);
    write_crash_report(nullptr, &ctx, origin);
    ::TerminateProcess(GetCurrentProcess(), exitCode);
}

static void TerminateHandler() {
    capture_and_report("std::terminate", 0xDEAD);
}

static void InvalidParameterHandler(
    const wchar_t*,
    const wchar_t*,
    const wchar_t*,
    unsigned int,
    uintptr_t
) {
    capture_and_report("CRT invalid parameter", 0xBADC0DE);
}

static void PureCallHandler() {
    capture_and_report("purecall", 0x50555245); // 'PURE'
}

static void InstallCrashHandler() {
    SetUnhandledExceptionFilter(UnhandledExceptionFilterImpl);
    std::set_terminate(TerminateHandler);
    _set_invalid_parameter_handler(InvalidParameterHandler);
    _set_purecall_handler(PureCallHandler);
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);
}
#else
static void InstallCrashHandler() {}
#endif