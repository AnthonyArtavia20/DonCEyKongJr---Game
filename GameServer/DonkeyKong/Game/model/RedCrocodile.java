package GameServer.DonkeyKong.Game.model;

public class RedCrocodile extends Enemy {
    private float minY;
    private float maxY;
    
    public RedCrocodile(int id, float x, float y, int vineId, float speed) {
        super(id, x, y, vineId, speed, "RED");
        // El cocodrilo rojo necesita conocer los límites de su liana
        // Esto se puede setear después de crear
        this.minY = 0;
        this.maxY = 800;
    }
    
    public void setVineLimits(float minY, float maxY) {
        this.minY = minY;
        this.maxY = maxY;
        // Posicionar en el centro
        this.y = (minY + maxY) / 2;
    }
    
    @Override
    public void updateBehavior(double delta) {
        // COCODRILO ROJO: Sube y baja en la liana
        y += speed * direction;
        
        // Cambiar dirección en los límites
        if (y <= minY) {
            y = minY;
            direction = 1;  // Bajar
        } else if (y >= maxY) {
            y = maxY;
            direction = -1; // Subir
        }
    }
}