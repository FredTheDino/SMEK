#include "log.h"
#include "../game.h"
#include "imgui/imgui.h"
#include <cstdio>
#include <cstring>

void _smek_log(const char *message, LogMessage log) {
    Logger *logger = &GAMESTATE()->logger;
    char print_buffer[LOG_BUFFER_SIZE] = {}; // with color
    char file_buffer[LOG_BUFFER_SIZE] = {};  // without color
    u32 print_len = 0;
    u32 file_len = 0;

    std::memcpy(&log.message, message, LOG_BUFFER_SIZE);

    if (log.level & logger->levels) {
        if (LogLevel::ERROR & log.level) {
            print_len += sntprint(print_buffer, LOG_BUFFER_SIZE, RED    "E ");
        } else if (LogLevel::WARNING & log.level) {
            print_len += sntprint(print_buffer, LOG_BUFFER_SIZE, YELLOW "W ");
        } else if (LogLevel::INFO & log.level) {
            print_len += sntprint(print_buffer, LOG_BUFFER_SIZE, WHITE  "I ");
        } else if (LogLevel::TRACE & log.level) {
            print_len += sntprint(print_buffer, LOG_BUFFER_SIZE, RESET  "T ");
        }
        print_len += sntprint(print_buffer + print_len, LOG_BUFFER_SIZE - print_len,
                              "{}" RESET " @ {} ({}:{}): {}\n",
                              log.file,
                              log.line,
                              log.func,
                              log.thread,
                              log.message);
        smek_print(print_buffer);
    }
    if (logger->file && (log.level & logger->levels_file)) {
        if (LogLevel::ERROR & log.level) {
            file_len += sntprint(file_buffer, LOG_BUFFER_SIZE, "E ");
        } else if (LogLevel::WARNING & log.level) {
            file_len += sntprint(file_buffer, LOG_BUFFER_SIZE, "W ");
        } else if (LogLevel::INFO & log.level) {
            file_len += sntprint(file_buffer, LOG_BUFFER_SIZE, "I ");
        } else if (LogLevel::TRACE & log.level) {
            file_len += sntprint(file_buffer, LOG_BUFFER_SIZE, "T ");
        }
        file_len += sntprint(file_buffer + file_len, LOG_BUFFER_SIZE - file_len,
                             "{} @ {} ({}:{}): {}\n",
                             log.file,
                             log.line,
                             log.func,
                             log.thread,
                             log.message);
        std::fprintf(logger->file, file_buffer);
    }
    if (logger->m_logs) {
        if (SDL_LockMutex(logger->m_logs) == 0) {
            logger->logs.push_back(log);
            SDL_UnlockMutex(logger->m_logs);
        } else {
            logger->m_logs = nullptr;
            ERR("Logger mutex broken");
        }
    }
}

void Logger::imgui_draw() {
#ifdef IMGUI_ENABLE
    ImGui::Begin("Messages");
    for (const auto &message : GAMESTATE()->logger.logs) {
        ImGui::Text("%s", message.message);
    }
    ImGui::End();
#endif
}

#ifdef WINDOWS
void print_stacktrace(unsigned int max_frames) {
    ERR("Unavailable on Windows");
}
#else
#include "color.h"
#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <stdexcept>
#include <execinfo.h>
#include <cxxabi.h>
#include <unistd.h>

#define STREAM stderr

// Kinda borrowed from https://panthema.net/2008/0901-stacktrace-demangled/
void print_stacktrace(unsigned int max_frames) {
    void *addrlist[max_frames + 1];

    int addrlen = backtrace(addrlist, sizeof(addrlist) / sizeof(void *));

    // filename(function+address)
    char **symbollist = backtrace_symbols(addrlist, addrlen);

    size_t funcnamesize = 256;
    char *funcname = new char[funcnamesize];

    // Iterate over the returned symbol lines. The first is this function,
    // the second is either _assert or _unreachable. The last two are _start
    // and libc_start.
    for (int i = 2; i < addrlen - 2; i++) {
        char *begin_name = 0, *begin_offset = 0, *end_offset = 0;

        // Find parentheses and +address offset surrounding the mangled name:
        // ./module(function+0x15c) [0x8048a6d]
        for (char *p = symbollist[i]; *p; ++p) {
            if (*p == '(') {
                begin_name = p;
            } else if (*p == '+') {
                begin_offset = p;
            } else if (*p == ')' && begin_offset) {
                end_offset = p;
                break;
            }
        }

        if (begin_name && begin_offset && end_offset && begin_name < begin_offset) {
            *begin_name++ = '\0';
            *begin_offset++ = '\0';
            *end_offset = '\0';

            // Mangled name is in [begin_name, begin_offset) and caller offset
            // is in [begin_offset, end_offset). Now apply __cxa_demangle().

            int status;
            char *ret = abi::__cxa_demangle(begin_name,
                                            funcname, &funcnamesize, &status);
            if (status == 0) {
                funcname = ret; // Use possibly realloc()-ed string
                std::fprintf(STREAM, "  %s\n", funcname);
            } else {
                // Demangling failed. Output function name as a C function with no arguments.
                std::fprintf(STREAM, "  %s()\n", begin_name);
            }
        } else {
            // Couldn't parse the line? print the whole line.
            std::fprintf(STREAM, "%s\n", symbollist[i]);
        }
    }
    std::free(symbollist);
    delete[] funcname;
}

#endif
