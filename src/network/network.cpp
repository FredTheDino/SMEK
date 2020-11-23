#include "network.h"
#include "../util/util.h"
#include "../game.h"
#include "../entity/entity_types.h"
#include "../test.h"

#include <errno.h>
#include <cstring>
#include "imgui/imgui.h"

void NetworkHandle::send(u8 *data, u32 data_len) {
    u32 data_written = 0;
    while (data_written < data_len) {
        int n = write(sockfd, data + data_written, data_len - data_written);
        if (n < 0) {
            UNREACHABLE("Error writing to socket, errno: {}", errno);
        }
        data_written += n;
    }
}

void NetworkHandle::send(Package *package) {
    u8 buf[sizeof(Package)] = {};
    package->header.client = client_id;
    package->header.id = next_package_id++;
    pack(buf, package);
    send(buf, sizeof(Package));
}

void NetworkHandle::close() {
    if (!active) return;
    active = false;
    shutdown(sockfd, SHUT_RDWR);
    SDL_WaitThread(thread, NULL);
    ::close(sockfd);
}

bool NetworkHandle::recv(u8 *buf, u32 data_len, Package *package) {
    u32 data_read = 0;
    while (data_read < data_len) {
        int n = read(sockfd, buf + data_read, data_len - data_read);
        if (n < 0) {
            UNREACHABLE("{}: Error reading from socket, errno={}", thread_name, errno); //TODO do something smart
        } else if (n == 0) {
            INFO("{}: Connection closed", thread_name);
            active = false;
            return false;
        }
        data_read += n;
    }
    unpack(package, buf);
    //package_log.push_front(*package);
    handle_package(package);
    return true;
    return false;
}

void NetworkHandle::handle_package(Package *package) {
    switch (package->header.type) {
    case PackageType::EVENT:
        ASSERT(SDL_LockMutex(GAMESTATE()->m_event_queue) == 0, "Failed to aquire lock");
        GAMESTATE()->event_queue.push(package->EVENT.event);
        SDL_UnlockMutex(GAMESTATE()->m_event_queue);
        break;
    default:
        break;
    }
}

void ServerHandle::send_heartbeat() {
    Package heartbeatPackage;
    heartbeatPackage.header.type = PackageType::HEARTBEAT;
    heartbeatPackage.HEARTBEAT.id = next_package_id;
    heartbeat_time_start = steady_clock::now();
    send(&heartbeatPackage);
    prev_heartbeat_id = heartbeatPackage.header.id;
}

void ServerHandle::recv_heartbeat(u32 id) {
    if (id < prev_heartbeat_id) {
        ERR("Received old heartbeat");
    } else if (id > prev_heartbeat_id) {
        ERR("Received heartbeat from the future");
    } else {
        auto now = steady_clock::now();
        TRACE("Heartbeat latency: {} ms", (f32)std::chrono::duration_cast<std::chrono::microseconds>(now - heartbeat_time_start).count() / 1000);
    }
}

int start_server_handle(void *data) {
    ServerHandle *handle = (ServerHandle *)data;
    handle->active = true;
    Package package;
    u8 buf[sizeof(Package)];
    while (handle->active) {
        if (handle->recv(buf, sizeof(buf), &package)) {
            switch (package.header.type) {
            case PackageType::SET_CLIENT_ID:
                handle->client_id = package.SET_CLIENT_ID.client_id;
                handle->wip_package.header.client = handle->client_id;
                if (SDL_LockMutex(GAMESTATE()->entity_system.m_client_id) != 0) {
                    ERR("Unable to lock mutex: {}", SDL_GetError());
                } else {
                    GAMESTATE()->entity_system.client_id = handle->client_id;
                    SDL_UnlockMutex(GAMESTATE()->entity_system.m_client_id);
                }
                on_connect_to_server();
                break;
            case PackageType::HEARTBEAT:
                handle->recv_heartbeat(package.HEARTBEAT.id);
                break;
            default:
                break;
            }
        }
    }
    close(handle->sockfd);
    handle->wip_entities.free();
    return 0;
}

int start_client_handle(void *data) {
    ClientHandle *handle = (ClientHandle *)data;
    handle->active = true;
    Package package;
    u8 buf[sizeof(Package)];
    while (handle->active) {
        if (handle->recv(buf, sizeof(buf), &package)) {
            if (SDL_LockMutex(GAMESTATE()->network.m_prev_package) != 0) {
                ERR("Unable to lock mutex: {}", SDL_GetError());
            } else {
                GAMESTATE()->network.prev_package = package;
                SDL_UnlockMutex(GAMESTATE()->network.m_prev_package);
            }
            switch (package.header.type) {
            case PackageType::HEARTBEAT: {
                Package heartbeatResponse;
                heartbeatResponse.header.type = PackageType::HEARTBEAT;
                heartbeatResponse.header.client = package.header.client;
                heartbeatResponse.HEARTBEAT.id = package.header.id;
                handle->send(&heartbeatResponse);
                break;
            }
            default:
                break;
            }
        }
    }
    close(handle->sockfd);
    handle->wip_entities.free();
    if (SDL_LockMutex(GAMESTATE()->m_event_queue) != 0) {
        ERR("Unable to lock mutex: {}", SDL_GetError());
    } else {
        Event event;
        event.type = EventType::DROP_CLIENT;
        event.DROP_CLIENT = {
            .client_id = handle->client_id,
        };
        GAMESTATE()->event_queue.push(event);
        SDL_UnlockMutex(GAMESTATE()->m_event_queue);
    }
    return 0;
}

void DropClientEvent::callback() {
    GAMESTATE()->entity_system.drop_client(client_id);
}

bool Network::setup_server(int portno) {
    if (server_listening) {
        WARN("Server already running");
        return false;
    }
    m_prev_package = SDL_CreateMutex();
    ASSERT(m_prev_package, "Unable to create mutex");
    INFO("Setting up server on port {}", portno);
    listen_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serv_addr = {};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(listen_sockfd, (sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        ERR("Error binding to socket, errno={}", errno);
        return false;
    }
    listen(listen_sockfd, 5);
    cli_len = sizeof(cli_addr);
    listener_thread = SDL_CreateThread(network_listen_for_clients, "ListenForClients", (void *)this);
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
            client_handles[i].close();
            client_handles[i].wip_entities.free();
        }
    }
    INFO("Server stopped");
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
    if (connect(server_handle.sockfd, (sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        WARN("Unable to connect to server, errno={}", errno);
        return false;
    }
    INFO("Connected");
    server_handle.wip_entities.alloc();
    GAMESTATE()->entity_system.remove_all();
    sntprint(server_handle.thread_name, sizeof(server_handle.thread_name), "ServerHandle");
    server_handle.thread = SDL_CreateThread(start_server_handle, "ServerHandle", (void *)&server_handle);
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
    ClientHandle *handle;
    for (u32 i = 0; i < MAX_CLIENTS; i++) {
        handle = client_handles + i;
        if (handle->active) continue;
        TRACE("Found client handle");
        *handle = {};
        handle->sockfd = newsockfd;
        handle->client_id = next_handle_id;
        next_handle_id += HANDLE_ID_FIRST_BIT;
        handle->wip_package.header.client = handle->client_id;
        handle->wip_entities.alloc();
        sntprint(handle->thread_name, sizeof(handle->thread_name), "ClientHandle {}", handle->client_id);
        handle->thread = SDL_CreateThread(start_client_handle, handle->thread_name, (void *)handle);
        if (!handle->thread) {
            ERR("Unable to create thread");
            return false;
        }
        Package id_package;
        id_package.header.type = PackageType::SET_CLIENT_ID;
        id_package.SET_CLIENT_ID.client_id = handle->client_id;
        handle->send(&id_package);
        GAMESTATE()->entity_system.send_initial_state(handle);
        return true;
    }
    WARN("No available client handles, ignoring");
    return false;
}

void Network::send_state_to_server() {}

void Network::send_state_to_clients() {
    for (u32 i = 0; i < MAX_CLIENTS; i++) {
        ClientHandle *handle = client_handles + i;
        if (handle->active) {
            GAMESTATE()->entity_system.send_state(handle);
        }
    }
}

#ifdef IMGUI_ENABLE
void Network::imgui_draw() {
    if (!GAMESTATE()->imgui.networking_enabled) return;
    ImGui::Begin("Networking");
    {
        ImGui::SetNextItemWidth(150);
        ImGui::InputInt("port", &GAMESTATE()->imgui.network_port);
        if (ImGui::Button("Create server")) {
            setup_server(GAMESTATE()->imgui.network_port);
        }
        if (server_listening) {
            ImGui::SameLine();
            if (ImGui::Button("Stop server")) {
                stop_server();
            }

            for (u32 i = 0; i < MAX_CLIENTS; i++) {
                NetworkHandle *handle = client_handles + i;
                if (!handle->active) continue;
                ImGui::PushID(i);
                ImGui::AlignTextToFramePadding();
                ImGui::Text(handle->thread_name);
                ImGui::SameLine();
                if (ImGui::Button("Send package")) {
                    handle->creating_package_to_send = !handle->creating_package_to_send;
                }
                if (handle->creating_package_to_send) {
                    ImGui::Begin(handle->thread_name);
                    imgui_package_create(&handle->wip_package, &handle->wip_entities);
                    if (ImGui::Button("Send")) {
                        handle->send(&handle->wip_package);
                    }
                    if (ImGui::Button("Disconnect")) {
                        handle->close();
                    }
                    ImGui::End();
                }
                ImGui::PopID();
            }

            ImGui::Text("Latest package");
            if (SDL_LockMutex(m_prev_package) != 0) {
                ERR("Unable to lock mutex: {}", SDL_GetError());
            } else {
                imgui_package_show(&prev_package);
                SDL_UnlockMutex(m_prev_package);
            }
        }

        ImGui::Separator();

        static char ip[64] = "127.0.0.1";
        ImGui::SetNextItemWidth(150);
        ImGui::InputText("server address", ip, LEN(ip));
        ImGui::SetNextItemWidth(150);
        ImGui::PushID(&GAMESTATE()->imgui.network_port);
        ImGui::InputInt("port", &GAMESTATE()->imgui.network_port);
        ImGui::PopID();
        if (ImGui::Button("Connect to server")) {
            connect_to_server(ip, GAMESTATE()->imgui.network_port);
        }
        if (server_handle.active) {
            ImGui::AlignTextToFramePadding();
            ImGui::Text(server_handle.thread_name);
            ImGui::SameLine();
            if (ImGui::Button("Send package")) {
                server_handle.creating_package_to_send = !server_handle.creating_package_to_send;
            }
            if (server_handle.creating_package_to_send) {
                ImGui::Begin(server_handle.thread_name);
                imgui_package_create(&server_handle.wip_package, &server_handle.wip_entities);
                if (ImGui::Button("Send")) {
                    server_handle.send(&server_handle.wip_package);
                }
                ImGui::End();
            }
            if (ImGui::Button("Send heartbeat")) {
                server_handle.send_heartbeat();
            }

            if (ImGui::BeginChild("Package log", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f, 160))) {
                static int package_log_limit = 10;
                ImGui::InputInt("Max packages shown", &package_log_limit);
                int i = 0;
                char buf[32] = {};
                for (Package &package : server_handle.package_log) {
                    ImGui::PushID(i);
                    u32 package_type = (u32)package.header.type;
                    if (package_type < LEN(package_type_list)) {
                        sntprint(buf, 32, "{}", package_type_list[(u32)package.header.type]);
                    } else {
                        sntprint(buf, 32, "(hidden)");
                    }
                    if (ImGui::Selectable(buf, false)) {
                        TRACE("{}", package);
                    }
                    if (++i == package_log_limit) {
                        ImGui::PopID();
                        break;
                    }
                    ImGui::PopID();
                }
            }
            ImGui::EndChild();

            if (ImGui::Button("Disconnect")) {
                disconnect_from_server();
            }
        }
    }
    ImGui::End();
}
#endif

int network_listen_for_clients(void *data) {
    WARN("Starting server");
    Network *system = (Network *)data;
    int newsockfd;
    system->server_listening = true;
    while (system->server_listening) {
        newsockfd = accept(system->listen_sockfd, (sockaddr *)&system->cli_addr, &system->cli_len);
        if (!system->server_listening) {
            break;
        }
        if (newsockfd < 0) {
            ERR("ListenForClients: Error accepting client connection, errno={}", errno);
            continue;
        }
        TRACE("ListenForClients: New client connected");
        if (!system->new_client_handle(newsockfd)) {
            ERR("ListenForClients: Unable to accept client connection, closing");
            //TODO(gu) send reason
            close(newsockfd);
        }
    }
    TRACE("Stopped listening for new clients");
    return 0;
}

TEST_CASE("network start/stop server", {
    GAMESTATE()->logger.levels &= ~(LogLevel::TRACE | LogLevel::INFO);
    ASSERT(GAMESTATE()->network.setup_server(8888), "error setting up server");
    GAMESTATE()->network.stop_server();
    return true;
});
