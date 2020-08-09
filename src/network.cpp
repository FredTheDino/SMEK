#include "network.h"
#include "util/util.h"

#include <errno.h>
#include <cstring>

void NetworkHandle::send(u8 *data, u32 data_len) {
    LOG("Sending some data");
    CHECK(write(sockfd, data, data_len) >= 0, "Error writing to socket");
}

int start_network_handle(void *data) {
    NetworkHandle *handle = (NetworkHandle *) data;
    handle->active = true;
    int n;
    u8 buf[1];
    LOG("Listening on new network handle");
    while (handle->active) {
        std::memset(buf, 0, sizeof(buf));
        n = read(handle->sockfd, buf, sizeof(buf));
        if (n < 0) {
            UNREACHABLE("Error reading from socket, errno: {}", errno);
            continue;
        }
        LOG("Received package");
        //TODO(gu) unpack package etc
    }
    close(handle->sockfd);
    return 0;
}

bool Network::connect_to_server(char *hostname, int portno) {
    LOG("Connecting to server at {}:{}", hostname, portno);
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
    return true;
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
    LOG("Setting up server at port {}", portno);
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
    LOG("Looking for available client handle");
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
        LOG("Client handle thread started");
        return true;
    }
    WARN("No available client handles, ignoring");
    return false;
}

int network_listen_for_clients(void *data) {
    Network *system = (Network *) data;
    int newsockfd;
    system->listening = true;
    LOG("Listening for new clients");
    while (system->listening) {
        newsockfd = accept(system->listen_sockfd, (sockaddr *) &system->cli_addr, &system->cli_len);
        LOG("New client connection");
        if (newsockfd < 0) {
            ERR("Error accepting client connection");
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
