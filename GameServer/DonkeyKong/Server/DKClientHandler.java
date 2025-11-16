package GameServer.DonkeyKong.Server;

import GameServer.CoreGenericServer.ClientHandler;
import GameServer.CoreGenericServer.GameServer;
import java.io.IOException;
import java.net.Socket;

/**
 * Handler específico para clientes de Donkey Kong Jr
 * Soporta 3 tipos: PLAYER, SPECTATOR, ADMIN
 * 
 * @author Anthony Artavia
 */
public class DKClientHandler extends ClientHandler {
    
    public enum ClientType {
        UNKNOWN,    // Recién conectado, aún no identificado
        PLAYER,     // Jugador activo (máximo 2)
        SPECTATOR,  // Espectador (máximo 2 por jugador)
        ADMIN       // Administrador (sin límite)
    }
    
    private ClientType clientType;
    private Integer playerId;        // Solo para PLAYER
    private String playerName;
    private int lives;
    private int score;
    
    public DKClientHandler(Socket socket, GameServer server) throws IOException {
        super(socket, server);
        this.clientType = ClientType.UNKNOWN;
        this.playerId = null;
        this.lives = 3;
        this.score = 0;
    }
    
    // Getters y Setters
    public ClientType getClientType() {
        return clientType;
    }
    
    public void setClientType(ClientType type) {
        this.clientType = type;
    }
    
    public boolean isPlayer() {
        return clientType == ClientType.PLAYER;
    }
    
    public boolean isSpectator() {
        return clientType == ClientType.SPECTATOR;
    }
    
    public boolean isAdmin() {
        return clientType == ClientType.ADMIN;
    }
    
    public Integer getPlayerId() {
        return playerId;
    }
    
    public void setPlayerId(Integer id) {
        this.playerId = id;
        this.clientType = ClientType.PLAYER;
    }
    
    public String getPlayerName() {
        return playerName;
    }
    
    public void setPlayerName(String name) {
        this.playerName = name;
    }
    
    public int getLives() {
        return lives;
    }
    
    public void setLives(int lives) {
        this.lives = lives;
    }
    
    public void loseLife() {
        this.lives = Math.max(0, this.lives - 1);
    }
    
    public int getScore() {
        return score;
    }
    
    public void addScore(int points) {
        this.score += points;
    }
    
    public void setScore(int score) {
        this.score = score;
    }
    
    public boolean isAlive() {
        return lives > 0;
    }
    
    @Override
    public String toString() {
        return String.format("DKClient[id=%s, type=%s, playerId=%s, name=%s, lives=%d, score=%d]",
            clientId, clientType, playerId, playerName, lives, score);
    }
}