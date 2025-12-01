package GameServer.DonkeyKong.Game.factory;

import GameServer.DonkeyKong.Game.model.*;

public interface EnemyFactory {
    
    /**
     * Crea un cocodrilo azul con características según el nivel
     */
    BlueCrocodile createBlueCrocodile(int id, int vineId, float x, float y);
    
    /**
     * Crea un cocodrilo rojo con características según el nivel
     */
    RedCrocodile createRedCrocodile(int id, int vineId, float x, float y);
    
    /**
     * Crea una fruta con puntos según el nivel
     */
    Fruit createFruit(int id, int vineId, float y);
    
    /**
     * Retorna el nivel de dificultad de esta factory
     */
    int getLevel();
}