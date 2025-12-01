// Modificado: agregado comandos CLI para eliminar fruta por id (interactive 'df' y inline 'deletef <id>')
package GameServer.DonkeyKong.Server;

import GameServer.CoreGenericServer.*;
import GameServer.DonkeyKong.Game.GameLogic;
import java.io.IOException;
import java.net.Socket;
import java.util.Map;
import java.util.Scanner;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.atomic.AtomicInteger;

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
    private final ConcurrentLinkedQueue<MessageProtocol.Message> gameEvents = new ConcurrentLinkedQueue<>();
    
    // Gestión de frutas con IDs
    private final AtomicInteger nextFruitId = new AtomicInteger(1);
    // map: fruitId -> int[]{vine, height, points}
    private final ConcurrentHashMap<Integer, int[]> fruitMap = new ConcurrentHashMap<>();

    
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
protected void update(double delta, boolean crash) {

    // Procesar eventos pendientes (otros tipos)
    MessageProtocol.Message evt;
    while ((evt = gameEvents.poll()) != null) {

        System.out.println("\n[SERVER UPDATE] Procesando evento: " + evt.command);

        switch (evt.command) {

            // --- ENEMY_HIT ya viene en cola (mantener) ---
            case "ENEMY_HIT": {
                int playerId = evt.getParamAsInt(0, -1);
                int enemyId = evt.getParamAsInt(1, -1);
                int damage  = evt.getParamAsInt(2, 1);


                if (gameLogic != null) {
                    gameLogic.enemyHit(playerId, enemyId, damage);
                } else {
                    System.out.println("  -> gameLogic es NULL, no se puede procesar ENEMY_HIT.");
                }
                break;
            }

            default:
                System.out.println("  -> Evento desconocido en cola: " + evt.command);
                break;
        }
    }

    // Ejecutar update del juego
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

            // HIT: ahora procesamos HIT por id (síncrono) para poder saber quién lo hizo
            // Formato esperado: HIT|<fruitId>|<playerId>

            case "POS": {
                if (!m.hasParams(3)) {
                    sendTo(client, MessageProtocol.encode("ERROR", "INVALID_PARAMS", "Expected: POS|PLAYER_ID|X|Y"));
                    break;
                }
                int pid = m.getParamAsInt(0, -1);
                String xs = m.getParam(1);
                String ys = m.getParam(2);

                // Broadcast a TODOS los clientes (incluye observadores)
                String posMsg = MessageProtocol.encode("PLAYER_POS", String.valueOf(pid), xs, ys);
                broadcast(posMsg);

                // (Opcional) Actualizar estado interno si GameLogic expone API:
                // if (gameLogic != null) gameLogic.updatePlayerPosition(pid, Float.parseFloat(xs), Float.parseFloat(ys));

                // No respondemos al cliente explicitamente (pero se podría)
                break;
            }

            case "HIT": {
                if (!m.hasParams(2)) {
                    sendTo(client, MessageProtocol.encode("ERROR", "INVALID_PARAMS", "Expected: HIT|FRUIT_ID|PLAYER_ID"));
                    break;
                }
                int fruitId = m.getParamAsInt(0, -1);
                int pId = m.getParamAsInt(1, -1);
                if (fruitId < 0) {
                    sendTo(client, MessageProtocol.encode("ERROR", "INVALID_FRUIT_ID"));
                    break;
                }
                // Lookup fruit info
                int[] info = fruitMap.remove(fruitId);
                if (info == null) {
                    // Puede que el cliente esté usando formato antiguo (vine,height)
                    // Para compatibilidad, si param0 es vine en vez de id, intentar manejar
                    System.out.println("[SERVER] HIT recibido pero fruitId no encontrado: " + fruitId);
                    sendTo(client, MessageProtocol.encode("ERROR", "FRUIT_NOT_FOUND", String.valueOf(fruitId)));
                    break;
                }
                int vine = info[0];
                int height = info[1];
                int points = info[2];

                System.out.println("[SERVER] HIT procesado: fruitId=" + fruitId + " vine=" + vine + " height=" + height + " by player " + pId);

                // Delegar a gameLogic para eliminar la fruta (si existe)
                boolean deleted = false;
                if (gameLogic != null) {
                    deleted = gameLogic.deleteFruit(vine, height);
                } else {
                    // Si no hay gameLogic, consideramos que sí se "eliminó" lógicamente
                    deleted = true;
                }

                // Broadcast FRUIT_DELETED y actualizar score del jugador
                if (deleted) {
                    String delMsg = MessageProtocol.encode("FRUIT_DELETED", String.valueOf(fruitId), String.valueOf(pId), String.valueOf(points));
                    broadcast(delMsg);

                    // Incrementar score en el handler (si es jugador)
                    if (dkClient.isPlayer()) {
                        dkClient.addScore(points);
                        // Enviar actualización de puntaje a TODOS (puede ser optimizado para enviar sólo al jugador)
                        String scoreMsg = MessageProtocol.encode("SCORE_UPDATE", String.valueOf(dkClient.getPlayerId()), String.valueOf(dkClient.getScore()));
                        broadcast(scoreMsg);
                    }
                    System.out.println("[SERVER] FRUIT_DELETED broadcast id=" + fruitId + " points=" + points);
                } else {
                    sendTo(client, MessageProtocol.encode("ERROR", "DELETE_FAILED"));
                }
                break;
            }

            case "ENEMY_HIT":
                System.out.println("[SERVER] ENEMY_HIT recibido de cliente: " + dkClient.getPlayerName());
                // Meter el evento correcto a la cola
                gameEvents.add(m);   // ✅ Aquí debe ir m, NO msg
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
            
            // Responder al cliente con su ID, vidas y score inicial
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
                    }
                    // Assign an id and store mapping
                    int fid = nextFruitId.getAndIncrement();
                    fruitMap.put(fid, new int[]{vine, height, points});
                    // Broadcast with id
                    String msg = MessageProtocol.encode("FRUIT_CREATED",
                                                        String.valueOf(fid),
                                                        String.valueOf(vine),
                                                        String.valueOf(height),
                                                        String.valueOf(points));
                    broadcast(msg);
                    sendTo(client, MessageProtocol.encode("OK", "FRUIT_CREATED", String.valueOf(fid)));
                } else {
                    sendTo(client, MessageProtocol.encode("ERROR", "INVALID_PARAMS"));
                }
                break;
            
            case "CREATE_CCA":
                if (m.hasParams(4)) {
                    int vine = m.getParamAsInt(1, -1);
                    int height = m.getParamAsInt(2, -1);
                    int points = m.getParamAsInt(3, 100);
                    if (gameLogic != null) {
                        gameLogic.createFruit(vine, height, points);
                    }
                    // For compatibility, also respond as CCA_CREATED without id
                    int fid = nextFruitId.getAndIncrement();
                    fruitMap.put(fid, new int[]{vine, height, points});
                    String msg = MessageProtocol.encode("CCA_CREATED",
                                                        String.valueOf(vine),
                                                        String.valueOf(height),
                                                        String.valueOf(points));
                    broadcast(msg);
                    // Also broadcast FRUIT_CREATED with id for clients expecting id
                    broadcast(MessageProtocol.encode("FRUIT_CREATED",
                                                     String.valueOf(fid),
                                                     String.valueOf(vine),
                                                     String.valueOf(height),
                                                     String.valueOf(points)));
                    sendTo(client, MessageProtocol.encode("OK", "CCA_CREATED", String.valueOf(fid)));
                } else {
                    sendTo(client, MessageProtocol.encode("ERROR", "INVALID_PARAMS"));
                }
                break;
                
            case "DELETE_FRUIT":
                // Mantener compatibilidad: DELETE_FRUIT|vine|height
                if (m.hasParams(3)) {
                    int vine = m.getParamAsInt(1, -1);
                    int height = m.getParamAsInt(2, -1);
                    boolean deleted = false;
                    if (gameLogic != null) {
                        deleted = gameLogic.deleteFruit(vine, height);
                    } else {
                        deleted = true;
                    }
                    // Buscar id(s) que tengan vine+height y eliminarlos
                    Integer foundId = null;
                    for (Map.Entry<Integer,int[]> e : fruitMap.entrySet()) {
                        int[] info = e.getValue();
                        if (info[0] == vine && info[1] == height) {
                            foundId = e.getKey();
                            fruitMap.remove(foundId);
                            break;
                        }
                    }
                    if (deleted) {
                        if (foundId != null) {
                            broadcast(MessageProtocol.encode("FRUIT_DELETED", String.valueOf(foundId), "0", String.valueOf(0)));
                        }
                        sendTo(client, MessageProtocol.encode("OK", "FRUIT_DELETED"));
                    } else {
                        sendTo(client, MessageProtocol.encode("ERROR", "FRUIT_NOT_FOUND"));
                    }
                } else {
                    sendTo(client, MessageProtocol.encode("ERROR", "INVALID_PARAMS"));
                }
                break;

            case "DELETE_FRUIT_BY_ID":
                // Nuevo: ADMIN|DELETE_FRUIT_BY_ID|<id>
                if (m.hasParams(2)) {
                    int fid = m.getParamAsInt(1, -1);
                    int[] info = fruitMap.remove(fid);
                    if (info != null) {
                        int vine = info[0];
                        int height = info[1];
                        boolean deleted = false;
                        if (gameLogic != null) deleted = gameLogic.deleteFruit(vine, height);
                        else deleted = true;
                        if (deleted) {
                            broadcast(MessageProtocol.encode("FRUIT_DELETED", String.valueOf(fid), "0", String.valueOf(info[2])));
                            sendTo(client, MessageProtocol.encode("OK", "FRUIT_DELETED", String.valueOf(fid)));
                        } else {
                            sendTo(client, MessageProtocol.encode("ERROR", "DELETE_FAILED"));
                        }
                    } else {
                        sendTo(client, MessageProtocol.encode("ERROR", "FRUIT_NOT_FOUND"));
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
        System.out.println("  Factory Pattern: ✓");
        System.out.println("  Nivel inicial: 1");
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
            System.out.println("\nComandos disponibles:");
            System.out.println("  stats  - Mostrar estadísticas del servidor");
            System.out.println("  cf     - Crear Fruta (puntos según nivel)");
            System.out.println("  cca    - Crear Cocodrilo Azul (velocidad según nivel)");
            System.out.println("  ccr    - Crear Cocodrilo Rojo (velocidad según nivel)");
            System.out.println("  df     - Delete Fruit (interactive)  -> ingresa ID");
            System.out.println("  deletef <id> - Delete Fruit by id inline");
            System.out.println("  level  - Cambiar nivel (cambia Factory automáticamente)");
            System.out.println("  info   - Información del estado del juego");
            System.out.println("  quit   - Detener servidor");
            System.out.println();
            
            while (scanner.hasNextLine()) {
                String line = scanner.nextLine().trim();
                String lower = line.toLowerCase();
                
                if (lower.equals("quit") || lower.equals("exit") || lower.equals("stop")) {
                    break;
                    
                } else if (lower.equals("stats")) {
                    System.out.println("\n=== ESTADÍSTICAS DEL SERVIDOR ===");
                    System.out.println(server.stats.toString());
                    System.out.println("Jugadores activos: " + server.playerCount);
                    System.out.println("Espectadores activos: " + server.spectatorCount);
                    System.out.println("==================================\n");
                    
                } else if (lower.equals("info")) {
                    System.out.println("\n=== ESTADO DEL JUEGO ===");
                    if (server.gameLogic != null) {
                        System.out.println("Nivel actual: " + server.gameLogic.getCurrentLevel());
                        System.out.println("Enemigos activos: " + server.gameLogic.getEnemyCount());
                        System.out.println("Frutas activas: " + server.gameLogic.getFruitCount());
                    } else {
                        System.out.println("GameLogic no inicializado");
                    }
                    System.out.println("=========================\n");
                    
                } else if (lower.equals("level")) {
                    System.out.print("Ingrese el nuevo nivel (1-3): ");
                    String inputLevel = scanner.nextLine().trim();
                    
                    try {
                        int newLevel = Integer.parseInt(inputLevel);
                        if (newLevel < 1 || newLevel > 3) {
                            System.out.println("[SERVER] ERROR: Nivel debe estar entre 1 y 3");
                            continue;
                        }
                        
                        if (server.gameLogic != null) {
                            server.gameLogic.changeLevel(newLevel);
                            System.out.println("[SERVER] ✓ Nivel cambiado a " + newLevel);
                            System.out.println("[SERVER] La Factory ahora creará enemigos de nivel " + newLevel);
                        }
                    } catch (NumberFormatException e) {
                        System.out.println("[SERVER] ERROR: Debe ingresar un número válido");
                    }
                    
                } else if (lower.equals("cf")) {
                    System.out.print("Ingrese la liana para la fruta (1-9): ");
                    String inputVine = scanner.nextLine().trim();

                    int vine;
                    try {
                        vine = Integer.parseInt(inputVine);
                        if (vine < 1 || vine > 9) {
                            System.out.println("[SERVER] ERROR: Liana debe estar entre 1 y 9");
                            continue;
                        }
                    } catch (NumberFormatException e) {
                        System.out.println("[SERVER] ERROR: Debe ingresar un número válido");
                        continue;
                    }

                    // Altura default (puedes cambiarla si quieres)
                    int height = 400;
                    int points = 100;
                    if (server.gameLogic != null) {
                        // La factory asigna los puntos según el nivel (llamar para efecto)
                        server.gameLogic.createFruit(vine, height, points);
                    }

                    // Asignar id y broadcast
                    int fid = server.nextFruitId.getAndIncrement();
                    server.fruitMap.put(fid, new int[]{vine, height, points});

                    String msg = MessageProtocol.encode("FRUIT_CREATED",
                                                        String.valueOf(fid),
                                                        String.valueOf(vine),
                                                        String.valueOf(height),
                                                        String.valueOf(points));

                    server.broadcast(msg);
                    
                    System.out.println("[SERVER] ✓ Fruta creada (id=" + fid + ") en liana " + vine + 
                                    " (Nivel " + (server.gameLogic != null ? server.gameLogic.getCurrentLevel() : 0) + ")");
                } else if (lower.equals("cca")) {
                    System.out.print("Ingrese la liana para el cocodrilo azul (1-9): ");
                    String inputVine = scanner.nextLine().trim();

                    int vine;
                    try {
                        vine = Integer.parseInt(inputVine);
                        if (vine < 1 || vine > 9) {
                            System.out.println("[SERVER] ERROR: Liana debe estar entre 1 y 9");
                            continue;
                        }
                    } catch (NumberFormatException e) {
                        System.out.println("[SERVER] ERROR: Debe ingresar un número válido");
                        continue;
                    }

                    if (server.gameLogic != null) {
                        // La factory asigna la velocidad según el nivel
                        server.gameLogic.createBlueCrocodile(vine, 0);
                    }

                    // Enviar a TODOS los clientes (mantener compatibilidad)
                    String msg = MessageProtocol.encode("CCA_CREATED",
                                                        String.valueOf(vine),
                                                        "0",
                                                        "0");

                    server.broadcast(msg);
                    
                    System.out.println("[SERVER] ✓ Cocodrilo AZUL creado en liana " + vine + 
                                    " (Nivel " + (server.gameLogic != null ? server.gameLogic.getCurrentLevel() : 0) + ")");
                } else if (lower.equals("ccr")) {
                    System.out.print("Ingrese la liana para el cocodrilo rojo (1-9): ");
                    String inputVine = scanner.nextLine().trim();

                    int vine;
                    try {
                        vine = Integer.parseInt(inputVine);
                        if (vine < 1 || vine > 9) {
                            System.out.println("[SERVER] ERROR: Liana debe estar entre 1 y 9");
                            continue;
                        }
                    } catch (NumberFormatException e) {
                        System.out.println("[SERVER] ERROR: Debe ingresar un número válido");
                        continue;
                    }

                    if (server.gameLogic != null) {
                        // La factory asigna la velocidad según el nivel
                        server.gameLogic.createRedCrocodile(vine, 0);
                    }

                    String msg = MessageProtocol.encode("CCR_CREATED",
                                                        String.valueOf(vine),
                                                        "0",
                                                        "0");

                    server.broadcast(msg);
                    
                    System.out.println("[SERVER] ✓ Cocodrilo ROJO creado en liana " + vine + 
                                    " (Nivel " + (server.gameLogic != null ? server.gameLogic.getCurrentLevel() : 0) + ")");
                } else if (lower.equals("df")) {
                    // Interactive delete by id
                    System.out.print("Ingrese ID de fruta a eliminar: ");
                    String idStr = scanner.nextLine().trim();
                    try {
                        int fid = Integer.parseInt(idStr);
                        int[] info = server.fruitMap.remove(fid);
                        if (info != null) {
                            int vine = info[0];
                            int height = info[1];
                            int points = info[2];
                            boolean deleted = false;
                            if (server.gameLogic != null) deleted = server.gameLogic.deleteFruit(vine, height);
                            else deleted = true;
                            if (deleted) {
                                server.broadcast(MessageProtocol.encode("FRUIT_DELETED", String.valueOf(fid), "0", String.valueOf(points)));
                                System.out.println("[SERVER] ✓ Fruta id=" + fid + " eliminada y broadcast enviada.");
                            } else {
                                System.out.println("[SERVER] ERROR: No se pudo eliminar fruta en GameLogic.");
                            }
                        } else {
                            System.out.println("[SERVER] ERROR: No existe fruta con id=" + fid);
                        }
                    } catch (NumberFormatException e) {
                        System.out.println("[SERVER] ERROR: ID inválido");
                    }
                } else if (lower.startsWith("deletef ")) {
                    // Inline deletef <id>
                    String[] parts = line.split("\\s+");
                    if (parts.length >= 2) {
                        try {
                            int fid = Integer.parseInt(parts[1]);
                            int[] info = server.fruitMap.remove(fid);
                            if (info != null) {
                                int vine = info[0];
                                int height = info[1];
                                int points = info[2];
                                boolean deleted = false;
                                if (server.gameLogic != null) deleted = server.gameLogic.deleteFruit(vine, height);
                                else deleted = true;
                                if (deleted) {
                                    server.broadcast(MessageProtocol.encode("FRUIT_DELETED", String.valueOf(fid), "0", String.valueOf(points)));
                                    System.out.println("[SERVER] ✓ Fruta id=" + fid + " eliminada y broadcast enviada.");
                                } else {
                                    System.out.println("[SERVER] ERROR: No se pudo eliminar fruta en GameLogic.");
                                }
                            } else {
                                System.out.println("[SERVER] ERROR: No existe fruta con id=" + fid);
                            }
                        } catch (NumberFormatException e) {
                            System.out.println("[SERVER] ERROR: ID inválido");
                        }
                    } else {
                        System.out.println("[SERVER] Uso: deletef <id>");
                    }
                } else {
                    System.out.println("[SERVER] Comando desconocido: " + line);
                    System.out.println("Escribe 'stats', 'info', 'cf', 'cca', 'ccr', 'df', 'deletef <id>', 'level' o 'quit'");
                }
            }
        }
        
        System.out.println("\n[SERVER] Deteniendo servidor...");
        server.stop();
        System.exit(0);
    }
}