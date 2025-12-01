package GameServer.DonkeyKong.Game.Observer;

/**
 * Interfaz Observer - Define el contrato que deben cumplir los observadores
 * Los observadores serán notificados cuando ocurran eventos en el juego
 * 
 * @author Anthony Artavia & Ariel Saborio
 */
public interface GameObserver {
    
    /**
     * Método que será llamado cuando ocurra un evento del juego
     * 
     * @param event El evento que ocurrió
     */
    void onGameEvent(GameEvent event);
}
