#pragma once

template<typename... Args>
constexpr int template_length();

template<typename T, typename... Args>
constexpr int template_length_helper() {
    return 1 + template_length<Args...>();
}

template<typename... Args>
constexpr int template_length() {
    return template_length_helper<Args...>();
}

template<>
constexpr int template_length<>() {
    return 0;
}

struct FormatHint {
    int num_decimals;
};

#include <stdio.h>
char format(char *buffer, FormatHint args, double a) {
    return sprintf(buffer, "%.*f", args.num_decimals, a);
}

char format(char *buffer, FormatHint args, int a) {
    return sprintf(buffer, "%d", a);
}

template<typename... Args>
void tprint_helper(int recursion_limit, const char **result, char **buffer, FormatHint **hint, Args...);

template<typename T, typename... Args>
void tprint_helper_inner(int recursion_limit, char **result, char **buffer, FormatHint *hint, T t, Args... rest) {
    if (recursion_limit == 0) return;
    result[0] = *buffer;
    *buffer += format(*buffer, *hint, t) + 1;
    tprint_helper(recursion_limit - 1, result + 1, buffer, hint + 1, rest...);
}

template<typename... Args>
void tprint_helper(int recursion_limit, char **result, char **buffer, FormatHint *hint, Args... args) {
    tprint_helper_inner(recursion_limit, result, buffer, hint, args...);
}

template<>
void tprint_helper<>(int recursion_limit, char **result, char **buffer, FormatHint *hint) {}

template<typename... Args>
void tprint(const char *fmt, Args... to_print) {
    constexpr u32 num_templates = template_length<Args...>();
    FormatHint hint[num_templates] = {};
    char *formatted[num_templates] = {};
    char format_buffer[512] = {}; // Arbitrarily choosen


    const char *original = fmt;
    char write_buffer[512] = {};
    char *write = write_buffer;
    const char *spaces[num_templates + 1] = {};
    u32 num_outputs = 0;

#define EAT *(write++) = *(fmt++)
#define SKIPP fmt++
    // Parses the
    spaces[0] = write;
    while (*fmt) {
        if (*fmt == '%') {
            if (*(fmt + 1) == '{') {
                fmt++;
            }
            EAT;
        } else if (*fmt == '{') {
            SKIPP;
            while (*fmt != '}') {
                if (!*fmt) {
                    WARN("Invalid format string, unclosed {} in format string.'%s'", original);
                    return;
                }
                if (*fmt == '.') {
                    SKIPP;
                    if ('0' <= *fmt && *fmt <= '9') {
                        hint[num_outputs].num_decimals = *fmt - '0';
                        SKIPP;
                    } else {
                        WARN("Expected number litteral in format string after '.', got '%c'.", *fmt);
                        SKIPP;
                        continue;
                    }
                } else {
                    WARN("Unexepected symbol in format string '%c'.", *fmt);
                    SKIPP;
                }
            }
            SKIPP;
            num_outputs++;
            if (num_outputs <= num_templates) {
                spaces[num_outputs] = ++write; // Leave a null terminator
            }
        } else {
            EAT;
        }
    }
#undef EAT
#undef SKIPP
    CHECK(num_outputs == num_templates, "Wrong #{} in fmt string, expected %d, got %d", num_templates, num_outputs);
    char *ptr = format_buffer;
    tprint_helper(num_outputs, formatted, &ptr, hint, to_print...);
    ASSERT((u32) (ptr - format_buffer) < LEN(format_buffer), "Buffer overrun!");

#define APPEND while (*read_head) *(write_head++) = *(read_head++)
    char final_buffer[512];
    char *write_head = final_buffer;
    const char *read_head;
    for (u32 i = 0; i < num_templates; i++) {
        read_head = spaces[i];
        APPEND;
        read_head = formatted[i];
        APPEND;
    }
    read_head = spaces[num_templates];
    APPEND;
    *write_head = '\0';
#undef APPEND

    fprintf(stderr, final_buffer);
}
