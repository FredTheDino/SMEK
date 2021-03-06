#pragma once
#include "../math/smek_math.h"
// includes "util.h" further down

///*
// The extra information passed to each format.
struct FormatHint {
    bool hex;
    int num_decimals;
    int num_zero_pad;
};

//
// This is where your overloads for format goes. They can also go in
// the header files where each of the structs are defined, but this
// is the function that should be overloaded.
//

i32 format(char *buffer, u32 size, FormatHint args, f64 a);
i32 format(char *buffer, u32 size, FormatHint args, i64 a);
i32 format(char *buffer, u32 size, FormatHint args, u64 a);
i32 format(char *buffer, u32 size, FormatHint args, i32 a);
i32 format(char *buffer, u32 size, FormatHint args, u32 a);
i32 format(char *buffer, u32 size, FormatHint args, i16 a);
i32 format(char *buffer, u32 size, FormatHint args, u16 a);
i32 format(char *buffer, u32 size, FormatHint args, char a);
i32 format(char *buffer, u32 size, FormatHint args, const char *a);

///*
// Helper functions to skip including stdio.h everywhere.
void smek_print(const char *buffer, FILE *stream = stderr);
u32 smek_snprint(char *out_buffer, u32 buf_size, const char *in_buffer);

///*
// A magical print function which writes out anything you throw at it,
// given of course that is has a format() overload.
template <typename... Args>
void tprint(const char *fmt, Args... to_print);

template <typename... Args>
i32 sntprint(char *buffer, u32 buf_size, const char *fmt, Args... rest);

template <typename... Args>
void ftprint(FILE *stream, const char *fmt, Args... to_print);

template <typename T, typename... Args>
i32 sntprint_helper(char *buffer, u32 buf_size, const char *fmt, T first, Args... rest);

// This needs to be here, to allow circular includes
#include "../util/util.h"

template <typename... Args>
void tprint(const char *fmt, Args... to_print) {
    char buffer[512] = {};
    sntprint(buffer, 512, fmt, to_print...);
    smek_print(buffer);
}

template <typename... Args>
void ftprint(FILE *stream, const char *fmt, Args... to_print) {
    char buffer[512] = {};
    sntprint(buffer, 512, fmt, to_print...);
    smek_print(buffer, stream);
}

template <>
i32 sntprint<>(char *buffer, u32 buf_size, const char *fmt);

template <typename... Args>
i32 sntprint(char *buffer, u32 buf_size, const char *fmt, Args... rest) {
    return sntprint_helper(buffer, buf_size, fmt, rest...);
}

template <typename T, typename... Args>
i32 sntprint_helper(char *buffer, u32 buf_size, const char *fmt, T first, Args... rest) {
    u32 head = 0;

    if (!buf_size) { return 0; }

#define EAT                          \
    do {                             \
        if (head == buf_size) {      \
            buffer[head - 1] = '\0'; \
            return head - 1;         \
        }                            \
        buffer[head++] = *(fmt++);   \
    } while (false);

#define SKIP fmt++

    while (*fmt) {
        if (*fmt == '!') {
            if (*(fmt + 1) == '{') { SKIP; }
            EAT;
        } else if (*fmt == '{') {
            SKIP;
            FormatHint hint; // Setting some sane defaults
            hint.num_decimals = 5;
            hint.num_zero_pad = 0;
            hint.hex = false;
            while (*fmt != '}') {
                if (!*fmt) {
                    WARN("Invalid format string");
                    return -1;
                }
                if (*fmt == 'x') {
                    SKIP;
                    hint.hex = true;
                } else if (*fmt == '.') {
                    SKIP;
                    if ('0' <= *fmt && *fmt <= '9') {
                        hint.num_decimals = *fmt - '0';
                        SKIP;
                    } else {
                        WARN("Expected number literal in format string after '.', got '{}'.", *fmt);
                        SKIP;
                        continue;
                    }
                } else if (*fmt == '0') {
                    SKIP;
                    if ('0' <= *fmt && *fmt <= '9') {
                        hint.num_zero_pad = *fmt - '0';
                        SKIP;
                    } else {
                        WARN("Expected number literal in format string after '0', got '{}'.", *fmt);
                        SKIP;
                        continue;
                    }
                } else {
                    WARN("Unexepected symbol in format string '{}'.", *fmt);
                    SKIP;
                }
            }
            SKIP;
            i32 format_write = format(buffer + head, buf_size - head, hint, first);
            head += format_write;
            if (head == buf_size - 1) { return head; }
            return head + sntprint(buffer + head, buf_size - head, fmt, rest...);
        } else {
            EAT;
        }
    }
    WARN("Unread arguments.");
    return head;
#undef EAT
#undef SKIP
}
