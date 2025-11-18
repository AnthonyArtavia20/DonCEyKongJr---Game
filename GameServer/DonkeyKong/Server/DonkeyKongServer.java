package GameServer.DonkeyKong.Server;

import GameServer.CoreGenericServer.*;
import GameServer.DonkeyKong.Game.GameLogic;
import java.io.IOException;
import java.net.Socket;
import java.util.Scanner;

/**
 * Servidor específico para DonCEy Kong Jr
 * Extiende el GameServer genérico
 * 
 * @author Anthony Artavia
 */
public class DonkeyKongServer extends GameServer {
    
    private GameLogic gameLogic;
    private int playerCount = 0;
    private int spectatorCount = 0;
    
    public DonkeyKongServer(ServerConfig config) {
        super(config);
        this.gameLogic = new GameLogic();
    }
    
    public DonkeyKongServer(int port) {
        this(new ServerConfig(port, 2, 2, 60)); // 2 jugadores, 2 espectadores/jugador, 60 FPS
    }
    
    @Override
    protected ClientHandler createClient(Socket socket) { //aquí se usa el patrón Factory DonkeyKongServer lo implementa desde GameServer
        try {
            return new DKClientHandler(socket, this);
        } catch (IOException e) {
            System.err.println("[DK] Error creando ClientHandler: " + e.getMessage());
            return null;
        }
    }
    
    @Override
    protected void update(double delta) {
        // Actualizar lógica del juego (Ariel implementa esto)
        if (gameLogic != null) {
            gameLogic.update(delta);
        }
    }
    
    @Override
    protected String getGameState() {
        // Serializar estado del juego para enviar a clientes
        if (gameLogic == null) {
            return MessageProtocol.encode("GAMESTATE", "0", "PLAYERS||CROCS||FRUITS|");
        }
        return gameLogic.serialize();
    }
    
    @Override
    protected void handleMessage(ClientHandler client, String msg) {
        if (!(client instanceof DKClientHandler)) {
            return;
        }
        
        DKClientHandler dkClient = (DKClientHandler) client;
        MessageProtocol.Message m = MessageProtocol.decode(msg);
        
        switch (m.command) {
            case "CONNECT":
                handleConnect(dkClient, m);
                break;
                
            case "MOVE":
                handleMove(dkClient, m);
                break;
                
            case "ADMIN":
                handleAdmin(dkClient, m);
                break;
                
            case "DISCONNECT":
                disconnectClient(dkClient);
                break;
                
            case "PING":
                sendTo(client, MessageProtocol.encode("PONG"));
                break;
                
            default:
                sendTo(client, MessageProtocol.encode("ERROR", "INVALID_COMMAND", m.command));
                break;
        }
    }
    
    /**
     * Maneja la conexión de un cliente (PLAYER o SPECTATOR)
     * Formato: CONNECT|PLAYER|<nombre>  o  CONNECT|SPECTATOR|<nombre>
     */
    private void handleConnect(DKClientHandler client, MessageProtocol.Message m) {
        if (!m.hasParams(2)) {
            sendTo(client, MessageProtocol.encode("ERROR", "INVALID_PARAMS", "Expected: CONNECT|TYPE|NAME"));
            return;
        }
        
        String type = m.getParam(0);
        String name = m.getParam(1);
        
        if (type.equals("PLAYER")) {
            // Verificar límite de jugadores
            if (playerCount >= config.getMaxPlayers()) {
                sendTo(client, MessageProtocol.encode("ERROR", "MAX_PLAYERS_REACHED"));
                disconnectClient(client);
                return;
            }
            
            // Asignar ID de jugador
            playerCount++;
            client.setPlayerId(playerCount);
            client.setPlayerName(name);
            client.setClientType(DKClientHandler.ClientType.PLAYER);
            
            // Notificar al GameLogic (Ariel implementa esto)
            if (gameLogic != null) {
                gameLogic.addPlayer(playerCount, name);
            }
            
            // Responder al cliente
            sendTo(client, MessageProtocol.encode("OK", "PLAYER_ID", 
                String.valueOf(playerCount), "LIVES", "3", "SCORE", "0"));
            
            System.out.println("[DK] Jugador conectado: " + name + " (ID: " + playerCount + ")");
            
        } else if (type.equals("SPECTATOR")) {
            // Verificar límite de espectadores
            int maxSpectators = config.getMaxPlayers() * config.getMaxSpectatorsPerPlayer();
            if (spectatorCount >= maxSpectators) {
                sendTo(client, MessageProtocol.encode("ERROR", "MAX_SPECTATORS_REACHED"));
                disconnectClient(client);
                return;
            }
            
            spectatorCount++;
            client.setPlayerName(name);
            client.setClientType(DKClientHandler.ClientType.SPECTATOR);
            
            // Responder al cliente
            sendTo(client, MessageProtocol.encode("OK", "SPECTATOR_ID", String.valueOf(spectatorCount)));
            
            System.out.println("[DK] Espectador conectado: " + name);
            
        } else if (type.equals("ADMIN")) {
            client.setPlayerName(name);
            client.setClientType(DKClientHandler.ClientType.ADMIN);
            sendTo(client, MessageProtocol.encode("OK", "ADMIN_CONNECTED"));
            System.out.println("[DK] Admin conectado: " + name);
            
        } else {
            sendTo(client, MessageProtocol.encode("ERROR", "INVALID_CLIENT_TYPE", type));
        }
    }
    
    /**
     * Maneja movimiento de jugador
     * Formato: MOVE|<player_id>|<direction>
     */
    private void handleMove(DKClientHandler client, MessageProtocol.Message m) {
        if (!client.isPlayer()) {
            sendTo(client, MessageProtocol.encode("ERROR", "NOT_A_PLAYER"));
            return;
        }
        
        if (!m.hasParams(2)) {
            sendTo(client, MessageProtocol.encode("ERROR", "INVALID_PARAMS", "Expected: MOVE|PLAYER_ID|DIRECTION"));
            return;
        }
        
        int playerId = m.getParamAsInt(0, -1);
        String direction = m.getParam(1);
        
        // Verificar que el jugador solo mueve su propio personaje
        if (client.getPlayerId() != playerId) {
            sendTo(client, MessageProtocol.encode("ERROR", "INVALID_PLAYER_ID"));
            return;
        }
        
        // Delegar al GameLogic (Ariel implementa esto)
        if (gameLogic != null) {
            gameLogic.movePlayer(playerId, direction);
        }
    }
    
    /**
     * Maneja comandos de administrador
     * Formato: ADMIN|<subcommand>|<params...>
     */
    private void handleAdmin(DKClientHandler client, MessageProtocol.Message m) {
        // En producción, verificarías autenticación
        // if (!client.isAdmin()) { ... }
        
        if (!m.hasParams(1)) {
            sendTo(client, MessageProtocol.encode("ERROR", "INVALID_ADMIN_COMMAND"));
            return;
        }
        
        String subCommand = m.getParam(0);
        
        switch (subCommand) {
            case "CREATE_CROC_RED":
                if (m.hasParams(3)) {
                    int vine = m.getParamAsInt(1, -1);
                    float speed = m.getParamAsFloat(2, 1.0f);
                    if (gameLogic != null) {
                        gameLogic.createRedCrocodile(vine, speed);
                        sendTo(client, MessageProtocol.encode("OK", "CROC_CREATED"));
                    }
                } else {
                    sendTo(client, MessageProtocol.encode("ERROR", "INVALID_PARAMS"));
                }
                break;
                
            case "CREATE_CROC_BLUE":
                if (m.hasParams(3)) {
                    int vine = m.getParamAsInt(1, -1);
                    float speed = m.getParamAsFloat(2, 1.0f);
                    if (gameLogic != null) {
                        gameLogic.createBlueCrocodile(vine, speed);
                        sendTo(client, MessageProtocol.encode("OK", "CROC_CREATED"));
                    }
                } else {
                    sendTo(client, MessageProtocol.encode("ERROR", "INVALID_PARAMS"));
                }
                break;
                
            case "CREATE_FRUIT":
                if (m.hasParams(4)) {
                    int vine = m.getParamAsInt(1, -1);
                    int height = m.getParamAsInt(2, -1);
                    int points = m.getParamAsInt(3, 100);
                    if (gameLogic != null) {
                        gameLogic.createFruit(vine, height, points);
                        sendTo(client, MessageProtocol.encode("OK", "FRUIT_CREATED"));
                    }
                } else {
                    sendTo(client, MessageProtocol.encode("ERROR", "INVALID_PARAMS"));
                }
                break;
                
            case "DELETE_FRUIT":
                if (m.hasParams(3)) {
                    int vine = m.getParamAsInt(1, -1);
                    int height = m.getParamAsInt(2, -1);
                    if (gameLogic != null) {
                        boolean deleted = gameLogic.deleteFruit(vine, height);
                        if (deleted) {
                            sendTo(client, MessageProtocol.encode("OK", "FRUIT_DELETED"));
                        } else {
                            sendTo(client, MessageProtocol.encode("ERROR", "FRUIT_NOT_FOUND"));
                        }
                    }
                } else {
                    sendTo(client, MessageProtocol.encode("ERROR", "INVALID_PARAMS"));
                }
                break;
                
            default:
                sendTo(client, MessageProtocol.encode("ERROR", "UNKNOWN_ADMIN_COMMAND", subCommand));
                break;
        }
    }
    
    @Override
    public void disconnectClient(ClientHandler client) {
        if (client instanceof DKClientHandler) {
            DKClientHandler dkClient = (DKClientHandler) client;
            
            if (dkClient.isPlayer()) {
                playerCount = Math.max(0, playerCount - 1);
                if (gameLogic != null && dkClient.getPlayerId() != null) {
                    gameLogic.removePlayer(dkClient.getPlayerId());
                }
                System.out.println("[DK] Jugador desconectado: " + dkClient.getPlayerName());
            } else if (dkClient.isSpectator()) {
                spectatorCount = Math.max(0, spectatorCount - 1);
                System.out.println("[DK] Espectador desconectado: " + dkClient.getPlayerName());
            }
        }
        
        super.disconnectClient(client);
    }
    
    // Main para ejecutar el servidor
    public static void main(String[] args) {
        int port = 5000;
        if (args.length >= 1) {
            try {
                port = Integer.parseInt(args[0]);
            } catch (NumberFormatException e) {
                System.err.println("Puerto inválido, usando 5000");
            }
        }
        
        System.out.println("=================================");
        System.out.println("  DonCEy Kong Jr Server");
        System.out.println("  Puerto: " + port);
        System.out.println("  Max Jugadores: 2");
        System.out.println("  Max Espectadores: 4");
        System.out.println("=================================");
        
        DonkeyKongServer server = new DonkeyKongServer(port);
        
        // Shutdown hook
        Runtime.getRuntime().addShutdownHook(new Thread(() -> {
            System.out.println("\n[SERVER] Cerrando servidor...");
            server.stop();
        }));
        
        server.start();
        
        // CLI simple
        try (Scanner scanner = new Scanner(System.in)) {
            System.out.println("\nComandos:");
            System.out.println("  stats  - Mostrar estadísticas");
            System.out.println("  quit   - Detener servidor");
            System.out.println();
            
            while (scanner.hasNextLine()) {
                String line = scanner.nextLine().trim().toLowerCase();
                
                if (line.equals("quit") || line.equals("exit") || line.equals("stop")) {
                    break;
                } else if (line.equals("stats")) {
                    System.out.println(server.stats.toString());
                    System.out.println("Jugadores activos: " + server.playerCount);
                    System.out.println("Espectadores activos: " + server.spectatorCount);
                }
            }
        }
        
        System.out.println("Deteniendo servidor...");
        server.stop();
        System.exit(0);
    }
}
