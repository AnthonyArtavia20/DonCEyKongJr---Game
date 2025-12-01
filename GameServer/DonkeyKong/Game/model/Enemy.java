package GameServer.DonkeyKong.Game.model;

public abstract class Enemy extends Entity {
    protected int vineId;      // En qué liana está
    protected float speed;      // Velocidad base
    protected int direction;    // 1 = abajo, -1 = arriba
    protected String type;      // "BLUE" o "RED"
    
    public Enemy(int id, float x, float y, int vineId, float speed, String type) {
        super(id, x, y);
        this.vineId = vineId;
        this.speed = speed;
        this.direction = 1;
        this.type = type;
    }
    
    // Cada tipo de enemigo implementa su propio comportamiento
    public abstract void updateBehavior(double delta);
    
    @Override
    public void update(double delta) {
        if (!active) return;
        updateBehavior(delta);
    }
    
    @Override
    public String serialize() {
        // Formato: id,tipo,x,y,vine
        return String.format("%d,%s,%.0f,%.0f,%d", id, type, x, y, vineId);
    }
    
    // Getters
    public int getVineId() { return vineId; }
    public String getType() { return type; }
}