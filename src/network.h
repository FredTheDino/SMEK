#pragma once

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "SDL.h"

#include "math/smek_math.h"

enum class PackageType {
    A,
    B,

    NUM_TYPES,
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

void pack(Package package, u8 *into);
Package unpack(u8 *from);

void log_pkg(Package package);

struct NetworkHandle {
    bool active = false;
    SDL_Thread *thread;
    int sockfd;

    void send(u8 *data, u32 data_len);  // send data to the handle
    void send(Package package);
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

    NetworkHandle server_handle;
    NetworkHandle client_handles[MAX_CLIENTS];

    bool setup_server(int portno);
    bool connect_to_server(char *hostname, int portno);

    bool new_client_handle(int newsockfd);
};

int network_listen_for_clients(void *data);  // thread entry point
