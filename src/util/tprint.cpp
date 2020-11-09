#include <stdio.h>
#include "tprint.h"
#include "../game.h"

i32 format(char *buffer, u32 size, FormatHint args, f64 a) {
    return snprintf(buffer, size, "%0*.*f", args.num_zero_pad, args.num_decimals, a);
}

i32 format(char *buffer, u32 size, FormatHint args, u64 a) {
    if (args.hex) return snprintf(buffer, size, "0x%lX", a);
    return snprintf(buffer, size, "%0*lu", args.num_zero_pad, a);
}

i32 format(char *buffer, u32 size, FormatHint args, i64 a) {
    if (args.hex) return snprintf(buffer, size, "0x%lX", a);
    return snprintf(buffer, size, "%0*ld", args.num_zero_pad, a);
}

i32 format(char *buffer, u32 size, FormatHint args, i32 a) {
    if (args.hex) return snprintf(buffer, size, "0x%X", a);
    return snprintf(buffer, size, "%0*d", args.num_zero_pad, a);
}

i32 format(char *buffer, u32 size, FormatHint args, u32 a) {
    if (args.hex) return snprintf(buffer, size, "0x%X", a);
    return snprintf(buffer, size, "%0*u", args.num_zero_pad, a);
}

i32 format(char *buffer, u32 size, FormatHint args, i16 a) {
    if (args.hex) return snprintf(buffer, size, "0x%X", a);
    return snprintf(buffer, size, "%0*d", args.num_zero_pad, a);
}

i32 format(char *buffer, u32 size, FormatHint args, u16 a) {
    if (args.hex) return snprintf(buffer, size, "0x%X", a);
    return snprintf(buffer, size, "%0*u", args.num_zero_pad, a);
}

i32 format(char *buffer, u32 size, FormatHint args, char a) {
    if (args.hex) return snprintf(buffer, size, "0x%X", a);
    return snprintf(buffer, size, "%c", a);
}

i32 format(char *buffer, u32 size, FormatHint args, const char *a) {
    return snprintf(buffer, size, "%s", a);
}

void smek_print(const char *buffer, FILE *stream) {
    fprintf(stream, buffer);
}

u32 smek_snprint(char *out_buffer, u32 buf_size, const char *in_buffer) {
    return snprintf(out_buffer, buf_size, in_buffer);
}

template <>
i32 sntprint<>(char *buffer, u32 buf_size, const char *fmt) {
    if (buf_size == 0) return 0;
    u32 write = 0;
    u32 head = 0;
    for (; fmt[head] && head != buf_size; head++) {
        if (fmt[head] == '{') {
            WARN("Invalid format string, unexpected '{}'", fmt + head);
        }
        if (fmt[head] == '!' && fmt[head + 1] == '{') {
            head++;
        }
        buffer[write++] = fmt[head];
    }
    if (head == buf_size) {
        buffer[write - 1] = '\0';
    } else {
        buffer[write] = '\0';
    }
    return write;
}

#include "../test.h"
#include <cstring>

TEST_FORMAT((f32)1.5, "1.50", .num_decimals = 2);
TEST_FORMAT((f32)1.5, "1.5", .num_decimals = 1);
TEST_FORMAT((f32)1.45, "1.5", .num_decimals = 1);
TEST_FORMAT((f32)1.44, "1.4", .num_decimals = 1);

TEST_FORMAT((f64)1.44, "1.4", .num_decimals = 1);
TEST_FORMAT((f64)1.44, "1.44", .num_decimals = 2);

TEST_FORMAT((f32)1.5, "1.50", .num_decimals = 2, .num_zero_pad = 4);
TEST_FORMAT((f32)1.5, "01.50", .num_decimals = 2, .num_zero_pad = 5);
TEST_FORMAT((u32)5000, "5000", .num_zero_pad = 4);
TEST_FORMAT((u32)5000, "05000", .num_zero_pad = 5);
TEST_FORMAT((i32)-5000, "-5000", .num_zero_pad = 5);
TEST_FORMAT((i32)-5000, "-05000", .num_zero_pad = 6);

TEST_FORMAT((char)'a', "a", .num_zero_pad = 6);

TEST_FORMAT((const char *)"abc123", "abc123", .num_zero_pad = 6);

TEST_CASE("reuse sntprint buffer - base case", {
    char buffer[4] = {};
    sntprint(buffer, LEN(buffer), "1");
    ASSERT(std::strcmp(buffer, "1") == 0, "Got '{}', '1' expected", buffer);
    sntprint(buffer, LEN(buffer), "22");
    ASSERT(std::strcmp(buffer, "22") == 0, "Got '{}', '22' expected", buffer);
    sntprint(buffer, LEN(buffer), "3");
    ASSERT(std::strcmp(buffer, "3") == 0, "Got '{}', '3' expected", buffer);
    return true;
});

TEST_CASE("reuse sntprint buffer - base case at buf_size", {
    char buffer[3] = {};
    sntprint(buffer, LEN(buffer), "1");
    ASSERT(std::strcmp(buffer, "1") == 0, "Got '{}', '1' expected", buffer);
    sntprint(buffer, LEN(buffer), "22");
    ASSERT(std::strcmp(buffer, "22") == 0, "Got '{}', '22' expected", buffer);
    sntprint(buffer, LEN(buffer), "3");
    ASSERT(std::strcmp(buffer, "3") == 0, "Got '{}', '3' expected", buffer);
    return true;
});

TEST_CASE("reuse sntprint buffer - base case over buf_size", {
    char buffer[2] = {};
    sntprint(buffer, LEN(buffer), "1");
    ASSERT(std::strcmp(buffer, "1") == 0, "Got '{}', '1' expected", buffer);
    sntprint(buffer, LEN(buffer), "22");
    ASSERT(std::strcmp(buffer, "2") == 0, "Got '{}', '2' expected", buffer);
    sntprint(buffer, LEN(buffer), "3");
    ASSERT(std::strcmp(buffer, "3") == 0, "Got '{}', '3' expected", buffer);
    return true;
});

TEST_CASE("reuse sntprint buffer - with formatting", {
    char buffer[4] = {};
    sntprint(buffer, LEN(buffer), "{}", 1);
    ASSERT(std::strcmp(buffer, "1") == 0, "Got '{}', '1' expected", buffer);
    sntprint(buffer, LEN(buffer), "{}", 22);
    ASSERT(std::strcmp(buffer, "22") == 0, "Got '{}', '22' expected", buffer);
    sntprint(buffer, LEN(buffer), "{}", 3);
    ASSERT(std::strcmp(buffer, "3") == 0, "Got '{}', '3' expected", buffer);
    return true;
});

TEST_CASE("reuse sntprint buffer - with formatting at buf_size", {
    char buffer[3] = {};
    sntprint(buffer, LEN(buffer), "{}", 1);
    ASSERT(std::strcmp(buffer, "1") == 0, "Got '{}', '1' expected", buffer);
    sntprint(buffer, LEN(buffer), "{}", 22);
    ASSERT(std::strcmp(buffer, "22") == 0, "Got '{}', '22' expected", buffer);
    sntprint(buffer, LEN(buffer), "{}", 3);
    ASSERT(std::strcmp(buffer, "3") == 0, "Got '{}', '3' expected", buffer);
    return true;
});

TEST_CASE("reuse sntprint buffer - with formatting over buf_size", {
    char buffer[2] = {};
    sntprint(buffer, LEN(buffer), "{}", 1);
    ASSERT(std::strcmp(buffer, "1") == 0, "Got '{}', '1' expected", buffer);
    sntprint(buffer, LEN(buffer), "{}", 22);
    ASSERT(std::strcmp(buffer, "2") == 0, "Got '{}', '2' expected", buffer);
    sntprint(buffer, LEN(buffer), "{}", 3);
    ASSERT(std::strcmp(buffer, "3") == 0, "Got '{}', '3' expected", buffer);
    return true;
});

#define SNTPRINT_TEST(name, buflen, exp, fmt, ...)                                       \
    TEST_CASE("sntprint - " name, {                                                      \
        GAMESTATE()->logger.levels &= ~LogLevel::WARNING;                                \
        char buf[buflen] = {};                                                           \
        const char *fmt_str = fmt;                                                       \
        const char *exp_str = exp;                                                       \
        sntprint(buf, buflen, fmt_str, __VA_ARGS__);                                     \
        ASSERT(std::strcmp(buf, exp_str) == 0, "Got '{}', '{}' expected", buf, exp_str); \
        return true;                                                                     \
    })

SNTPRINT_TEST("extra fmt #1", 4, "1{}", "{}{}", 1);

SNTPRINT_TEST("extra fmt #2", 4, "2{}", "{}{}{}{}{}{}", 2);

SNTPRINT_TEST("escape fmt #1", 15, "3-{}2-1{}", "{}-!{}{}-{}!{}", 3, 2, 1);

SNTPRINT_TEST("escape fmt #2", 15, "3!-{}2-1!", "{}!-!{}{}-{}!", 3, 2, 1);

SNTPRINT_TEST("escape fmt #3", 15, "{}1!", "!{}{}!", 1);

SNTPRINT_TEST("escape fmt #4", 15, "!{}! 1!!", "!!{}! {}!!", 1);

SNTPRINT_TEST("escape fmt #5", 15, "{}1!-2!", "!{}{}!-{}!", 1, 2);
