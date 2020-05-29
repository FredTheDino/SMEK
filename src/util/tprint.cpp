#include <stdio.h>
#include "tprint.h"

template<>
void tprint_helper<>(int recursion_limit, char **result, char **buffer, FormatHint *hint) {}

char format(char *buffer, FormatHint args, Vec3 a) {
    return sprintf(buffer, "(%.*f, %.*f, %.*f)", args.num_decimals, a.x,
                                                 args.num_decimals, a.y,
                                                 args.num_decimals, a.z);
}

char format(char *buffer, FormatHint args, f64 a) {
    return sprintf(buffer, "%.*f", args.num_decimals, a);
}

char format(char *buffer, FormatHint args, i32 a) {
    return sprintf(buffer, "%d", a);
}

char format(char *buffer, FormatHint args, u32 a) {
    return sprintf(buffer, "%u", a);
}

char format(char *buffer, FormatHint args, const char *a) {
    return sprintf(buffer, "%s", a);
}

u32 parse_format_string(const char **outputs, char *write, FormatHint *hint, u32 num_templates, const char *fmt) {
#define EAT *(write++) = *(fmt++)
#define SKIPP fmt++
    const char *original = fmt;
    u32 num_outputs = 0;
    if (num_templates)
        outputs[num_outputs] = write;
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
                    return -1;
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
                outputs[num_outputs] = ++write; // Leave a null terminator
            }
        } else {
            EAT;
        }
    }
#undef EAT
#undef SKIPP
    return num_outputs;
}

u32 concatenate_fmt_string_into(char *final_string, u32 num_concats, const char **padding, char **content) {
#define APPEND(str) \
    do { const char *read_head = str;\
        while (*read_head) *(write_head++) = *(read_head++);\
    } while(false)

    char *write_head = final_string;
    for (u32 i = 0; i < num_concats; i++) {
        APPEND(padding[i]);
        APPEND(content[i]);
    }
    APPEND(padding[num_concats]);
    *write_head = '\0';
#undef APPEND
    return write_head - final_string;
}

void smek_print(const char *buffer) {
    fprintf(stderr, buffer);
}
