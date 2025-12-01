package GameServer.DonkeyKong.Game.Observer;

/**
 * Representa un evento del juego que será notificado a los observadores
 * 
 * @author Anthony Artavia & Ariel Saborio
 */
public class GameEvent {
    
    // Tipos de eventos posibles
    public enum EventType {
        PLAYER_JOINED,           // Un jugador se conectó
        PLAYER_LEFT,             // Un jugador se desconectó
        PLAYER_DIED,             // Un jugador murió
        ENEMY_SPAWNED,           // Se creó un enemigo
        ENEMY_DEFEATED,          // Se derrotó un enemigo
        FRUIT_COLLECTED,         // Se recolectó una fruta
        FRUIT_SPAWNED,           // Se creó una fruta
        LEVEL_CHANGED,           // Se cambió de nivel
        GAME_STATE_UPDATED,      // Estado del juego actualizado
        COLLISION_DETECTED       // Se detectó una colisión
    }
    
    private EventType type;
    private long timestamp;
    private Object source;
    private String data;
    private int playerId;
    private int entityId;
    
    /**
     * Constructor completo
     */
    public GameEvent(EventType type, Object source, String data, int playerId, int entityId) {
        this.type = type;
        this.source = source;
        this.data = data;
        this.playerId = playerId;
        this.entityId = entityId;
        this.timestamp = System.currentTimeMillis();
    }
    
    /**
     * Constructor simplificado
     */
    public GameEvent(EventType type, Object source, String data) {
        this(type, source, data, -1, -1);
    }
    
    /**
     * Constructor mínimo
     */
    public GameEvent(EventType type, String data) {
        this(type, null, data, -1, -1);
    }
    
    // ===== GETTERS =====
    
    public EventType getType() {
        return type;
    }
    
    public long getTimestamp() {
        return timestamp;
    }
    
    public Object getSource() {
        return source;
    }
    
    public String getData() {
        return data;
    }
    
    public int getPlayerId() {
        return playerId;
    }
    
    public int getEntityId() {
        return entityId;
    }
    
    @Override
    public String toString() {
        return String.format("[GameEvent] Type: %s, Timestamp: %d, PlayerId: %d, EntityId: %d, Data: %s",
                type, timestamp, playerId, entityId, data);
    }
}
