#pragma once
#include "../math/smek_math.h"
#include "../math/smek_vec.h"
#include "../util/util.h"

///* FormatHint
// The extra information passed to each format.
struct FormatHint;

struct FormatHint {
    int num_decimals;
};

//
// This is where your overloads for format goes. They can also go in
// the header files where each of the structs are defined, but this
// is the function that should be overloaded.
//

char format(char *buffer, FormatHint args, Vec3 a);
char format(char *buffer, FormatHint args, f64 a);
char format(char *buffer, FormatHint args, i64 a);
char format(char *buffer, FormatHint args, u64 a);
char format(char *buffer, FormatHint args, i32 a);
char format(char *buffer, FormatHint args, u32 a);
char format(char *buffer, FormatHint args, i16 a);
char format(char *buffer, FormatHint args, u16 a);
char format(char *buffer, FormatHint args, const char *a);

//
// Templates to get the length of an argument list. This is
// also a simpler version of the code bellow.
//

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

//
// These functions are various wrappers for the code to play nice with the C++ template
// system, but the code calls the overloaded format() function on all arguments.
//

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
void tprint_helper<>(int recursion_limit, char **result, char **buffer, FormatHint *hint);

///*
// Parses a format string. Places where you want the values inserted should be marked with the "{}" token,
// and arguments for printing are given inside the "{}", so to write out with 3 decimals write "{.3}".
u32 parse_format_string(const char **outputs, char *write, FormatHint *hint, u32 num_templates, const char *fmt);

///*
// Concatenates the strings in padding and content into one string, returns
// the size of the new string. One extra padding string is added at the end,
// padding[num_concats], which might cause segfaults if used poorly.
u32 concatenate_fmt_string_into(char *final_string, u32 num_concats, const char **padding, char **content);

///*
// A helper function to skip including stdio.h everywhere.
void smek_print(const char *buffer);

u32 smek_snprint(char *out_buffer, u32 buf_size, const char *in_buffer);

///*
// A magical print function which writes out anything you throw at it,
// given of course that is has a format() overload.
template<typename... Args>
void tprint(const char *fmt, Args... to_print);

template<typename... Args>
u32 sntprint(char *buffer, u32 buf_size, const char *fmt, Args... to_print);

template<typename... Args>
void tprint(const char *fmt, Args... to_print) {
    constexpr u32 num_templates = template_length<Args...>();
    if (num_templates == 0) {
        smek_print(fmt);
        return;
    }
    FormatHint hint[num_templates] = {};
    char *formatted[num_templates] = {};
    char format_buffer[512] = {}; // Arbitrarily choosen


    char write_buffer[512] = {};
    const char *spaces[num_templates + 1] = {};
    u32 num_outputs = parse_format_string(spaces, write_buffer, hint, num_templates, fmt);
    CHECK(num_outputs == num_templates, "Wrong #%{} in fmt string, expected {}, got {} ('{}')",
                                        num_templates, num_outputs, fmt);
    if (num_outputs == (u32) -1) return;

    char *ptr = format_buffer;
    tprint_helper(num_outputs, formatted, &ptr, hint, to_print...);
    ASSERT((u32) (ptr - format_buffer) < LEN(format_buffer), "Buffer overrun!");

    char final_string[512];
    u32 written = concatenate_fmt_string_into(final_string, Math::min(num_templates, num_outputs),
                                              spaces, formatted);
    ASSERT(written < LEN(final_string), "Buffer overrun!");
    tprint(final_string);
}

template<typename... Args>
u32 sntprint(char *buffer, u32 buf_size, const char *fmt, Args... to_print) {
    constexpr u32 num_templates = template_length<Args...>();
    if (num_templates == 0) {
        return smek_snprint(buffer, buf_size, fmt);
    }
    FormatHint hint[num_templates] = {};
    char *formatted[num_templates] = {};
    char format_buffer[512] = {}; // Arbitrarily choosen

    char write_buffer[512] = {};
    const char *spaces[num_templates + 1] = {};
    u32 num_outputs = parse_format_string(spaces, write_buffer, hint, num_templates, fmt);
    CHECK(num_outputs == num_templates, "Wrong #%{} in fmt string, expected {}, got {} ('{}')",
                                        num_templates, num_outputs, fmt);
    if (num_outputs == (u32) -1) return 0;

    char *ptr = format_buffer;
    tprint_helper(num_outputs, formatted, &ptr, hint, to_print...);
    ASSERT((u32) (ptr - format_buffer) < LEN(format_buffer), "Buffer overrun!");

    char final_string[512];
    u32 written = concatenate_fmt_string_into(final_string, Math::min(num_templates, num_outputs),
                                              spaces, formatted);
    ASSERT(written < LEN(final_string), "Buffer overrun!");
    return sntprint(buffer, buf_size, final_string);
}
