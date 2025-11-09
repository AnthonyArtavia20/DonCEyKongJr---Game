package GameServer.Core;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;
import java.util.UUID;

public class ClientHandler implements Runnable {
    protected Socket socket;
    protected BufferedReader in;
    protected PrintWriter out;
    protected String clientId;
    protected volatile boolean connected;
    protected GameServer server;

    public ClientHandler(Socket socket, GameServer server) throws IOException {
        this.socket = socket;
        this.server = server;
        this.in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
        this.out = new PrintWriter(socket.getOutputStream(), true);
        this.clientId = UUID.randomUUID().toString();
        this.connected = true;
    }

    // Enviar un mensaje al cliente
    public void send(String message) {
        if (!connected) return;
        out.println(message);
    }

    // Cerrar la conexión y recursos
    public void close() {
        connected = false;
        try {
            if (in != null) in.close();
        } catch (IOException ignored) {}
        try {
            if (out != null) out.close();
        } catch (Exception ignored) {}
        try {
            if (socket != null && !socket.isClosed()) socket.close();
        } catch (IOException ignored) {}
    }

    // Loop de lectura de mensajes
    @Override
    public void run() {
        String line;
        try {
            while (connected && (line = in.readLine()) != null) {
                onMessageReceived(line);
            }
        } catch (IOException e) {
            // conexión perdida
        } finally {
            connected = false;
            try {
                server.disconnectClient(this);
            } catch (Exception ignored) {}
            close();
        }
    }

    // Manejo por defecto: reenviar al servidor. Se puede sobrescribir.
    protected void onMessageReceived(String message) {
        if (server != null) {
            server.dispatchMessage(this, message);
        }
    }
}
