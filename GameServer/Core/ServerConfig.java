package GameServer.Core;

/**
 * Configuración simple del servidor con valores por defecto.
 * Puedes extender esta clase para cargar desde fichero/env.
 */
public class ServerConfig {
    private int port = 12345;
    private int maxPlayers = 2;
    private int maxSpectatorsPerPlayer = 2; // por jugador
    private int tickRate = 20; // updates por segundo

    public ServerConfig() {}

    public ServerConfig(int port, int maxPlayers, int maxSpectatorsPerPlayer, int tickRate) {
        this.port = port;
        this.maxPlayers = maxPlayers;
        this.maxSpectatorsPerPlayer = maxSpectatorsPerPlayer;
        this.tickRate = tickRate;
    }

    public int getPort() {
        return port;
    }

    public void setPort(int port) {
        this.port = port;
    }

    public int getMaxPlayers() {
        return maxPlayers;
    }

    public void setMaxPlayers(int maxPlayers) {
        this.maxPlayers = maxPlayers;
    }

    public int getMaxSpectatorsPerPlayer() {
        return maxSpectatorsPerPlayer;
    }

    public void setMaxSpectatorsPerPlayer(int maxSpectatorsPerPlayer) {
        this.maxSpectatorsPerPlayer = maxSpectatorsPerPlayer;
    }

    public int getTickRate() {
        return tickRate;
    }

    public void setTickRate(int tickRate) {
        this.tickRate = tickRate;
    }

    /**
     * Calcula el número máximo total de conexiones simultáneas (players + spectators).
     */
    public int getMaxClients() {
        return maxPlayers * (1 + Math.max(0, maxSpectatorsPerPlayer));
    }

    @Override
    public String toString() {
        return "ServerConfig{port=" + port + ", maxPlayers=" + maxPlayers + ", maxSpectatorsPerPlayer=" + maxSpectatorsPerPlayer + ", tickRate=" + tickRate + '}';
    }
}

