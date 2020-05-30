#include "log.h"
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
    void* addrlist[max_frames+1];

    int addrlen = backtrace(addrlist, sizeof(addrlist) / sizeof(void*));

    // filename(function+address)
    char **symbollist = backtrace_symbols(addrlist, addrlen);

    size_t funcnamesize = 256;
    char *funcname = new char[funcnamesize];

    // Iterate over the returned symbol lines. The first is this function,
    // the second is either _assert or _unreachable. The last two are _start
    // and libc_start.
    for (int i = 2; i < addrlen-2; i++) {
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
        }
        else {
            // Couldn't parse the line? print the whole line.
            std::fprintf(STREAM, "%s\n", symbollist[i]);
        }
    }
    std::free(symbollist);
    delete[] funcname;
}

