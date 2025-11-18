#ifndef SOCKET_CLIENT_H
#define SOCKET_CLIENT_H

/**
 * Conecta al servidor de Donkey Kong
 * @param host IP del servidor (ej: "localhost")
 * @param port Puerto del servidor (ej: 5000)
 * @return 0 si éxito, -1 si error
 */
int conectar_servidor(const char* host, int port);

/**
 * Envía un mensaje al servidor
 * @param mensaje Comando a enviar (ej: "MOVE|1|RIGHT")
 * @return 0 si éxito, -1 si error
 */
int enviar_mensaje(const char* mensaje);

/**
 * Recibe un mensaje del servidor (no bloqueante)
 * @param buffer Buffer donde guardar el mensaje
 * @param max_len Tamaño máximo del buffer
 * @return Número de bytes recibidos, 0 si no hay datos
 */
int recibir_mensaje(char* buffer, int max_len);

/**
 * Cierra la conexión con el servidor
 */
void desconectar_servidor();

/**
 * Verifica si está conectado
 * @return 1 si conectado, 0 si no
 */
int esta_conectado();

#endif