package GameServer.DonkeyKong.Game.model;

public class Fruit extends Entity {
    private int vineId;
    private int points;
    
    public Fruit(int id, int vineId, float y, int points) {
        super(id, calculateX(vineId), y);  // X se calcula según la liana
        this.vineId = vineId;
        this.points = points;
    }
    
    private static float calculateX(int vineId) {
        // Mapeo de liana -> posición X (igual que en fruta.c)
        switch (vineId) {
            case 1: return 75;
            case 2: return 200;
            case 3: return 300;
            case 4: return 490;
            case 5: return 650;
            case 6: return 750;
            case 7: return 850;
            case 8: return 950;
            case 9: return 1075;
            default: return 75;
        }
    }
    
    @Override
    public void update(double delta) {
        // Las frutas no se mueven, solo existen
    }
    
    @Override
    public String serialize() {
        // Formato: id,vine,x,y,points
        return String.format("%d,%d,%.0f,%.0f,%d", id, vineId, x, y, points);
    }
    
    public int getVineId() { return vineId; }
    public int getPoints() { return points; }
}