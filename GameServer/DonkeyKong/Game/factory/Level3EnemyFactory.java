package GameServer.DonkeyKong.Game.factory;

import GameServer.DonkeyKong.Game.model.*;

public class Level3EnemyFactory implements EnemyFactory {
    
    @Override
    public BlueCrocodile createBlueCrocodile(int id, int vineId, float x, float y) {
        // Nivel 3: muy rápido
        return new BlueCrocodile(id, x, y, vineId, 3.5f);
    }
    
    @Override
    public RedCrocodile createRedCrocodile(int id, int vineId, float x, float y) {
        // Nivel 3: muy rápido
        return new RedCrocodile(id, x, y, vineId, 3.0f);
    }
    
    @Override
    public Fruit createFruit(int id, int vineId, float y) {
        // Nivel 3: 300 puntos
        return new Fruit(id, vineId, y, 300);
    }
    
    @Override
    public int getLevel() {
        return 3;
    }
}