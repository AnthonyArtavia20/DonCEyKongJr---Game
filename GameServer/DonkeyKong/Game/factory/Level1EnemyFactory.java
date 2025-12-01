package GameServer.DonkeyKong.Game.factory;

import GameServer.DonkeyKong.Game.model.*;

public class Level1EnemyFactory implements EnemyFactory {
    
    @Override
    public BlueCrocodile createBlueCrocodile(int id, int vineId, float x, float y) {
        // Nivel 1: velocidad normal
        return new BlueCrocodile(id, x, y, vineId, 2.0f);
    }
    
    @Override
    public RedCrocodile createRedCrocodile(int id, int vineId, float x, float y) {
        // Nivel 1: velocidad lenta
        return new RedCrocodile(id, x, y, vineId, 1.5f);
    }
    
    @Override
    public Fruit createFruit(int id, int vineId, float y) {
        // Nivel 1: 100 puntos
        return new Fruit(id, vineId, y, 100);
    }
    
    @Override
    public int getLevel() {
        return 1;
    }
}