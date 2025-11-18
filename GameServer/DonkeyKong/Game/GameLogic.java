package GameServer.DonkeyKong.Game;

public class GameLogic {
    
    // Stub mínimo para que compile
    public void update(double delta) {
        // TODO: Ariel implementa
    }
    
    public String serialize() {
        return "GAMESTATE|0|PLAYERS||CROCS||FRUITS|";
    }
    
    public void addPlayer(int id, String name) {
        System.out.println("[GameLogic] Jugador agregado: " + name);
    }
    
    public void removePlayer(int id) {
        System.out.println("[GameLogic] Jugador removido: " + id);
    }
    
    public void movePlayer(int id, String direction) {
        System.out.println("[GameLogic] Jugador " + id + " se mueve " + direction);
    }
    
    public void createRedCrocodile(int vine, float speed) {
        System.out.println("[GameLogic] Cocodrilo rojo creado");
    }
    
    public void createBlueCrocodile(int vine, float speed) {
        System.out.println("[GameLogic] Cocodrilo azul creado");
    }
    
    public void createFruit(int vine, int height, int points) {
        System.out.println("[GameLogic] Fruta creada");
    }
    
    public boolean deleteFruit(int vine, int height) {
        System.out.println("[GameLogic] Fruta eliminada");
        return true;
    }
}