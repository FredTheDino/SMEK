#include <stdio.h>
#include "tprint.h"

i32 format(char *buffer, u32 size, FormatHint args, Vec3 a) {
    int b = 0;
    return snprintf(buffer, size, "(%.*f, %.*f, %.*f)", args.num_decimals, a.x,
                                                 args.num_decimals, a.y,
                                                 args.num_decimals, a.z);
}

i32 format(char *buffer, u32 size, FormatHint args, f64 a) {
    return snprintf(buffer, size, "%.*f", args.num_decimals, a);
}

i32 format(char *buffer, u32 size, FormatHint args, u64 a) {
    return snprintf(buffer, size, "%lu", a);
}

i32 format(char *buffer, u32 size, FormatHint args, i64 a) {
    return snprintf(buffer, size, "%ld", a);
}

i32 format(char *buffer, u32 size, FormatHint args, i32 a) {
    return snprintf(buffer, size, "%d", a);
}

i32 format(char *buffer, u32 size, FormatHint args, u32 a) {
    return snprintf(buffer, size, "%u", a);
}

i32 format(char *buffer, u32 size, FormatHint args, i16 a) {
    return snprintf(buffer, size, "%d", a);
}

i32 format(char *buffer, u32 size, FormatHint args, u16 a) {
    return snprintf(buffer, size, "%u", a);
}

i32 format(char *buffer, u32 size, FormatHint args, const char *a) {
    return snprintf(buffer, size, "%s", a);
}

void smek_print(const char *buffer) {
    fprintf(stderr, buffer);
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

