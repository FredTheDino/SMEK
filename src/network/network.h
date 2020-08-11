#pragma once

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "SDL.h"

#include "package.h"
#include "../math/smek_math.h"
#include "../util/util.h"

struct NetworkHandle {
    bool active = false;
    bool is_server_handle;
    u32 id;
    SDL_Thread *thread;
    char thread_name[32] = {};
    int sockfd;

    bool creating_package_to_send = true;
    Package wip_package;

    void send(u8 *data, u32 data_len);
    void send(Package *package);

    void close();
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
    Package prev_package;
    SDL_mutex *m_prev_package;

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
