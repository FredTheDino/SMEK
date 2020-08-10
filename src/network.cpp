#include "network.h"
#include "util/util.h"

#include <errno.h>
#include <cstring>
#include "imgui/imgui.h"

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
            LOG("{}: Connection closed by remote", handle->thread_name);
            break;
        } else if (n < 0) {
            ERR("{}: Error reading from socket, errno: {}", handle->thread_name, errno);
            continue;
        }
        log_pkg(unpack(buf));
    }
    LOG("{}: Setting active to FALSE", handle->thread_name);
    handle->active = false;
    close(handle->sockfd);
    return 0;
}

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
        ERR("Error binding to socket, errno={}", errno);
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

void Network::stop_server() {
    if (!server_listening) {
        WARN("Server not running");
        return;
    }
    server_listening = false;
    shutdown(listen_sockfd, SHUT_RDWR);
    SDL_WaitThread(listener_thread, NULL);
    close(listen_sockfd);
    for (u32 i = 0; i < MAX_CLIENTS; i++) {
        if (client_handles[i].active) {
            client_handles[i].active = false;
            shutdown(client_handles[i].sockfd, SHUT_RDWR);
            close(client_handles[i].sockfd);
            SDL_WaitThread(client_handles[i].thread, NULL);
        }
    }
    LOG("Server stopped");
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
    server_handle.thread = SDL_CreateThread(start_network_handle, "ServerHandle", (void *) &server_handle);
    if (!server_handle.thread) {
        ERR("Unable to create thread");
        return false;
    }
    return true;
}

void Network::disconnect_from_server() {
    if (!server_handle.active) {
        WARN("Not connected to a server");
        return;
    }
    server_handle.active = false;
    shutdown(server_handle.sockfd, SHUT_RDWR);
    close(server_handle.sockfd);
    SDL_WaitThread(server_handle.thread, NULL);
}

bool Network::new_client_handle(int newsockfd) {
    NetworkHandle *handle;
    for (u32 i = 0; i < MAX_CLIENTS; i++) {
        handle = client_handles + i;
        if (handle->active) continue;
        LOG("Found client handle");
        *handle = {};
        handle->sockfd = newsockfd;
        sntprint(handle->thread_name, sizeof(handle->thread_name), "ClientHandle {}", next_handle_id++);
        handle->thread = SDL_CreateThread(start_network_handle, handle->thread_name, (void *) handle);
        if (!handle->thread) {
            ERR("Unable to create thread");
            return false;
        }
        return true;
    }
    WARN("No available client handles, ignoring");
    return false;
}

#ifndef IMGUI_DISABLE
void Network::imgui_draw() {
    ImGui::Begin("Networking");
    {
        static int serverport = 8888;
        ImGui::SetNextItemWidth(150);
        ImGui::PushID(&serverport);
        ImGui::InputInt("port", &serverport);
        ImGui::PopID();
        if (ImGui::Button("Create server")) {
            setup_server(serverport);
        }
        if (server_listening) {
            ImGui::SameLine();
            if (ImGui::Button("Stop server")) {
                stop_server();
            }
        }

        ImGui::Separator();

        static char ip[64] = "127.0.0.1";
        static int connectport = 8888;
        ImGui::SetNextItemWidth(150);
        ImGui::InputText("server address", ip, IM_ARRAYSIZE(ip));
        ImGui::SetNextItemWidth(150);
        ImGui::PushID(&connectport);
        ImGui::InputInt("port", &connectport);
        ImGui::PopID();
        if (ImGui::Button("Connect to server")) {
            connect_to_server(ip, connectport);
        }
        if (server_handle.active) {
            //TODO(gu) Generate instead of writing manually. Custom imgui widget?
            static PackageType package_type = PackageType::B;
            static int a_a = 0;
            static int b_a = 0;
            static int b_b = 0;

            static int type_current_id = 0;
            ImGui::Combo("", &type_current_id, "A\0B\0\0");
            
            Package package;
            package.type = (PackageType) type_current_id;
            switch (package.type) {
            case PackageType::A:
                ImGui::InputInt("A::a", &a_a);
                package.PKG_A.a = a_a;
                break;
            case PackageType::B:
                ImGui::InputInt("B::a", &b_a);
                ImGui::InputInt("B::b", &b_b);
                package.PKG_B.a = b_a;
                package.PKG_B.b = b_b;
                break;
            default:
                break;
            }

            if (ImGui::Button("Send")) {
                server_handle.send(&package);
            }

            if (ImGui::Button("Disconnect")) {
                disconnect_from_server();
            }
        }
    }
    ImGui::End();
}
#endif

int network_listen_for_clients(void *data) {
    Network *system = (Network *) data;
    int newsockfd;
    system->server_listening = true;
    while (system->server_listening) {
        newsockfd = accept(system->listen_sockfd, (sockaddr *) &system->cli_addr, &system->cli_len);
        if (newsockfd < 0) {
            ERR("ListenForClients: Error accepting client connection, errno={}", errno);
            continue;
        }
        LOG("ListenForClients: New client connected");
        if (!system->new_client_handle(newsockfd)) {
            ERR("ListenForClients: Unable to accept client connection, closing");
            //TODO(gu) send reason
            close(newsockfd);
        }
    }
    LOG("Stopped listening for new clients");
    return 0;
}
