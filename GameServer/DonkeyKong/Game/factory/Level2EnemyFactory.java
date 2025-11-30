package GameServer.DonkeyKong.Game.factory;

import GameServer.DonkeyKong.Game.model.*;

public class Level2EnemyFactory implements EnemyFactory {
    
    @Override
    public BlueCrocodile createBlueCrocodile(int id, int vineId, float x, float y) {
        // Nivel 2: más rápido
        return new BlueCrocodile(id, x, y, vineId, 2.5f);
    }
    
    @Override
    public RedCrocodile createRedCrocodile(int id, int vineId, float x, float y) {
        // Nivel 2: más rápido
        return new RedCrocodile(id, x, y, vineId, 2.0f);
    }
    
    @Override
    public Fruit createFruit(int id, int vineId, float y) {
        // Nivel 2: 200 puntos
        return new Fruit(id, vineId, y, 200);
    }
    
    @Override
    public int getLevel() {
        return 2;
    }
}