package GameServer.DonkeyKong.Game.model;

public abstract class Entity {
    protected int id;
    protected float x;
    protected float y;
    protected boolean active;
    
    public Entity(int id, float x, float y) {
        this.id = id;
        this.x = x;
        this.y = y;
        this.active = true;
    }
    
    // Método abstracto que cada entidad implementa
    public abstract void update(double delta);
    
    // Serialización para enviar al cliente
    public abstract String serialize();
    
    // Getters y setters básicos
    public int getId() { return id; }
    public float getX() { return x; }
    public float getY() { return y; }
    public boolean isActive() { return active; }
    public void setActive(boolean active) { this.active = active; }
}