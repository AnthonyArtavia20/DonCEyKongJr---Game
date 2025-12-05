package GameServer.DonkeyKong.Server;

import GameServer.CoreGenericServer.*;
import GameServer.DonkeyKong.Game.GameLogic;
import java.io.IOException;
import java.net.Socket;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Scanner;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.atomic.AtomicInteger;

public class DonkeyKongServer extends GameServer {
    
    private final ConcurrentHashMap<Integer, GameRoom> gameRooms = new ConcurrentHashMap<>();
    private final AtomicInteger nextRoomId = new AtomicInteger(1);
    private int spectatorCount = 0;
    private int playerCount = 0; // Contador de jugadores activos

    // Clase interna para representar una sala de juego
    private static class GameRoom {
        final int roomId;
        final GameLogic gameLogic;
        final ConcurrentHashMap<Integer, int[]> fruitMap;
        final ConcurrentLinkedQueue<MessageProtocol.Message> gameEvents;
        final AtomicInteger roomFruitId;
        String playerName;
        int spectatorCount;
        
        GameRoom(int roomId, int playerId, String playerName) {
            this.roomId = roomId;
            this.playerName = playerName;
            this.gameLogic = new GameLogic();
            this.fruitMap = new ConcurrentHashMap<>();
            this.gameEvents = new ConcurrentLinkedQueue<>();
            this.roomFruitId = new AtomicInteger(1);
            this.spectatorCount = 0;
            
            // Inicializar GameLogic con nivel 1
            if (this.gameLogic != null) {
                this.gameLogic.changeLevel(1);
            }
        }
        
        // Crear fruta en esta sala específica
        public int createFruit(int vine, int height, int points) {
            int fid = roomFruitId.getAndIncrement();
            fruitMap.put(fid, new int[]{vine, height, points});
            
            if (gameLogic != null) {
                gameLogic.createFruit(vine, height, points);
                // System.out.println("[DEBUG] Fruta creada en GameLogic: id=" + fid + ", vine=" + vine + ", height=" + height);
            }
            
            return fid;
        }
        
        // Eliminar fruta de esta sala
        public int[] deleteFruit(int fruitId) {
    int[] info = fruitMap.remove(fruitId);
    
    if (info != null && gameLogic != null) {
        int vine = info[0];
        int height = info[1];
        boolean result = gameLogic.deleteFruit(vine, height);
        
        System.out.println("[DEBUG GameRoom] deleteFruit: id=" + fruitId + 
            ", vine=" + vine + ", height=" + height + 
            ", result=" + result + ", gameLogic=" + (gameLogic != null));
        
        if (result) {
            return info; // Retornar info si se eliminó correctamente
        } else {
            // Si falló en GameLogic, restaurar en el map
            fruitMap.put(fruitId, info);
            return null;
        }
    } else {
        System.out.println("[DEBUG GameRoom] deleteFruit: id=" + fruitId + 
            " not found in fruitMap or gameLogic is null");
        return null;
    }
}
        
        // Crear enemigo en esta sala
        public void createEnemy(String type, int vine, float speed) {
            if (gameLogic != null) {
                if (type.equals("RED")) {
                    gameLogic.createRedCrocodile(vine, speed);
                    // System.out.println("[DEBUG] Cocodrilo ROJO creado en GameLogic: vine=" + vine);
                } else if (type.equals("BLUE")) {
                    gameLogic.createBlueCrocodile(vine, speed);
                    // System.out.println("[DEBUG] Cocodrilo AZUL creado en GameLogic: vine=" + vine);
                }
            }
        }
        
        public void addSpectator() {
            spectatorCount++;
        }
        
        public void removeSpectator() {
            spectatorCount = Math.max(0, spectatorCount - 1);
        }
        
        // Getters para estadísticas
        public int getCurrentLevel() {
            return gameLogic != null ? gameLogic.getCurrentLevel() : 1;
        }
        
        public int getEnemyCount() {
            return gameLogic != null ? gameLogic.getEnemyCount() : 0;
        }
        
        public int getFruitCount() {
            return fruitMap.size();
        }
        
        // Obtener eventos pendientes de GameLogic
        public List<String> getPendingGameLogicEvents() {
            List<String> events = new ArrayList<>();
            if (gameLogic != null) {
                // Intentar obtener eventos de GameLogic si tiene el método
                try {
                    // Esta es una implementación genérica - ajusta según tu GameLogic real
                    // Si GameLogic tiene eventos pendientes, los agregamos aquí
                } catch (Exception e) {
                    // Si GameLogic no tiene este método, no hacemos nada
                }
            }
            return events;
        }
        
        // Limpiar eventos de GameLogic
        public void clearGameLogicEvents() {
            if (gameLogic != null) {
                try {
                    // Limpiar eventos si el método existe
                } catch (Exception e) {
                    // Si no existe, no hacemos nada
                }
            }
        }
    }

    public DonkeyKongServer(ServerConfig config) {
        super(config);
    }
    
    public DonkeyKongServer(int port) {
        this(new ServerConfig(port, 2, 2, 60));
    }
    
    @Override
    protected ClientHandler createClient(Socket socket) {
        try {
            return new DKClientHandler(socket, this);
        } catch (IOException e) {
            System.err.println("[DK] Error creando ClientHandler: " + e.getMessage());
            return null;
        }
    }
    
    @Override
    protected void update(double delta, boolean crash) {
        // Actualizar cada sala de juego independientemente
        for (GameRoom room : gameRooms.values()) {
            // Procesar eventos de esta sala
            MessageProtocol.Message evt;
            while ((evt = room.gameEvents.poll()) != null) {
                switch (evt.command) {
                    case "ENEMY_HIT": {
                        int playerId = evt.getParamAsInt(0, -1);
                        int enemyId = evt.getParamAsInt(1, -1);
                        int damage = evt.getParamAsInt(2, 1);
                        
                        if (room.gameLogic != null) {
                            room.gameLogic.enemyHit(playerId, enemyId, damage);
                        }
                        break;
                    }
                }
            }
            
            // Actualizar lógica del juego de esta sala
            if (room.gameLogic != null) {
                room.gameLogic.update(delta);
                
                // Procesar eventos pendientes de GameLogic
                List<String> gameEvents = room.getPendingGameLogicEvents();
                for (String event : gameEvents) {
                    System.out.println("[DEBUG] Evento de GameLogic: " + event);
                    
                    // Traducir eventos de GameLogic a nuestro protocolo
                    if (event.startsWith("FRUIT_CREATED:")) {
                        // Formato: FRUIT_CREATED:id:vine:height:points
                        String[] parts = event.split(":");
                        if (parts.length >= 5) {
                            String msg = MessageProtocol.encode("FRUIT_CREATED",
                                parts[1], parts[2], parts[3], parts[4],
                                String.valueOf(room.roomId));
                            broadcastToRoom(room.roomId, msg);
                            System.out.println("[DEBUG] Traducido a: " + msg);
                        }
                    }
                    else if (event.startsWith("ENEMY_SPAWNED:")) {
                        // Formato: ENEMY_SPAWNED:id:type:vine
                        String[] parts = event.split(":");
                        if (parts.length >= 4) {
                            String type = parts[2];
                            if (type.equals("RED")) {
                                String msg = MessageProtocol.encode("CCR_CREATED",
                                    parts[3], "0", "0", String.valueOf(room.roomId));
                                broadcastToRoom(room.roomId, msg);
                                System.out.println("[DEBUG] Traducido a: " + msg);
                            } else if (type.equals("BLUE")) {
                                String msg = MessageProtocol.encode("CCA_CREATED",
                                    parts[3], "0", "0", String.valueOf(room.roomId));
                                broadcastToRoom(room.roomId, msg);
                                System.out.println("[DEBUG] Traducido a: " + msg);
                            }
                        }
                    }
                }
                
                // Limpiar eventos procesados
                room.clearGameLogicEvents();
                
                // NO enviar GAMESTATE completo en cada frame (causa lag masivo)
                // Solo enviar cuando hay cambios (POS, FRUITS, ENEMIES, etc)
            }
        }
    }
    
    /**
     * Broadcast solo a clientes de una sala específica
     */
    public void broadcastToRoom(int roomId, String message) {
    if (message == null || message.isEmpty()) return;
    
    // Solo mostrar debug para comandos importantes (NO para POS para reducir spam de logs)
    boolean showDebug = message.startsWith("FRUIT_") || 
                       message.startsWith("CCA_") || 
                       message.startsWith("CCR_") ||
                       message.startsWith("SCORE_") ||
                       message.startsWith("OK|") ||
                       message.startsWith("ERROR");
    
    if (showDebug) {
        System.out.println("[NET] Sala " + roomId + " ← " + 
            (message.length() > 60 ? message.substring(0, 60) + "..." : message));
    }
    
    for (ClientHandler client : clients) {
        if (client instanceof DKClientHandler) {
            DKClientHandler dkClient = (DKClientHandler) client;
            Integer clientRoom = dkClient.getGameRoomId();
            
            if (clientRoom != null && clientRoom == roomId) {
                sendTo(client, message);
            }
        }
    }
}

    /**
     * Obtener sala por ID
     */
    public GameRoom getRoomById(int roomId) {
        return gameRooms.get(roomId);
    }
    
    /**
     * Broadcast a todos los clientes (para compatibilidad CLI)
     * Cambiado a public para coincidir con la declaración en la clase padre
     */
    @Override
    public void broadcast(String message) {
        if (message == null || message.isEmpty()) return;
        
        for (ClientHandler client : clients) {
            sendTo(client, message);
        }
    }
    
    // Método auxiliar para contar clientes en sala
    private int countClientsInRoom(int roomId) {
        int count = 0;
        for (ClientHandler client : clients) {
            if (client instanceof DKClientHandler) {
                DKClientHandler dkClient = (DKClientHandler) client;
                if (roomId == dkClient.getGameRoomId()) {
                    count++;
                }
            }
        }
        return count;
    }
    
    @Override
    protected String getGameState() {
        // Serializar estado del juego (para compatibilidad)
        StringBuilder sb = new StringBuilder();
        sb.append("GAMESTATE|");
        sb.append(gameRooms.size()).append("|");
        
        for (GameRoom room : gameRooms.values()) {
            sb.append("ROOM:").append(room.roomId).append(":");
            sb.append("PLAYER:").append(room.playerName).append(":");
            sb.append("LEVEL:").append(room.gameLogic != null ? room.gameLogic.getCurrentLevel() : 1).append(":");
            sb.append("FRUITS:").append(room.fruitMap.size()).append("|");
        }
        
        return sb.toString();
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
                
            case "POS": {
                if (!m.hasParams(3)) {
                    sendTo(client, MessageProtocol.encode("ERROR", "INVALID_PARAMS"));
                    break;
                }
                
                int pid = m.getParamAsInt(0, -1);
                String xs = m.getParam(1);
                String ys = m.getParam(2);
                
                Integer roomId = dkClient.getGameRoomId();
                if (roomId != null) {
                    String posMsg = MessageProtocol.encode("PLAYER_POS", 
                        String.valueOf(roomId), String.valueOf(pid), xs, ys);
                    broadcastToRoom(roomId, posMsg);
                }
                break;
            }
                
            case "HIT": {
    if (!m.hasParams(2)) {
        sendTo(client, MessageProtocol.encode("ERROR", "INVALID_PARAMS"));
        break;
    }
    
    int fruitId = m.getParamAsInt(0, -1);
    int pId = m.getParamAsInt(1, -1);
    Integer roomId = dkClient.getGameRoomId();
    
    if (roomId == null || fruitId < 0) {
        sendTo(client, MessageProtocol.encode("ERROR", "INVALID_STATE"));
        break;
    }
    
    GameRoom room = getRoomById(roomId);
    if (room == null) {
        sendTo(client, MessageProtocol.encode("ERROR", "ROOM_NOT_FOUND"));
        break;
    }
    
    // Usar el método de la sala en lugar de llamar directamente a gameLogic
   int[] info = room.deleteFruit(fruitId);
    if (info != null) {
    int points = info[2];
        
        String delMsg = MessageProtocol.encode("FRUIT_DELETED", 
            String.valueOf(fruitId), String.valueOf(pId), String.valueOf(points));
        broadcastToRoom(roomId, delMsg);
        
        if (dkClient.isPlayer()) {
            dkClient.addScore(points);
            String scoreMsg = MessageProtocol.encode("SCORE_UPDATE", 
                String.valueOf(pId), String.valueOf(dkClient.getScore()));
            broadcastToRoom(roomId, scoreMsg);
        }
    } else {
        sendTo(client, MessageProtocol.encode("ERROR", "DELETE_FAILED", 
            "No se pudo eliminar la fruta del GameLogic"));
    }
    break;
}
                
            case "ENEMY_HIT": {
                Integer roomId = dkClient.getGameRoomId();
                if (roomId != null) {
                    GameRoom room = getRoomById(roomId);
                    if (room != null) {
                        room.gameEvents.add(m);
                    }
                }
                break;
            }
                
            case "ACTION": {
                if (m.hasParams(3)) {
                    String action = m.getParam(1);
                    String param = m.getParam(2);
                    
                    if (action.equals("LEVEL_UP")) {
                        Integer roomId = dkClient.getGameRoomId();
                        if (roomId != null) {
                            GameRoom room = getRoomById(roomId);
                            if (room != null && room.gameLogic != null) {
                                int newLevel = Integer.parseInt(param);
                                room.gameLogic.changeLevel(newLevel);
                                System.out.println("[DK] Sala " + roomId + " subió a nivel " + newLevel);
                            }
                        }
                    }
                }
                break;
            }
                
            case "ADMIN":
                handleAdmin(dkClient, m);
                break;
                
            default:
                System.out.println("[DK] Comando no reconocido: " + m.command);
                break;
        }
    }
    
    /**
     * Maneja la conexión de un cliente
     */
    private void handleConnect(DKClientHandler client, MessageProtocol.Message m) {
        if (!m.hasParams(2)) {
            sendTo(client, MessageProtocol.encode("ERROR", "INVALID_PARAMS", 
                "Expected: CONNECT|TYPE|NAME[|ROOM_ID]"));
            return;
        }
        
        String type = m.getParam(0);
        String name = m.getParam(1);
        
        if (type.equals("PLAYER")) {
            // Verificar si ya hay demasiados jugadores
            if (playerCount >= 2) { // Máximo 2 jugadores
                sendTo(client, MessageProtocol.encode("ERROR", "MAX_PLAYERS_REACHED", 
                    "Ya hay 2 jugadores activos"));
                disconnectClient(client);
                return;
            }
            
            // Crear nueva sala para este jugador
            int roomId = nextRoomId.getAndIncrement();
            int playerId = roomId; // Usar roomId como playerId
            
            GameRoom room = new GameRoom(roomId, playerId, name);
            gameRooms.put(roomId, room);
            playerCount++;
            
            client.setPlayerId(playerId);
            client.setPlayerName(name);
            client.setGameRoomId(roomId);
            client.setClientType(DKClientHandler.ClientType.PLAYER);
            
            sendTo(client, MessageProtocol.encode("OK", "PLAYER_ID", 
                String.valueOf(playerId), "ROOM_ID", String.valueOf(roomId), 
                "LIVES", "3", "SCORE", "0"));
            
            System.out.println("[DK] Jugador conectado: " + name + 
                " (ID: " + playerId + ", Sala: " + roomId + ")");
            
            // Notificar a posibles espectadores de esta sala
            String joinMsg = MessageProtocol.encode("PLAYER_JOINED", 
                String.valueOf(playerId), name);
            broadcastToRoom(roomId, joinMsg);
            
        } else if (type.equals("SPECTATOR")) {
            // Espectador debe especificar qué sala observar
            int targetRoomId = 1; // Default: sala 1
            
            if (m.hasParams(3)) {
                targetRoomId = m.getParamAsInt(2, 1);
            }
            
            // Verificar que la sala existe
            GameRoom targetRoom = getRoomById(targetRoomId);
            if (targetRoom == null) {
                sendTo(client, MessageProtocol.encode("ERROR", "ROOM_NOT_FOUND", 
                    "La sala " + targetRoomId + " no existe. Use 'rooms' para ver salas activas."));
                disconnectClient(client);
                return;
            }
            
            // Verificar límite de espectadores (máximo 2 por sala)
            if (targetRoom.spectatorCount >= 2) {
                sendTo(client, MessageProtocol.encode("ERROR", "MAX_SPECTATORS_REACHED", 
                    "La sala " + targetRoomId + " ya tiene 2 espectadores"));
                disconnectClient(client);
                return;
            }
            
            spectatorCount++;
            targetRoom.addSpectator();
            
            client.setPlayerName(name);
            client.setGameRoomId(targetRoomId);
            client.setClientType(DKClientHandler.ClientType.SPECTATOR);
            
            sendTo(client, MessageProtocol.encode("OK", "SPECTATOR_ID", 
                String.valueOf(spectatorCount), "ROOM_ID", String.valueOf(targetRoomId)));
            
            System.out.println("[DK] Espectador conectado: " + name + 
                " (Observando Sala: " + targetRoomId + ")");
            
            // Enviar estado actual de la sala al espectador (todos los enemigos y frutas activos)
            if (targetRoom.gameLogic != null) {
                List<String> initialMessages = targetRoom.gameLogic.getInitialStateMessages(targetRoomId);
                for (String msg : initialMessages) {
                    sendTo(client, msg);
                }
                System.out.println("[DK] Enviados " + initialMessages.size() + " mensajes iniciales al espectador en sala " + targetRoomId);
            }
            
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
     * Maneja comandos de administrador
     */
    private void handleAdmin(DKClientHandler client, MessageProtocol.Message m) {
        if (!client.isAdmin()) {
            sendTo(client, MessageProtocol.encode("ERROR", "NOT_ADMIN"));
            return;
        }
        
        if (!m.hasParams(1)) {
            sendTo(client, MessageProtocol.encode("ERROR", "INVALID_ADMIN_COMMAND"));
            return;
        }
        
        String subCommand = m.getParam(0);
        
        switch (subCommand) {
            case "CREATE_FRUIT": {
                if (!m.hasParams(5)) {
                    sendTo(client, MessageProtocol.encode("ERROR", "INVALID_PARAMS", 
                        "Formato: ADMIN|CREATE_FRUIT|ROOM_ID|VINE|HEIGHT|POINTS"));
                    return;
                }
                
                int targetRoomId = m.getParamAsInt(1, -1);
                int vine = m.getParamAsInt(2, -1);
                int height = m.getParamAsInt(3, -1);
                int points = m.getParamAsInt(4, 100);
                
                GameRoom room = getRoomById(targetRoomId);
                if (room == null) {
                    sendTo(client, MessageProtocol.encode("ERROR", "ROOM_NOT_FOUND", 
                        "Sala " + targetRoomId + " no existe"));
                    return;
                }
                
                if (vine < 1 || vine > 9) {
                    sendTo(client, MessageProtocol.encode("ERROR", "INVALID_VINE"));
                    return;
                }
                
                // Crear fruta en la sala específica
                int fid = room.createFruit(vine, height, points);
                
                // Enviar mensaje directo al cliente (formato que espera el cliente C)
                String msgToSend = MessageProtocol.encode("FRUIT_CREATED",
                    String.valueOf(fid), 
                    String.valueOf(vine),
                    String.valueOf(height), 
                    String.valueOf(points),
                    String.valueOf(targetRoomId));
                
                System.out.println("[ADMIN DEBUG] Enviando mensaje: " + msgToSend);
                System.out.println("[ADMIN DEBUG] Clientes en sala " + targetRoomId + ": " + countClientsInRoom(targetRoomId));
                
                broadcastToRoom(targetRoomId, msgToSend);
                sendTo(client, MessageProtocol.encode("OK", "FRUIT_CREATED", 
                    "id=" + fid, "room=" + targetRoomId));
                
                System.out.println("[ADMIN] Fruta creada: id=" + fid + 
                    " sala=" + targetRoomId + " liana=" + vine + " altura=" + height + " puntos=" + points);
                break;
            }
                
            case "CREATE_CCA": {
                if (!m.hasParams(3)) {
                    sendTo(client, MessageProtocol.encode("ERROR", "INVALID_PARAMS",
                        "Formato: ADMIN|CREATE_CCA|ROOM_ID|VINE"));
                    return;
                }
                
                int targetRoomId = m.getParamAsInt(1, -1);
                int vine = m.getParamAsInt(2, -1);
                
                GameRoom room = getRoomById(targetRoomId);
                if (room == null) {
                    sendTo(client, MessageProtocol.encode("ERROR", "ROOM_NOT_FOUND"));
                    return;
                }
                
                room.createEnemy("BLUE", vine, 0);
                
                // Enviar mensaje directo al cliente
                String msgToSend = MessageProtocol.encode("CCA_CREATED",
                    String.valueOf(vine), 
                    "0", 
                    "0",
                    String.valueOf(targetRoomId));
                
                System.out.println("[ADMIN DEBUG] Enviando mensaje: " + msgToSend);
                broadcastToRoom(targetRoomId, msgToSend);
                sendTo(client, MessageProtocol.encode("OK", "CCA_CREATED"));
                
                System.out.println("[ADMIN] Cocodrilo Azul creado en sala " + targetRoomId + " liana=" + vine);
                break;
            }
                
            case "CREATE_CCR": {
                if (!m.hasParams(3)) {
                    sendTo(client, MessageProtocol.encode("ERROR", "INVALID_PARAMS",
                        "Formato: ADMIN|CREATE_CCR|ROOM_ID|VINE"));
                    return;
                }
                
                int targetRoomId = m.getParamAsInt(1, -1);
                int vine = m.getParamAsInt(2, -1);
                
                GameRoom room = getRoomById(targetRoomId);
                if (room == null) {
                    sendTo(client, MessageProtocol.encode("ERROR", "ROOM_NOT_FOUND"));
                    return;
                }
                
                room.createEnemy("RED", vine, 0);
                
                // Enviar mensaje directo al cliente
                String msgToSend = MessageProtocol.encode("CCR_CREATED",
                    String.valueOf(vine), 
                    "0", 
                    "0",
                    String.valueOf(targetRoomId));
                
                System.out.println("[ADMIN DEBUG] Enviando mensaje: " + msgToSend);
                broadcastToRoom(targetRoomId, msgToSend);
                sendTo(client, MessageProtocol.encode("OK", "CCR_CREATED"));
                
                System.out.println("[ADMIN] Cocodrilo Rojo creado en sala " + targetRoomId + " liana=" + vine);
                break;
            }
                
            case "DELETE_FRUIT_BY_ID": {
    if (!m.hasParams(3)) {
        sendTo(client, MessageProtocol.encode("ERROR", "INVALID_PARAMS",
            "Formato: ADMIN|DELETE_FRUIT_BY_ID|ROOM_ID|FRUIT_ID"));
        return;
    }
    
    int targetRoomId = m.getParamAsInt(1, -1);
    int fid = m.getParamAsInt(2, -1);
    
    GameRoom room = getRoomById(targetRoomId);
    if (room == null) {
        sendTo(client, MessageProtocol.encode("ERROR", "ROOM_NOT_FOUND"));
        return;
    }
    
    int[] info = room.deleteFruit(fid);
    
    if (info != null) {
        int points = info[2];
        String delMsg = MessageProtocol.encode("FRUIT_DELETED",
            String.valueOf(fid), "0", String.valueOf(points));
        broadcastToRoom(targetRoomId, delMsg);
        sendTo(client, MessageProtocol.encode("OK", "FRUIT_DELETED", String.valueOf(fid)));
    } else {
        sendTo(client, MessageProtocol.encode("ERROR", "FRUIT_NOT_FOUND"));
    }
    break;
}
    }
}
    @Override
    public void disconnectClient(ClientHandler client) {
        if (client instanceof DKClientHandler) {
            DKClientHandler dkClient = (DKClientHandler) client;
            
            if (dkClient.isPlayer()) {
                Integer roomId = dkClient.getGameRoomId();
                if (roomId != null) {
                    // Remover la sala cuando el jugador se desconecta
                    GameRoom room = gameRooms.remove(roomId);
                    if (room != null) {
                        playerCount--;
                        System.out.println("[DK] Sala " + roomId + " cerrada (Jugador: " + room.playerName + ")");
                        
                        // Notificar a espectadores de esta sala
                        String leftMsg = MessageProtocol.encode("PLAYER_LEFT", 
                            String.valueOf(dkClient.getPlayerId()));
                        broadcastToRoom(roomId, leftMsg);
                        
                        // Desconectar a todos los espectadores de esta sala
                        for (ClientHandler c : clients.toArray(new ClientHandler[0])) {
                            if (c instanceof DKClientHandler) {
                                DKClientHandler other = (DKClientHandler) c;
                                if (other.isSpectator() && roomId.equals(other.getGameRoomId())) {
                                    disconnectClient(other);
                                }
                            }
                        }
                    }
                }
            } else if (dkClient.isSpectator()) {
                Integer roomId = dkClient.getGameRoomId();
                if (roomId != null) {
                    GameRoom room = getRoomById(roomId);
                    if (room != null) {
                        room.removeSpectator();
                        spectatorCount = Math.max(0, spectatorCount - 1);
                    }
                }
            }
        }
        
        super.disconnectClient(client);
    }
    
    // Método para obtener estadísticas del servidor
    public String getStats() {
        StringBuilder stats = new StringBuilder();
        stats.append("=== ESTADÍSTICAS DEL SERVIDOR ===\n");
        stats.append("Jugadores activos: ").append(playerCount).append("\n");
        stats.append("Espectadores activos: ").append(spectatorCount).append("\n");
        stats.append("Salas activas: ").append(gameRooms.size()).append("\n");
        stats.append("Clientes totales conectados: ").append(clients.size()).append("\n");
        stats.append("================================");
        return stats.toString();
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
            System.out.println("  rooms  - Mostrar salas activas");
            System.out.println("  cf     - Crear Fruta en sala específica");
            System.out.println("  cca    - Crear Cocodrilo Azul en sala específica");
            System.out.println("  ccr    - Crear Cocodrilo Rojo en sala específica");
            System.out.println("  df     - Delete Fruit (interactive)");
            System.out.println("  deletef <id> - Delete Fruit by id inline");
            System.out.println("  level <room> <nivel> - Cambiar nivel de una sala");
            System.out.println("  debug  - Mostrar debug info");
            System.out.println("  quit   - Detener servidor");
            System.out.println();
            
            while (scanner.hasNextLine()) {
                String line = scanner.nextLine().trim();
                String lower = line.toLowerCase();
                
                if (lower.equals("quit") || lower.equals("exit") || lower.equals("stop")) {
                    break;
                    
                } else if (lower.equals("stats")) {
                    System.out.println("\n" + server.getStats() + "\n");
                    
                } else if (lower.equals("debug")) {
                    System.out.println("\n=== DEBUG INFO ===");
                    System.out.println("Total clientes: " + server.clients.size());
                    for (int i = 0; i < server.clients.size(); i++) {
                        ClientHandler client = server.clients.get(i);
                        if (client instanceof DKClientHandler) {
                            DKClientHandler dkClient = (DKClientHandler) client;
                            System.out.println("Cliente " + i + ": " + 
                                dkClient.getClientType() + " " + dkClient.getPlayerName() + 
                                " (Sala: " + dkClient.getGameRoomId() + ")");
                        }
                    }
                    System.out.println("==================\n");
                    
                } else if (lower.equals("rooms")) {
                    System.out.println("\n╔═════════════════════════════════════════════════════════════╗");
                    System.out.println("║                     SALAS ACTIVAS                           ║");
                    System.out.println("╠═════╦═══════════╦═══════╦═════════╦═══════════╦═════════════╣");
                    System.out.println("║ Sala║ Jugador   ║ Nivel ║ Enemigos║ Frutas    ║ Espectadores║");
                    System.out.println("╠═════╬═══════════╬═══════╬═════════╬═══════════╬═════════════╣");
                    
                    for (Map.Entry<Integer, GameRoom> entry : server.gameRooms.entrySet()) {
                        GameRoom room = entry.getValue();
                        int level = room.getCurrentLevel();
                        int enemies = room.getEnemyCount();
                        int fruits = room.getFruitCount();
                        int spectators = room.spectatorCount;
                        
                        System.out.printf("║  %-2d ║ %-9s ║  %-4d ║  %-6d ║  %-8d ║  %-10d ║\n",
                            room.roomId, room.playerName, level, enemies, fruits, spectators);
                    }
                    System.out.println("╚═════╩═══════════╩═══════╩═════════╩═══════════╩═════════════╝\n");
                    
                } else if (lower.startsWith("level ")) {
                    // Formato: level <room> <nivel>
                    String[] parts = line.split("\\s+");
                    if (parts.length >= 3) {
                        try {
                            int roomId = Integer.parseInt(parts[1]);
                            int newLevel = Integer.parseInt(parts[2]);
                            
                            if (newLevel < 1 || newLevel > 3) {
                                System.out.println("[SERVER] ERROR: Nivel debe estar entre 1 y 3");
                                continue;
                            }
                            
                            GameRoom room = server.getRoomById(roomId);
                            if (room == null) {
                                System.out.println("[SERVER] ERROR: Sala " + roomId + " no existe");
                                continue;
                            }
                            
                            if (room.gameLogic != null) {
                                room.gameLogic.changeLevel(newLevel);
                                System.out.println("[SERVER] ✓ Sala " + roomId + " cambiada a nivel " + newLevel);
                            }
                            
                        } catch (NumberFormatException e) {
                            System.out.println("[SERVER] ERROR: Formato: level <sala> <nivel>");
                        }
                    } else {
                        System.out.println("[SERVER] Uso: level <sala> <nivel>");
                    }
                    
                } else if (lower.equals("cf")) {
                    System.out.print("Ingrese número de sala (1-" + server.gameRooms.size() + "): ");
                    String roomStr = scanner.nextLine().trim();
                    
                    int roomId;
                    try {
                        roomId = Integer.parseInt(roomStr);
                        GameRoom room = server.getRoomById(roomId);
                        if (room == null) {
                            System.out.println("[SERVER] ERROR: Sala " + roomId + " no existe");
                            continue;
                        }
                    } catch (NumberFormatException e) {
                        System.out.println("[SERVER] ERROR: Número de sala inválido");
                        continue;
                    }
                    
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

                    System.out.print("Ingrese altura en la liana (100-700): ");
                    String inputHeight = scanner.nextLine().trim();
                    
                    int height;
                    try {
                        height = Integer.parseInt(inputHeight);
                        if (height < 100 || height > 700) {
                            System.out.println("[SERVER] ERROR: Altura debe estar entre 100 y 700");
                            continue;
                        }
                    } catch (NumberFormatException e) {
                        System.out.println("[SERVER] ERROR: Altura inválida");
                        continue;
                    }

                    System.out.print("Ingrese puntos de la fruta (50-500): ");
                    String inputPoints = scanner.nextLine().trim();
                    
                    int points = 100;
                    try {
                        points = Integer.parseInt(inputPoints);
                        if (points < 50 || points > 500) {
                            System.out.println("[SERVER] Usando puntos por defecto: 100");
                            points = 100;
                        }
                    } catch (NumberFormatException e) {
                        System.out.println("[SERVER] Usando puntos por defecto: 100");
                    }

                    // Crear fruta en la sala específica
                    GameRoom room = server.getRoomById(roomId);
                    if (room != null) {
                        int fid = room.createFruit(vine, height, points);
                        
                        // Broadcast solo a esa sala
                        String msg = MessageProtocol.encode("FRUIT_CREATED",
                            String.valueOf(fid),
                            String.valueOf(vine),
                            String.valueOf(height),
                            String.valueOf(points),
                            String.valueOf(roomId));
                        
                        server.broadcastToRoom(roomId, msg);
                        
                        System.out.println("[SERVER] ✓ Fruta creada (id=" + fid + ") en Sala " + roomId + 
                            ", liana " + vine + ", altura " + height + ", puntos " + points);
                    }
                    
                } else if (lower.equals("cca")) {
                    System.out.print("Ingrese número de sala (1-" + server.gameRooms.size() + "): ");
                    String roomStr = scanner.nextLine().trim();
                    
                    int roomId;
                    try {
                        roomId = Integer.parseInt(roomStr);
                        GameRoom room = server.getRoomById(roomId);
                        if (room == null) {
                            System.out.println("[SERVER] ERROR: Sala " + roomId + " no existe");
                            continue;
                        }
                    } catch (NumberFormatException e) {
                        System.out.println("[SERVER] ERROR: Número de sala inválido");
                        continue;
                    }
                    
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

                    System.out.print("Ingrese velocidad (0.5-3.0, 0 para predeterminado): ");
                    String inputSpeed = scanner.nextLine().trim();
                    
                    float speed = 0; // 0 significa usar velocidad por defecto según nivel
                    try {
                        speed = Float.parseFloat(inputSpeed);
                        if (speed < 0) speed = 0;
                    } catch (NumberFormatException e) {
                        speed = 0;
                    }

                    // Crear enemigo en la sala específica
                    GameRoom room = server.getRoomById(roomId);
                    if (room != null) {
                        room.createEnemy("BLUE", vine, speed);
                        
                        String msg = MessageProtocol.encode("CCA_CREATED",
                            String.valueOf(vine),
                            "0",
                            "0",
                            String.valueOf(roomId));
                        
                        server.broadcastToRoom(roomId, msg);
                        
                        System.out.println("[SERVER] ✓ Cocodrilo AZUL creado en Sala " + roomId + 
                            ", liana " + vine + (speed > 0 ? ", velocidad " + speed : ""));
                    }
                    
                } else if (lower.equals("ccr")) {
                    System.out.print("Ingrese número de sala (1-" + server.gameRooms.size() + "): ");
                    String roomStr = scanner.nextLine().trim();
                    
                    int roomId;
                    try {
                        roomId = Integer.parseInt(roomStr);
                        GameRoom room = server.getRoomById(roomId);
                        if (room == null) {
                            System.out.println("[SERVER] ERROR: Sala " + roomId + " no existe");
                            continue;
                        }
                    } catch (NumberFormatException e) {
                        System.out.println("[SERVER] ERROR: Número de sala inválido");
                        continue;
                    }
                    
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

                    System.out.print("Ingrese velocidad (0.5-3.0, 0 para predeterminado): ");
                    String inputSpeed = scanner.nextLine().trim();
                    
                    float speed = 0; // 0 significa usar velocidad por defecto según nivel
                    try {
                        speed = Float.parseFloat(inputSpeed);
                        if (speed < 0) speed = 0;
                    } catch (NumberFormatException e) {
                        speed = 0;
                    }

                    // Crear enemigo en la sala específica
                    GameRoom room = server.getRoomById(roomId);
                    if (room != null) {
                        room.createEnemy("RED", vine, speed);
                        
                        String msg = MessageProtocol.encode("CCR_CREATED",
                            String.valueOf(vine),
                            "0",
                            "0",
                            String.valueOf(roomId));
                        
                        server.broadcastToRoom(roomId, msg);
                        
                        System.out.println("[SERVER] ✓ Cocodrilo ROJO creado en Sala " + roomId + 
                            ", liana " + vine + (speed > 0 ? ", velocidad " + speed : ""));
                    }
                    
                  } else if (lower.equals("df")) {
    System.out.print("Ingrese número de sala (1-" + server.gameRooms.size() + "): ");
    String roomStr = scanner.nextLine().trim();
    
    int roomId;
    try {
        roomId = Integer.parseInt(roomStr);
        GameRoom room = server.getRoomById(roomId);
        if (room == null) {
            System.out.println("[SERVER] ERROR: Sala " + roomId + " no existe");
            continue;
        }
    } catch (NumberFormatException e) {
        System.out.println("[SERVER] ERROR: Número de sala inválido");
        continue;
    }
    
    // Interactive delete by id
    System.out.print("Ingrese ID de fruta a eliminar: ");
    String idStr = scanner.nextLine().trim();
    try {
        int fid = Integer.parseInt(idStr);
        GameRoom room = server.getRoomById(roomId);
        if (room != null) {
            // CORREGIDO: Solo llamar a deleteFruit una vez, nota para documentación: pasaba en ambas partidas
            int[] info = room.deleteFruit(fid);
            
            if (info != null) {
                //int vine = info[0];
                //int height = info[1];
                int points = info[2];
                
                String msg = MessageProtocol.encode("FRUIT_DELETED", 
                    String.valueOf(fid), "0", String.valueOf(points));
                server.broadcastToRoom(roomId, msg);
                System.out.println("[SERVER] ✓ Fruta id=" + fid + " eliminada de Sala " + roomId);
            } else {
                System.out.println("[SERVER] ERROR: No existe fruta con id=" + fid + " en Sala " + roomId);
            }
        }
    } catch (NumberFormatException e) {
        System.out.println("[SERVER] ERROR: ID inválido");
    }
    
} else if (lower.startsWith("deletef ")) {
    String[] parts = line.split("\\s+");
    if (parts.length >= 3) {
        try {
            int roomId = Integer.parseInt(parts[1]);
            int fid = Integer.parseInt(parts[2]);
            
            GameRoom room = server.getRoomById(roomId);
            if (room == null) {
                System.out.println("[SERVER] ERROR: Sala " + roomId + " no existe");
                continue;
            }

            int[] info = room.deleteFruit(fid);
            
            if (info != null) {
                //int vine = info[0];
                //int height = info[1];
                int points = info[2];
                
                String msg = MessageProtocol.encode("FRUIT_DELETED", 
                    String.valueOf(fid), "0", String.valueOf(points));
                server.broadcastToRoom(roomId, msg);
                System.out.println("[SERVER] ✓ Fruta id=" + fid + " eliminada de Sala " + roomId);
            } else {
                System.out.println("[SERVER] ERROR: No existe fruta con id=" + fid + " en Sala " + roomId);
            }
        } catch (NumberFormatException e) {
            System.out.println("[SERVER] ERROR: Formato: deletef <sala> <id>");
        }
    } else {
        System.out.println("[SERVER] Uso: deletef <sala> <id>");
    }
    
} else {
    System.out.println("[SERVER] Comando desconocido: " + line);
    System.out.println("Comandos disponibles: stats, rooms, cf, cca, ccr, df, deletef <sala> <id>, level <sala> <nivel>, debug, quit");
}   }
        
        System.out.println("\n[SERVER] Deteniendo servidor...");
        server.stop();
        System.exit(0);
    }
}}