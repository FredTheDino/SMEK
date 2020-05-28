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

struct PrintArgs {
    int num_decimals;
};

#include <stdio.h>
char format(char *buffer, PrintArgs args, int a) {
    return sprintf(buffer, "%d", a);
}

template<typename... Args>
void template_print(int recursion_limit, const char **result, char **buffer, PrintArgs **printing, Args...);

template<typename T, typename... Args>
void template_print_helper(int recursion_limit, char **result, char **buffer, PrintArgs *printing, T t, Args... rest) {
    if (recursion_limit == 0) return;
    result[0] = *buffer;
    *buffer += format(*buffer, *printing, t) + 1;
    template_print(recursion_limit - 1, result + 1, buffer, printing + 1, rest...);
}

template<typename... Args>
void template_print(int recursion_limit, char **result, char **buffer, PrintArgs *printing, Args... args) {
    template_print_helper(recursion_limit, result, buffer, printing, args...);
}

template<>
void template_print<>(int recursion_limit, char **result, char **buffer, PrintArgs *printing) {}

template<typename... Args>
void better_print(const char *fmt, Args... to_print) {
    char buffer[512] = {}; // Arbitrarily choosen
    constexpr u32 num_templates = template_length<Args...>();
    PrintArgs args[num_templates] = {};
    char *formatted[num_templates] = {};

    u32 num_outputs = 0;

    "Hello {.2} world!";
    const char *original = fmt;
    char write_buffer[512] = {};
    char *write = write_buffer;
    const char *spaces[num_templates + 1] = {};
#define EAT *(write++) = *(fmt++)
#define SKIPP fmt++

    spaces[0] = write;
    while (*fmt) {
        LOG("%c", *fmt);
        if (*fmt == '%') {
            if (*(fmt + 1) == '{') {
                fmt++;
            }
            EAT;
        } else if (*fmt == '{') {
            while (*fmt != '}') {
                if (!*fmt) {
                    WARN("Invalid format string, unclosed {} in format string.'%s'", original);
                    return;
                }
                if (*fmt == '.') {
                    SKIPP;
                    if (*fmt >= '0' && '9' <= *fmt) {
                        // TODO(ed): We only read one number, can be expanded if needed.
                        args[num_outputs].num_decimals = *fmt - '9';
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
            spaces[++num_outputs] = write++; // Leave a null terminator
        } else {
            EAT;
        }
    }
    LOG("s: %s", spaces[0]);
    CHECK(num_outputs == num_templates, "Wrong #{} in fmt string, expected %d, got %d", num_templates, num_outputs);
    char *ptr = buffer;
    template_print(num_outputs, formatted, &ptr, args, to_print...);
    ASSERT(ptr - buffer < LEN(buffer), "Buffer overrun!");

#define APPEND while (*read_head) *(write_head++) = *(read_head++)

    char final_buffer[512];
    char *write_head = final_buffer;
    const char *read_head = spaces[0];
    APPEND;
    for (u32 i = 0; i < num_outputs; i++) {
        read_head = formatted[i];
        APPEND;
        read_head = spaces[i];
        APPEND;
    }
    *write_head = '\0';
    LOG("%s", final_buffer);
}
