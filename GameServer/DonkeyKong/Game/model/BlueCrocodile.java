package GameServer.DonkeyKong.Game.model;

public class BlueCrocodile extends Enemy {
    
    public BlueCrocodile(int id, float x, float y, int vineId, float speed) {
        super(id, x, y, vineId, speed, "BLUE");
    }
    
    @Override
    public void updateBehavior(double delta) {
        // COCODRILO AZUL: Solo baja por la liana
        y += speed * direction;
        
        // Si llega abajo, desactivar (el cliente ya tiene la lógica de límites)
        if (y > 800) {  // ALTO_PANTALLA
            active = false;
        }
    }
}