#pragma once

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "SDL.h"

#include "math/smek_math.h"
#include "util/util.h"

enum class PackageType {
    A,
    B,

    NUM_TYPES,
};

static const char *package_type_list[] = {
    "A",
    "B",
};

struct PackageA {
    int a;
};

struct PackageB {
    int b;
    int a;
};

struct Package {
    PackageType type;
    union {
        PackageA PKG_A;
        PackageB PKG_B;
    };
};

void pack(Package *package, u8 *into);
Package unpack(u8 *from);

i32 format(char *buffer, u32 size, FormatHint args, Package pkg);

struct NetworkHandle {
    bool active = false;
    SDL_Thread *thread;
    char thread_name[32] = {};
    int sockfd;

    void send(u8 *data, u32 data_len);
    void send(Package *package);
};

int start_network_handle(void *data);  // thread entry point

struct Network {
    static const u32 MAX_CLIENTS = 2;

    bool server_listening = false;
    SDL_Thread *listener_thread;

    // server variables
    int listen_sockfd;
    sockaddr_in cli_addr;
    socklen_t cli_len;

    u32 next_handle_id = 0;
    NetworkHandle server_handle;
    NetworkHandle client_handles[MAX_CLIENTS];

    bool setup_server(int portno);
    void stop_server();
    bool connect_to_server(char *hostname, int portno);
    void disconnect_from_server();

    bool new_client_handle(int newsockfd);

#ifndef IMGUI_DISABLE
    void imgui_draw();
#endif
};

int network_listen_for_clients(void *data);  // thread entry point
