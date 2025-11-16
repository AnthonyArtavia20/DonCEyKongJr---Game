package GameServer.CoreGenericServer;
//Networking
import java.net.*;
import java.io.*;
//Colecciones thread-safe
//Concurrencia
import java.util.concurrent.*;
//Básicos
import java.util.*;

/*
 * Descripción de la clase...
 * 
 * @author Anthony Artavia
 * @version 1.0
*/

public abstract class GameServer {
    
    //Atributos del servidor
    protected ServerSocket serverSocket;
    protected List<ClientHandler> clients; //CopyOnWriteArrayList
    protected ExecutorService threadPool;
    protected volatile boolean running;
    protected int port;
    protected int maxClients;
    protected ServerConfig config;
    protected ServerStats stats;
    protected ScheduledExecutorService scheduler;

    //Constructors
    public GameServer(int port, int maxClients) {
        // Preserve behavior: treat provided maxClients as total allowed (no spectators)
        this(new ServerConfig(port, Math.max(1, maxClients), 0, 20));
    }

    public GameServer(ServerConfig config) {
        this.config = config == null ? new ServerConfig() : config;
        this.port = this.config.getPort();
        this.maxClients = this.config.getMaxClients();
        this.clients = new CopyOnWriteArrayList<>();
        int poolSize = Math.max(4, this.maxClients + 2); //Max players and spectors at the moment
        this.threadPool = Executors.newFixedThreadPool(poolSize);
        this.stats = new ServerStats();
    }

    //Métodos utilizables del server
    public void start() {
        if (running) return;
        running = true;

        try {
            serverSocket = new ServerSocket(port);
            System.out.println("[SERVER] Iniciado en puerto " + port);
        } catch (IOException a) {
            System.err.println("[SERVER] No se pudo crear el Socket: " + a.getMessage());
            a.printStackTrace();
            running = false;
            return;
        }

        // Scheduler, esta parte permite calcular un delta de tiempo,o diferencia de tiempo para actualizar los procesos cada cierto tiempo, esto permite
        //permitiendo  mantener el juego sincronizado y fluido, de no usarlo las actualizaciones serían irregulares, sería dificil sincronizar cleintes ,el movimiento se´ria menos preciso.
        if (config != null && config.getTickRate() > 0) {
            scheduler = Executors.newSingleThreadScheduledExecutor(); //crea un hilo dedicado
            final double delta = 1.0 / config.getTickRate(); //Aquí se calcula el tiempo entre updates (ej: 1/20 = 0.05s)
            long periodMs = Math.max(1, 1000 / config.getTickRate()); // Convierte a milisegundos (ej: 50ms)
            scheduler.scheduleAtFixedRate(() -> {
                try {
                    update(delta); //Y aquí se llamaría al método update, para esos miulisegundos, ejemplo 50ms
                } catch (Throwable t) {
                    System.err.println("[Error] update() lanzó excepción: " + t.getMessage());
                }
            }, 0, periodMs, TimeUnit.MILLISECONDS);
        }

        // Hilo aceptador de clientes
        threadPool.submit(() -> {
            while (running) {
                try {
                    Socket clientSocket = serverSocket.accept();
                    System.out.println("[SERVER] Cliente Conectado: " + clientSocket.getInetAddress());

                    // Enforcement de límite de clientes
                    if (maxClients > 0 && clients.size() >= maxClients) {
                        System.err.println("[SERVER] Rechazando conexión: límite de clientes alcanzado");
                        try { clientSocket.close(); } catch (IOException ignored) {}
                        continue;
                    }

                    ClientHandler handler = createClient(clientSocket);
                    if (handler != null) {
                        clients.add(handler);
                        stats.onNewConnection();
                        threadPool.submit(handler);
                    } else {
                        try { clientSocket.close(); } catch (IOException ignored) {}
                    }
                } catch (IOException b) {
                    if (running) {
                        System.err.println("[Error] Error aceptando conexión: " + b.getMessage());
                    }
                }
            }
        });
    }

    public void stop() {
        running = false;

        try {
            if (serverSocket != null && !serverSocket.isClosed()) {
                serverSocket.close();
            }
        } catch (IOException c) {
            System.err.println("[Error] Error cerrando el socket: " + c.getMessage());
        }

        // Cerrar y limpiar clientes
        for (ClientHandler ch : clients) {
            try {
                ch.close();
            } catch (Exception ignored) {}
        }
        clients.clear();

        // Detener el pool de hilos
        threadPool.shutdownNow();
        if (scheduler != null) scheduler.shutdownNow();
        System.out.println("[SERVER] Detenido, acción realizada: Apagar server");
    }

    public void broadcast(String msg) {
        for (ClientHandler ch : clients) {
            try {
                ch.send(msg);
                stats.onMessageSent();
            } catch (Exception e) {
                System.err.println("[Error] Al enviar broadcast: " + e.getMessage());
            }
        }
    }

    public void sendTo(ClientHandler client, String msg) {
        if (client == null) return;
        try {
            client.send(msg);
            stats.onMessageSent();
        } catch (Exception e) {
            System.err.println("[Error] Al enviar a cliente: " + e.getMessage());
        }
    }

    public void disconnectClient(ClientHandler client) {
        if (client == null) return;
        clients.remove(client);
        stats.onDisconnect();
        try {
            client.close();
        } catch (Exception ignored) {}
    }

    /**
     * Método final que contabiliza mensajes recibidos y delega al manejador concreto.
     * ClientHandler debe llamar a este método cuando reciba mensajes.
     */
    public final void dispatchMessage(ClientHandler client, String msg) {
        if (stats != null) stats.onMessageReceived();
        handleMessage(client, msg);
    }

    //Métodos abstractos:
    protected abstract ClientHandler createClient(Socket socket); //Patrón Factory, GameServer delega la creación de clientes, DonkeyKongServer.createClient() lo implementa
    protected abstract void update(double delta);
    protected abstract String getGameState();
    protected abstract void handleMessage(ClientHandler client, String msg);
}
