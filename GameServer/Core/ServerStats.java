package GameServer.Core;

import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicLong;

/**
 * Estadísticas simples y thread-safe del servidor.
 */
public class ServerStats {
    private final AtomicInteger connectedClients = new AtomicInteger(0);
    private final AtomicLong totalConnections = new AtomicLong(0);
    private final AtomicLong messagesReceived = new AtomicLong(0);
    private final AtomicLong messagesSent = new AtomicLong(0);

    public void onNewConnection() {
        connectedClients.incrementAndGet();
        totalConnections.incrementAndGet();
    }

    public void onDisconnect() {
        connectedClients.updateAndGet(v -> Math.max(0, v - 1));
    }

    public void onMessageReceived() {
        messagesReceived.incrementAndGet();
    }

    public void onMessageSent() {
        messagesSent.incrementAndGet();
    }

    public int getConnectedClients() {
        return connectedClients.get();
    }

    public long getTotalConnections() {
        return totalConnections.get();
    }

    public long getMessagesReceived() {
        return messagesReceived.get();
    }

    public long getMessagesSent() {
        return messagesSent.get();
    }

    @Override
    public String toString() {
        return "ServerStats{" +
                "connectedClients=" + connectedClients +
                ", totalConnections=" + totalConnections +
                ", messagesReceived=" + messagesReceived +
                ", messagesSent=" + messagesSent +
                '}';
    }
}
