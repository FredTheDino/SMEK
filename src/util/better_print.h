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
    LOG("%s", __PRETTY_FUNCTION__);
    return sprintf(buffer, "%d", a);
}

template<typename... Args>
void template_print(int recursion_limit, const char **result, char **buffer, PrintArgs **printing, Args...);

template<typename T, typename... Args>
void template_print_helper(int recursion_limit, char **result, char **buffer, PrintArgs *printing, T t, Args... rest) {
    if (recursion_limit == 0) return;
    result[0] = *buffer;
    LOG("called %p", buffer);
    *buffer += format(*buffer, *printing, t) + 1;
    LOG("called %p %d", result, t);
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
    char buffer[512]; // Arbitrarily choosen
    constexpr u32 num_templates = template_length<Args...>();
    PrintArgs args[num_templates] = {};
    char *formatted[num_templates] = {};

    u32 num_outputs = 0;

    char *ptr = buffer;
    template_print(num_outputs, formatted, &ptr, args, to_print...);
    ASSERT(ptr - buffer < LEN(buffer), "Buffer overrun!");
    for (u32 i = 0; i < num_outputs; i++) {
        LOG("%s", formatted[i]);
    }
}
