#include "network.h"
#include "util/util.h"

#include <errno.h>
#include <cstring>

void pack(Package *package, u8 *into) {
    std::memcpy(into, package, sizeof(Package));
}

Package unpack(u8 *from) {
    Package package;
    std::memcpy(&package, from, sizeof(Package));
    return package;
}

void log_pkg(Package package) {
    switch (package.type) {
    case PackageType::A:
        LOG("A: a={}", package.PKG_A.a);
        break;
    case PackageType::B:
        LOG("B: a={} b={}", package.PKG_B.a, package.PKG_B.b);
        break;
    default:
        LOG("Unknown type");
        break;
    }
}

void NetworkHandle::send(u8 *data, u32 data_len) {
    int n = write(sockfd, data, data_len);
    if (n < 0) {
        ERR("Error writing to socket, errno: {}", errno);
    } else if ((u32) n < data_len) {
        ERR("write did not write all data to socket, n={}, data_len={}", n, data_len);
    }
}

void NetworkHandle::send(Package *package) {
    u8 buf[sizeof(Package)];
    pack(package, buf);
    send(buf, sizeof(Package));
}

int start_network_handle(void *data) {
    NetworkHandle *handle = (NetworkHandle *) data;
    handle->active = true;
    int n;
    u8 buf[sizeof(Package)];
    while (handle->active) {
        n = read(handle->sockfd, buf, sizeof(Package));
        if (n == 0) {
            LOG("Connection closed by remote");
            break;
        } else if (n < 0) {
            ERR("Error reading from socket, errno: {}", errno);
            continue;
        }
        log_pkg(unpack(buf));
    }
    handle->active = false;
    close(handle->sockfd);
    return 0;
}

bool Network::connect_to_server(char *hostname, int portno) {
    if (server_handle.active) {
        WARN("Already connected to server");
        return false;
    }
    server_handle.sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_handle.sockfd < 0) {
        ERR("Error opening socket");
        return false;
    }
    hostent *server = gethostbyname(hostname);
    if (!server) {
        ERR("No such host");
        return false;
    }
    sockaddr_in serv_addr = {};
    serv_addr.sin_family = AF_INET;
    std::memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    serv_addr.sin_port = htons(portno);
    if (::connect(server_handle.sockfd, (sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        WARN("Unable to connect to server");
        return false;
    }
    LOG("Connected");
    server_handle.active = true;
    return true;
}

void Network::disconnect_from_server() {
    if (!server_handle.active) {
        WARN("Not connected to a server");
        return;
    }
    close(server_handle.sockfd);
    server_handle.active = false;
}

#if 0
void Network::stop() {
    if (listening) {
        listening = false;
        close(listen_sockfd);
        SDL_WaitThread(listener_thread, NULL);
    }

    for (u32 i = 0; i < MAX_CLIENTS; i++) {
        if (client_handles[i].active) {
            //TODO(gu) send restart notice to clients
            client_handles[i].active = false;
            close(client_handles[i].sockfd);
            SDL_WaitThread(client_handles[i].thread, NULL);
        }
    }
}
#endif

bool Network::setup_server(int portno) {
    if (server_listening) {
        WARN("Server already running");
        return false;
    }
    LOG("Setting up server on port {}", portno);
    listen_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serv_addr = {};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(listen_sockfd, (sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        ERR("Error binding to socket");
        return false;
    }
    listen(listen_sockfd, 5);
    cli_len = sizeof(cli_addr);
    listener_thread = SDL_CreateThread(network_listen_for_clients, "ListenForClients", (void *) this);
    if (!listener_thread) {
        ERR("Unable to create thread");
        return false;
    }
    return true;
}

bool Network::new_client_handle(int newsockfd) {
    NetworkHandle *handle;
    for (u32 i = 0; i < MAX_CLIENTS; i++) {
        handle = client_handles + i;
        if (handle->active) continue;
        LOG("Found client handle");
        *handle = {};
        handle->sockfd = newsockfd;
        handle->thread = SDL_CreateThread(start_network_handle, "ClientHandle", (void *) handle);
        if (!handle->thread) {
            ERR("Unable to create thread");
            return false;
        }
        return true;
    }
    WARN("No available client handles, ignoring");
    return false;
}

int network_listen_for_clients(void *data) {
    Network *system = (Network *) data;
    int newsockfd;
    system->server_listening = true;
    while (system->server_listening) {
        newsockfd = accept(system->listen_sockfd, (sockaddr *) &system->cli_addr, &system->cli_len);
        LOG("New client connected");
        if (newsockfd < 0) {
            ERR("Error accepting client connection, errno={}", errno);
            continue;
        }
        if (!system->new_client_handle(newsockfd)) {
            ERR("Unable to accept client connection, closing");
            //TODO(gu) send reason
            close(newsockfd);
        }
    }
    return 0;
}
