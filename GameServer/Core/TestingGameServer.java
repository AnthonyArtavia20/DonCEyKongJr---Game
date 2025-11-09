package GameServer.Core;

import java.io.IOException;
import java.util.Scanner;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * Demo implementation of GameServer for testing and as a template.
 */
public class TestingGameServer extends GameServer {
    private final AtomicInteger tickCount = new AtomicInteger(0);

    public TestingGameServer(ServerConfig config) {
        super(config);
    }

    public TestingGameServer(int port, int maxClients) {
        super(port, maxClients);
    }

    @Override
    protected ClientHandler createClient(java.net.Socket socket) {
        try {
            return new ClientHandler(socket, this);
        } catch (IOException e) {
            System.err.println("[Demo] Error creando ClientHandler: " + e.getMessage());
            return null;
        }
    }

    @Override
    protected void update(double delta) {
        int t = tickCount.incrementAndGet();
        // Broadcast game state roughly once per second (assuming tickRate ticks/sec)
        if (config != null && config.getTickRate() > 0) {
            if (t % config.getTickRate() == 0) {
                broadcast(getGameState());
            }
        }
    }

    @Override
    protected String getGameState() {
        return MessageProtocol.encode(MessageProtocol.Type.GAME_STATE, "tick=" + tickCount.get());
    }

    @Override
    protected void handleMessage(ClientHandler client, String msg) {
        MessageProtocol.Message m = MessageProtocol.decode(msg);
        switch (m.type) {
            case MessageProtocol.Type.JOIN:
                // Simple accept: reply with ACCEPT|<clientId>
                ClientHandler ch = client;
                String id = (ch != null) ? ch.clientId : "";
                if (ch != null) ch.send(MessageProtocol.encode("ACCEPT", id));
                break;
            case MessageProtocol.Type.CHAT:
                // Broadcast chat with client id prefix
                String prefix = (client != null) ? client.clientId + ": " : "";
                broadcast(MessageProtocol.encode(MessageProtocol.Type.CHAT, prefix + m.payload));
                break;
            case MessageProtocol.Type.PING:
                sendTo(client, MessageProtocol.encode(MessageProtocol.Type.PONG, ""));
                break;
            default:
                // Unknown message: echo back
                sendTo(client, MessageProtocol.encode("ECHO", m.payload));
                break;
        }
    }

    // Simple CLI to run demo server
    public static void main(String[] args) {
        int port = 12345;
        int maxClients = 4;
        if (args.length >= 1) {
            try { port = Integer.parseInt(args[0]); } catch (NumberFormatException ignored) {}
        }
        if (args.length >= 2) {
            try { maxClients = Integer.parseInt(args[1]); } catch (NumberFormatException ignored) {}
        }

        ServerConfig cfg = new ServerConfig();
        cfg.setPort(port);
        cfg.setMaxPlayers(Math.max(1, maxClients));

        TestingGameServer server = new TestingGameServer(cfg);
        server.start();

        System.out.println("DemoGameServer iniciado en puerto " + port + ". Escribe 'quit' para detener.");
        try (Scanner sc = new Scanner(System.in)) {
            while (sc.hasNextLine()) {
                String line = sc.nextLine().trim().toLowerCase();
                if (line.equals("quit") || line.equals("stop") || line.equals("exit")) {
                    break;
                }
                if (line.equals("stats")) {
                    System.out.println(server.stats.toString());
                }
            }
        }

        System.out.println("Deteniendo servidor...");
        server.stop();
    }
}
