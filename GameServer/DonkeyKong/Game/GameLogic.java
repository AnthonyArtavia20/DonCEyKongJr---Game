package GameServer.DonkeyKong.Game;

import GameServer.DonkeyKong.Game.factory.*;
import GameServer.DonkeyKong.Game.model.*;
import GameServer.DonkeyKong.Game.Observer.*;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * Lógica central del juego DonCEy Kong Jr
 * Maneja jugadores, enemigos, frutas y el patrón Factory
 * 
 * @author Anthony Artavia & Ariel Saborio
 */
public class GameLogic {
    
    // ===== FACTORY PATTERN =====
    private EnemyFactory currentFactory;
    private int currentLevel;
    
    // ===== OBSERVER PATTERN =====
    private final BroadcastManager broadcastManager;
    
    // ===== COLECCIONES THREAD-SAFE =====
    private final CopyOnWriteArrayList<Enemy> enemies;
    private final CopyOnWriteArrayList<Fruit> fruits;
    private final ConcurrentHashMap<Integer, PlayerState> players;
    
    // ===== GENERADORES DE IDs =====
    private final AtomicInteger nextEnemyId;
    private final AtomicInteger nextFruitId;
    
    // ===== MAPEO DE LIANAS (igual que en C) =====
    private static final float[] VINE_X_POSITIONS = {
        75f, 200f, 300f, 490f, 650f, 750f, 850f, 950f, 1075f
    };
    
    // Clase interna para estado del jugador
    private static class PlayerState {
        String name;
        float x, y;
        int lives;
        int score;
        
        PlayerState(String name) {
            this.name = name;
            this.x = 50;
            this.y = 100;
            this.lives = 3;
            this.score = 0;
        }
    }
    
    // ===== CONSTRUCTOR =====
    public GameLogic() {
        this.currentLevel = 1;
        this.currentFactory = new Level1EnemyFactory();
        
        this.enemies = new CopyOnWriteArrayList<>();
        this.fruits = new CopyOnWriteArrayList<>();
        this.players = new ConcurrentHashMap<>();
        
        this.nextEnemyId = new AtomicInteger(1);
        this.nextFruitId = new AtomicInteger(1);
        
        // Inicializar el BroadcastManager (Singleton)
        this.broadcastManager = BroadcastManager.getInstance();
        
        System.out.println("[GameLogic] Inicializado - Nivel 1, Factory: Level1EnemyFactory");
    }
    
    // ===== ACTUALIZACIÓN DEL JUEGO (60 FPS) =====
    public void update(double delta) {
        // Actualizar todos los enemigos
        for (Enemy enemy : enemies) {
            if (enemy.isActive()) {
                enemy.update(delta);
                
                // Si el enemigo se desactivó, removerlo de la lista
                if (!enemy.isActive()) {
                    enemies.remove(enemy);
                    System.out.println("[GameLogic] Enemigo " + enemy.getId() + " eliminado (inactivo)");
                }
            }
        }
        
        // Las frutas no necesitan update (son estáticas)
    }
    
    // ===== SERIALIZACIÓN PARA ENVIAR A CLIENTES =====
    public String serialize() {
        StringBuilder sb = new StringBuilder();
        
        // GAMESTATE|timestamp|PLAYERS|...|ENEMIES|...|FRUITS|...
        sb.append("GAMESTATE|");
        sb.append(System.currentTimeMillis()).append("|");
        
        // PLAYERS
        sb.append("PLAYERS|");
        for (PlayerState p : players.values()) {
            sb.append(String.format("%s,%.0f,%.0f,%d,%d|", 
                p.name, p.x, p.y, p.lives, p.score));
        }
        
        // ENEMIES
        sb.append("ENEMIES|");
        for (Enemy e : enemies) {
            if (e.isActive()) {
                sb.append(e.serialize()).append("|");
            }
        }
        
        // FRUITS
        sb.append("FRUITS|");
        for (Fruit f : fruits) {
            if (f.isActive()) {
                sb.append(f.serialize()).append("|");
            }
        }
        
        return sb.toString();
    }
    
    // ===== GESTIÓN DE JUGADORES =====
    public void addPlayer(int id, String name) {
        players.put(id, new PlayerState(name));
        System.out.println("[GameLogic] Jugador agregado: " + name + " (ID: " + id + ")");
        
        // DISPARAR EVENTO: PLAYER_JOINED
        GameEvent event = new GameEvent(
            GameEvent.EventType.PLAYER_JOINED,
            this,
            "Jugador " + name + " se conectó",
            id,
            -1
        );
        broadcastManager.notify(event);
    }
    
    public void removePlayer(int id) {
        PlayerState removed = players.remove(id);
        if (removed != null) {
            System.out.println("[GameLogic] Jugador removido: " + removed.name + " (ID: " + id + ")");
            
            // DISPARAR EVENTO: PLAYER_LEFT
            GameEvent event = new GameEvent(
                GameEvent.EventType.PLAYER_LEFT,
                this,
                "Jugador " + removed.name + " se desconectó",
                id,
                -1
            );
            broadcastManager.notify(event);
        }
    }
    
    public void movePlayer(int id, String direction) {
        PlayerState player = players.get(id);
        if (player == null) return;
        
        // Aquí tu compañero puede implementar la física del servidor
        // Por ahora solo log
        System.out.println("[GameLogic] Jugador " + id + " se mueve " + direction);
    }
    
    // ===== CREACIÓN DE ENEMIGOS USANDO FACTORY =====
    public void createRedCrocodile(int vineId, float speed) {
        if (vineId < 1 || vineId > 9) {
            System.err.println("[GameLogic] ERROR: Liana inválida: " + vineId);
            return;
        }
        
        int id = nextEnemyId.getAndIncrement();
        float x = getVineXPosition(vineId);
        float y = 0; // Inicia arriba
        
        // USAR LA FACTORY para crear el enemigo
        RedCrocodile croc = currentFactory.createRedCrocodile(id, vineId, x, y);
        
        // Configurar límites de la liana (esto debería venir del mapa)
        croc.setVineLimits(50, 700); // Ejemplo: desde Y=50 hasta Y=700
        
        enemies.add(croc);
        
        System.out.println("[GameLogic] Cocodrilo ROJO creado - ID: " + id + 
                          ", Liana: " + vineId + ", Nivel: " + currentLevel);
        
        // DISPARAR EVENTO: ENEMY_SPAWNED
        GameEvent event = new GameEvent(
            GameEvent.EventType.ENEMY_SPAWNED,
            this,
            "RedCrocodile creado en liana " + vineId,
            -1,
            id
        );
        broadcastManager.notify(event);
    }
    
    public void createBlueCrocodile(int vineId, float speed) {
        if (vineId < 1 || vineId > 9) {
            System.err.println("[GameLogic] ERROR: Liana inválida: " + vineId);
            return;
        }
        
        int id = nextEnemyId.getAndIncrement();
        float x = getVineXPosition(vineId);
        float y = 0; // Inicia arriba
        
        // USAR LA FACTORY para crear el enemigo
        BlueCrocodile croc = currentFactory.createBlueCrocodile(id, vineId, x, y);
        enemies.add(croc);
        
        System.out.println("[GameLogic] Cocodrilo AZUL creado - ID: " + id + 
                          ", Liana: " + vineId + ", Nivel: " + currentLevel);
        
        // DISPARAR EVENTO: ENEMY_SPAWNED
        GameEvent event = new GameEvent(
            GameEvent.EventType.ENEMY_SPAWNED,
            this,
            "BlueCrocodile creado en liana " + vineId,
            -1,
            id
        );
        broadcastManager.notify(event);
    }
    
    public void createFruit(int vineId, int height, int points) {
        if (vineId < 1 || vineId > 9) {
            System.err.println("[GameLogic] ERROR: Liana inválida: " + vineId);
            return;
        }
        
        int id = nextFruitId.getAndIncrement();
        
        // USAR LA FACTORY (ignora los puntos del parámetro, usa los del nivel)
        Fruit fruit = currentFactory.createFruit(id, vineId, height);
        fruits.add(fruit);
        
        System.out.println("[GameLogic] Fruta creada - ID: " + id + 
                          ", Liana: " + vineId + ", Y: " + height + 
                          ", Puntos: " + fruit.getPoints());
        
        // DISPARAR EVENTO: FRUIT_SPAWNED
        GameEvent event = new GameEvent(
            GameEvent.EventType.FRUIT_SPAWNED,
            this,
            "Fruta creada en liana " + vineId + " con " + fruit.getPoints() + " puntos",
            -1,
            id
        );
        broadcastManager.notify(event);
    }
    
    public boolean deleteFruit(int vineId, int height) {
        // Buscar fruta en esa posición
        for (Fruit fruit : fruits) {
            if (fruit.getVineId() == vineId && 
                Math.abs(fruit.getY() - height) < 50) { // Tolerancia de 50px
                
                fruits.remove(fruit);
                System.out.println("[GameLogic] Fruta eliminada - ID: " + fruit.getId());
                
                // DISPARAR EVENTO: FRUIT_COLLECTED
                GameEvent event = new GameEvent(
                    GameEvent.EventType.FRUIT_COLLECTED,
                    this,
                    "Fruta recolectada, puntos: " + fruit.getPoints(),
                    -1,
                    fruit.getId()
                );
                broadcastManager.notify(event);
                
                return true;
            }
        }
        
        System.out.println("[GameLogic] Fruta NO encontrada en liana " + vineId + ", altura " + height);
        return false;
    }
    
    // ===== MANEJO DE COLISIONES =====
    public void enemyHit(int playerId, int enemyId, int damage) {
        PlayerState player = players.get(playerId);
        if (player == null) {
            System.err.println("[GameLogic] ERROR: Jugador " + playerId + " no existe");
            return;
        }
        
        // Restar vida
        player.lives -= damage;
        System.out.println("[GameLogic] Jugador " + player.name + 
                          " golpeado por enemigo " + enemyId + 
                          " - Vidas: " + player.lives);
        
        // DISPARAR EVENTO: COLLISION_DETECTED
        GameEvent collisionEvent = new GameEvent(
            GameEvent.EventType.COLLISION_DETECTED,
            this,
            "Colisión entre jugador " + player.name + " y enemigo " + enemyId,
            playerId,
            enemyId
        );
        broadcastManager.notify(collisionEvent);
        
        // Buscar y eliminar el enemigo
        for (Enemy enemy : enemies) {
            if (enemy.getId() == enemyId) {
                enemy.setActive(false);
                enemies.remove(enemy);
                System.out.println("[GameLogic] Enemigo " + enemyId + " eliminado tras colisión");
                
                // DISPARAR EVENTO: ENEMY_DEFEATED
                GameEvent defeatedEvent = new GameEvent(
                    GameEvent.EventType.ENEMY_DEFEATED,
                    this,
                    "Enemigo " + enemyId + " fue derrotado",
                    playerId,
                    enemyId
                );
                broadcastManager.notify(defeatedEvent);
                
                break;
            }
        }
        
        // Si el jugador murió
        if (player.lives <= 0) {
            System.out.println("[GameLogic] Jugador " + player.name + " MUERTO");
            
            // DISPARAR EVENTO: PLAYER_DIED
            GameEvent diedEvent = new GameEvent(
                GameEvent.EventType.PLAYER_DIED,
                this,
                "Jugador " + player.name + " ha muerto",
                playerId,
                -1
            );
            broadcastManager.notify(diedEvent);
        }
    }
    
    // ===== CAMBIO DE NIVEL =====
    public void changeLevel(int newLevel) {
        this.currentLevel = newLevel;
        
        // CAMBIAR LA FACTORY según el nivel
        switch (newLevel) {
            case 1:
                currentFactory = new Level1EnemyFactory();
                break;
            case 2:
                currentFactory = new Level2EnemyFactory();
                break;
            case 3:
            default:
                currentFactory = new Level3EnemyFactory();
                break;
        }
        
        System.out.println("[GameLogic] Nivel cambiado a " + newLevel + 
                          " - Factory: " + currentFactory.getClass().getSimpleName());
        
        // DISPARAR EVENTO: LEVEL_CHANGED
        GameEvent event = new GameEvent(
            GameEvent.EventType.LEVEL_CHANGED,
            this,
            "Nivel cambió a " + newLevel,
            -1,
            newLevel
        );
        broadcastManager.notify(event);
    }
    
    // ===== UTILIDADES =====
    private float getVineXPosition(int vineId) {
        if (vineId < 1 || vineId > VINE_X_POSITIONS.length) {
            return VINE_X_POSITIONS[0]; // Default
        }
        return VINE_X_POSITIONS[vineId - 1]; // vineId empieza en 1
    }
    
    public int getCurrentLevel() {
        return currentLevel;
    }
    
    public int getEnemyCount() {
        return enemies.size();
    }
    
    public int getFruitCount() {
        return fruits.size();
    }
    
    /**
     * Permite suscribir un observador para recibir eventos del juego
     */
    public void subscribe(GameObserver observer) {
        broadcastManager.subscribe(observer);
    }
    
    /**
     * Permite desuscribir un observador
     */
    public void unsubscribe(GameObserver observer) {
        broadcastManager.unsubscribe(observer);
    }
    
    /**
     * Retorna la instancia del BroadcastManager para acceso directo
     */
    public BroadcastManager getBroadcastManager() {
        return broadcastManager;
    }
}