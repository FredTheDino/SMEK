#include <stdio.h>
#include "better_print.h"

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

