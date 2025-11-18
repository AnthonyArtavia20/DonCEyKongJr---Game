#include "socket_client.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string.h>

#pragma comment(lib, "ws2_32.lib")

static SOCKET servidor_socket = INVALID_SOCKET;
static int conectado = 0;

// Inicializar Winsock y conectar al servidor
int conectar_servidor(const char* host, int port) {
    WSADATA wsa;
    struct sockaddr_in server;
    
    // Inicializar Winsock
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        printf("[Socket] Error inicializando Winsock: %d\n", WSAGetLastError());
        return -1;
    }
    
    // Crear socket
    servidor_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (servidor_socket == INVALID_SOCKET) {
        printf("[Socket] Error creando socket: %d\n", WSAGetLastError());
        WSACleanup();
        return -1;
    }
    
    // Configurar servidor
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    inet_pton(AF_INET, host, &server.sin_addr);
    
    // Conectar
    if (connect(servidor_socket, (struct sockaddr*)&server, sizeof(server)) < 0) {
        printf("[Socket] Error conectando al servidor: %d\n", WSAGetLastError());
        closesocket(servidor_socket);
        WSACleanup();
        return -1;
    }
    
    conectado = 1;
    printf("[Socket] Conectado a %s:%d\n", host, port);
    return 0;
}

// Enviar mensaje al servidor
int enviar_mensaje(const char* mensaje) {
    if (!conectado) return -1;
    
    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "%s\n", mensaje);
    
    int resultado = send(servidor_socket, buffer, strlen(buffer), 0);
    if (resultado == SOCKET_ERROR) {
        printf("[Socket] Error enviando: %d\n", WSAGetLastError());
        return -1;
    }
    
    return 0;
}

// Recibir mensaje del servidor (no bloqueante)
int recibir_mensaje(char* buffer, int max_len) {
    if (!conectado) return -1;
    
    // Hacer socket no bloqueante
    u_long mode = 1;
    ioctlsocket(servidor_socket, FIONBIO, &mode);
    
    int bytes = recv(servidor_socket, buffer, max_len - 1, 0);
    
    if (bytes > 0) {
        buffer[bytes] = '\0';
        return bytes;
    }
    
    return 0; // No hay datos
}

// Cerrar conexión
void desconectar_servidor() {
    if (conectado) {
        closesocket(servidor_socket);
        WSACleanup();
        conectado = 0;
        printf("[Socket] Desconectado\n");
    }
}

// Verificar si está conectado
int esta_conectado() {
    return conectado;
}