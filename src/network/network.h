#pragma once

///# Actual networking
// The networking lets instances communicate with each other through a central
// server. The networking in and of itself only sends packages to a single
// client/server, leaving most of the handling of the package content to the
// relevant systems.
//
// The networking contains quite a few threads so remember to watch your step.

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <forward_list>
#include <chrono>
#include "SDL.h"

#include "package.h"
#include "events.h"
#include "../math/smek_math.h"
#include "../util/util.h"

struct NetworkHandle {
    bool active = false;
    SDL_Thread *thread;
    char thread_name[32] = {};
    int sockfd;

    u64 client_id;
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
    static const u64 HANDLE_ID_FIRST_BIT = 0x0100000000000000;
    static const u32 MAX_CLIENTS = 2;

    bool autostart_server = false;
    bool autostart_client = false;
    char *client_server_addr;
    int autostart_port = 8888;

    bool server_listening = false;
    SDL_Thread *listener_thread;

    // server variables
    int listen_sockfd;
    sockaddr_in cli_addr;
    socklen_t cli_len;
    Package prev_package;
    SDL_mutex *m_prev_package;
    u64 next_handle_id = HANDLE_ID_FIRST_BIT;

    ServerHandle server_handle;
    ClientHandle client_handles[MAX_CLIENTS];

    bool setup_server(int portno);
    void stop_server();
    bool connect_to_server(char *hostname, int portno);
    void disconnect_from_server();

    bool new_client_handle(int newsockfd);

    void send_state_to_server();
    void send_state_to_clients();

#ifdef IMGUI_ENABLE
    void imgui_draw();
#endif
};

int network_listen_for_clients(void *data); // thread entry point

#if 0

///*
// A network handle is either a client's representation of its server or vice
// versa. A server keeps one ClientHandle per connected client, while a client
// keeps one (and only one) ServerHandle. (This naming might feel a bit
// backwards at first.) The handle listens for packages on a separate thread.
struct NetworkHandle {
    bool active = false;
    SDL_Thread *thread;
    // ...
    void send(u8 *data, u32 data_len);
    void send(Package *package);
    bool recv(u8 *buf, u32 data_len, Package *package);
    void handle_package(Package *package);
    void close();
};

///*
// Send an array of bytes to the handle. The bytes are expected to contain a
// correctly formatted Package.
void NetworkHandle::send(u8 *data, u32 data_len);

///*
// Send a Package to the server/client.
void NetworkHandle::send(Package *package);

///*
// Receive a Package from the server/client and store it at a Package pointer.
bool NetworkHandle::recv(u8 *buf, u32 data_len, Package *package);

///*
// Handle (i.e. do something with) the package stored at a pointer.
// Usually this will put an event on the event queue but might do something
// different for more special packages.
void NetworkHandle::handle_package(Package *package);

///*
// Close the underlying socket.
void NetworkHandle::close();

///*
// A client's representation of its connected server.
struct ServerHandle : public NetworkHandle {
    void send_heartbeat();
    void recv_heartbeat(u32 id);
};

///*
// Send a heartbeat to the server. Every heartbeat contains a unique ID that
// the server will immediately respond with.
void ServerHandle::send_heartbeat();

///*
// Receive and handle a received heartbeat with some ID. If the ID doesn't
// match the previous sent ID it is assumed that a package has gotten lost
// or there's something very wrong going on.
void ServerHandle::recv_heartbeat(u32 id);

///*
// The entry point for the thread listening for packages sent <i>by</i> the
// server <i>to</i> the client.
int start_server_handle(void *data);

///*
// A server's representation of one of its connected clients.
struct ClientHandle : NetworkHandle {};

///*
// The entry point for the thread listening for packages sent <i>by</i> the
// client <i>to</i> the server.
int start_client_handle(void *data);

///*
// The struct containing everything relevant to networking.
struct Network {
    static const u32 MAX_CLIENTS = ..;
    SDL_Thread *listener_thread;
    ServerHandle server_handle;
    ClientHandle client_handles[MAX_CLIENTS];
    bool setup_server(int portno);
    void stop_server();
    bool connect_to_server(char *hostname, int portno);
    void disconnect_from_server();
    bool new_client_handle(int newsockfd);
};

///*
// Setup the server on the specified port and start listening for new clients.
// Returns false if the setup fails for some reason, most probably that the
// port is already in use.
bool Network::setup_server(int portno);

///*
// Stop listening for new clients and disconnect all connected clients.
void Network::stop_server();

///*
bool Network::connect_to_server(char *hostname, int portno);

///*
void Network::disconnect_from_server();

///*
// Setup a new client handle communicating over the specified socket.
// This is called when a client tries to connect to the server.
// Returns false if all client handles are taken.
bool Network::new_client_handle(int newsockfd);

///*
// The entry point for the thread listening for new clients. When a new client
// tries to connect the thread tries to pair it to a ClientHandle on a new
// thread. 
int network_listen_for_clients(void *data);

#endif
