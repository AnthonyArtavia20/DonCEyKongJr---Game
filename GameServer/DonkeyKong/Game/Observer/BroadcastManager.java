package GameServer.DonkeyKong.Game.Observer;

import java.util.concurrent.CopyOnWriteArrayList;
import java.util.concurrent.Executor;
import java.util.concurrent.Executors;

/**
 * Subject del patrón Observer - Gestiona los observadores y notifica eventos
 * Es thread-safe para uso en entornos concurrentes
 * Cada sala tiene su propia instancia (no es más Singleton)
 * 
 * @author Anthony Artavia & Ariel Saborio
 */
public class BroadcastManager {
    
    private static final BroadcastManager globalInstance = new BroadcastManager();
    
    // Lista thread-safe de observadores
    private final CopyOnWriteArrayList<GameObserver> observers;
    
    // Thread pool para notificaciones asincrónicas (opcional, para no bloquear)
    private final Executor executor;
    
    // Bandera para loguear eventos
    private boolean verbose = true;
    
    /**
     * Constructor privado - para instancia global
     */
    private BroadcastManager() {
        this.observers = new CopyOnWriteArrayList<>();
        this.executor = Executors.newFixedThreadPool(4); // Pool de 4 threads
    }
    
    /**
     * Constructor público - para crear instancias per-sala
     */
    public BroadcastManager(boolean isPublic) {
        this.observers = new CopyOnWriteArrayList<>();
        this.executor = Executors.newFixedThreadPool(4); // Pool de 4 threads
    }
    
    /**
     * Obtiene la instancia global del BroadcastManager (Singleton global - deprecated)
     * Se mantiene por compatibilidad pero no se recomienda usar
     */
    public static BroadcastManager getInstance() {
        return globalInstance;
    }
    
    /**
     * Registra un observador para recibir notificaciones
     * 
     * @param observer El observador a registrar
     */
    public void subscribe(GameObserver observer) {
        if (observer != null && !observers.contains(observer)) {
            observers.add(observer);
            if (verbose) {
                System.out.println("[BroadcastManager] Observador registrado: " + observer.getClass().getSimpleName());
            }
        }
    }
    
    /**
     * Desregistra un observador
     * 
     * @param observer El observador a remover
     */
    public void unsubscribe(GameObserver observer) {
        if (observer != null && observers.remove(observer)) {
            if (verbose) {
                System.out.println("[BroadcastManager] Observador removido: " + observer.getClass().getSimpleName());
            }
        }
    }
    
    /**
     * Notifica a todos los observadores de un evento (SÍNCRONO)
     * Los observadores recibirán la notificación inmediatamente
     * 
     * @param event El evento a notificar
     */
    public void notify(GameEvent event) {
        if (verbose) {
            System.out.println("[BroadcastManager] Notificando evento: " + event.getType() + 
                             " a " + observers.size() + " observadores");
        }
        
        for (GameObserver observer : observers) {
            try {
                observer.onGameEvent(event);
            } catch (Exception e) {
                System.err.println("[BroadcastManager] Error notificando a observador: " + e.getMessage());
                e.printStackTrace();
            }
        }
    }
    
    /**
     * Notifica a todos los observadores de un evento (ASINCRÓNICO)
     * Las notificaciones se ejecutan en un thread pool separado
     * 
     * @param event El evento a notificar
     */
    public void notifyAsync(GameEvent event) {
        executor.execute(() -> notify(event));
    }
    
    /**
     * Notifica a observadores específicos
     * 
     * @param event El evento
     * @param observerClass La clase de observador a notificar
     */
    public void notifySpecific(GameEvent event, Class<? extends GameObserver> observerClass) {
        for (GameObserver observer : observers) {
            if (observerClass.isInstance(observer)) {
                try {
                    observer.onGameEvent(event);
                } catch (Exception e) {
                    System.err.println("[BroadcastManager] Error notificando observador específico: " + e.getMessage());
                }
            }
        }
    }
    
    /**
     * Retorna el número de observadores registrados
     */
    public int getObserverCount() {
        return observers.size();
    }
    
    /**
     * Limpia todos los observadores
     */
    public void clearObservers() {
        observers.clear();
        if (verbose) {
            System.out.println("[BroadcastManager] Todos los observadores removidos");
        }
    }
    
    /**
     * Habilita/deshabilita logs verbosos
     */
    public void setVerbose(boolean verbose) {
        this.verbose = verbose;
    }
}
