// Config

#define DEFAULT_PORT 55555
#define DEFAULT_MAX_CLIENTS 32




// Implementation

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#ifdef _WIN32
        #include <Windows.h>
#else
        #include <unistd.h>
#endif

#define ENET_IMPLEMENTATION
#include "libs/enet.h"


void SleepMS(int ms) {
        #ifdef _WIN32
        Sleep(ms);
        #else
        usleep(ms * 1000);
        #endif
}


bool keep_running = true;
void SignalHandler(int _) {
        keep_running = false;
}
int port = DEFAULT_PORT;
int max_clients = DEFAULT_MAX_CLIENTS;

int main(int argc, char* argv[]) {
        #ifdef _WIN32
        timeBeginPeriod(1);
        #endif

        signal(SIGINT, SignalHandler);


        if (argc >= 2) {
                port = atoi(argv[1]);
                if (port == 0) {
                        printf("ERROR: Provided port '%s' is invalid\n", argv[1]);
                        puts("USAGE: [PORT] [MAX_CLIENT_COUNT]");
                        return 1;
                }
        }
        if (argc >= 3) {
                max_clients = atoi(argv[2]);
                if (max_clients == 0) {
                        printf("ERROR: Provided max client count '%s' is invalid\n", argv[2]);
                        puts("USAGE: [PORT] [MAX_CLIENT_COUNT]");
                        return 1;
                }
        }

        printf("Port: %d\nMax client count: %d\n", port, max_clients);

        if (enet_initialize() != 0) {
                puts("ERROR: Failed to initialize ENet");
                return 1;
        }

        ENetAddress address = {0};
        address.host = ENET_HOST_ANY;
        address.port = port;
        ENetHost* server = enet_host_create(&address, max_clients, 1, 0, 0);
        if (!server) {
                puts("ERROR: Failed to create ENet server host");
                return 1;
        }

        puts("Server started");

        ENetEvent event;
        while (keep_running) {
                while (enet_host_service(server, &event, 0) > 0) {
                        switch (event.type) {
                                case ENET_EVENT_TYPE_CONNECT:
                                {
                                        char ip[64];
                                        enet_address_get_host_ip(
                                                &event.peer->address,
                                                ip,
                                                sizeof(ip)
                                        );
                                        printf("Client %s connected\n", ip);
                                }
                                break;

                                case ENET_EVENT_TYPE_RECEIVE:
                                {
                                        enet_host_broadcast(server, 0, event.packet);
                                }
                                break;

                                case ENET_EVENT_TYPE_DISCONNECT:
                                case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
                                {
                                        char ip[64];
                                        enet_address_get_host_ip(
                                                &event.peer->address,
                                                ip,
                                                sizeof(ip)
                                        );
                                        printf("Client %s disconnected\n", ip);
                                }
                                break;

                                case ENET_EVENT_TYPE_NONE:
                                break;
                        }
                }

                SleepMS(1);
        }

        puts("Stopping server...");

        enet_host_destroy(server);
        enet_deinitialize();

        #ifdef _WIN32
        timeEndPeriod(1);
        #endif

        return 0;
}