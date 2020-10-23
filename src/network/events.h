#pragma once

#include "../math/smek_math.h"

// Created when a client is dropped so the main thread can do clean-up
struct DropClientEvent {
    u64 client_id;
    void callback();
};
