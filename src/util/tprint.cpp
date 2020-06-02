#include <stdio.h>
#include "tprint.h"

i32 format(char *buffer, u32 size, FormatHint args, f64 a) {
    return snprintf(buffer, size, "%0*.*f", args.num_zero_pad, args.num_decimals, a);
}

i32 format(char *buffer, u32 size, FormatHint args, u64 a) {
    return snprintf(buffer, size, "%0*lu", args.num_zero_pad, a);
}

i32 format(char *buffer, u32 size, FormatHint args, i64 a) {
    return snprintf(buffer, size, "%0*ld", args.num_zero_pad, a);
}

i32 format(char *buffer, u32 size, FormatHint args, i32 a) {
    return snprintf(buffer, size, "%0*d", args.num_zero_pad, a);
}

i32 format(char *buffer, u32 size, FormatHint args, u32 a) {
    return snprintf(buffer, size, "%0*u", args.num_zero_pad, a);
}

i32 format(char *buffer, u32 size, FormatHint args, i16 a) {
    return snprintf(buffer, size, "%0*d", args.num_zero_pad, a);
}

i32 format(char *buffer, u32 size, FormatHint args, u16 a) {
    return snprintf(buffer, size, "%0*u", args.num_zero_pad, a);
}

i32 format(char *buffer, u32 size, FormatHint args, char a) {
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

template<>
i32 sntprint<>(char *buffer, u32 buf_size, const char *fmt) {
    u32 head = 0;
    while (fmt[head]) {
        if (buf_size == head) { buffer[head-1] = '\0'; return -1; }
        buffer[head] = fmt[head];
        head++;
    }
    return head;
}

#include "../test.h"

TEST_FORMAT((f32) 1.5,  "1.50", .num_decimals=2);
TEST_FORMAT((f32) 1.5,  "1.5",  .num_decimals=1);
TEST_FORMAT((f32) 1.45, "1.5",  .num_decimals=1);
TEST_FORMAT((f32) 1.44, "1.4",  .num_decimals=1);

TEST_FORMAT((f64) 1.44, "1.4",  .num_decimals=1);
TEST_FORMAT((f64) 1.44, "1.44", .num_decimals=2);

TEST_FORMAT((f32) 1.5,   "1.50",   .num_decimals=2, .num_zero_pad=4);
TEST_FORMAT((f32) 1.5,   "01.50",  .num_decimals=2, .num_zero_pad=5);
TEST_FORMAT((u32) 5000,  "5000",   .num_zero_pad=4);
TEST_FORMAT((u32) 5000,  "05000",  .num_zero_pad=5);
TEST_FORMAT((i32) -5000, "-5000",  .num_zero_pad=5);
TEST_FORMAT((i32) -5000, "-05000", .num_zero_pad=6);

TEST_FORMAT((char) 'a', "a", .num_zero_pad=6);

TEST_FORMAT((const char *) "abc123", "abc123", .num_zero_pad=6);
