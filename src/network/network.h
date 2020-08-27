#pragma once

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <forward_list>
#include <chrono>
#include "SDL.h"

#include "package.h"
#include "../math/smek_math.h"
#include "../util/util.h"

struct NetworkHandle {
    bool active = false;
    SDL_Thread *thread;
    char thread_name[32] = {};
    int sockfd;

    u32 client_id;
    u32 next_package_id = 0;

    bool creating_package_to_send = false;
    Package wip_package;
    WipEntities wip_entities;

    std::forward_list<Package> package_log;

    void send(u8 *data, u32 data_len);
    void send(Package *package);
    bool recv(u8 *buf, u32 data_len, Package *package);

    void handle_package(Package *package);

    void close();
};

struct ServerHandle : public NetworkHandle {
    using steady_clock = std::chrono::steady_clock;
    steady_clock::time_point heartbeat_time_start;
    u32 prev_heartbeat_id;

    void send_heartbeat();
    void recv_heartbeat(u32 id);
};
int start_server_handle(void *data); // thread entry point, takes a (ServerHandle *)

struct ClientHandle : public NetworkHandle {};
int start_client_handle(void *data); // thread entry point, takes a (ClientHandle *)

struct Network {
    static const u32 MAX_CLIENTS = 2;

    bool server_listening = false;
    SDL_Thread *listener_thread;

    // server variables
    int listen_sockfd;
    sockaddr_in cli_addr;
    socklen_t cli_len;
    Package prev_package;
    SDL_mutex *m_prev_package;

    u32 next_handle_id = 0;
    ServerHandle server_handle;
    ClientHandle client_handles[MAX_CLIENTS];

    bool setup_server(int portno);
    void stop_server();
    bool connect_to_server(char *hostname, int portno);
    void disconnect_from_server();

    bool new_client_handle(int newsockfd);

#ifndef IMGUI_DISABLE
    void imgui_draw();
#endif
};

int network_listen_for_clients(void *data); // thread entry point
