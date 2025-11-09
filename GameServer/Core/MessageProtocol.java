package GameServer.Core;

/**
 * Protocolo mejorado que soporta múltiples parámetros
 * Formato: COMMAND|PARAM1|PARAM2|PARAM3|...
 * 
 * @author Anthony Artavia
 * @version 2.0
 */

public class MessageProtocol {
    public static final String SEP = "|";

    public static final class Type {
        //Mensajes genéricos del servidor
        public static final String JOIN = "JOIN";
        public static final String MOVE = "MOVE";
        public static final String CHAT = "CHAT";
        public static final String GAME_STATE = "GAME_STATE";
        public static final String PING = "PING";
        public static final String PONG = "PONG";

        //Mensajes específicos para el proyecto
        public static final String CONNECT = "CONNECT";
        public static final String DISCONNECT = "DISCONNECT";
        public static final String ADMIN = "ADMIN";
        public static final String EVENT = "EVENT";
        public static final String ERROR = "ERROR";
        public static final String OK = "OK";
    }

    /**
     * Codifica un mensaje con múltiples parámetros
     * @param parts Comando y parámetros
     * @return String codificado
     */
    public static String encode(String... parts) {
        if (parts == null || parts.length == 0) {
            return "";
        }
        return String.join(SEP, parts);
    }

    /**
     * Decodifica un mensaje en sus partes
     * @param raw Mensaje crudo
     * @return Message con comando y parámetros
     */
    public static Message decode(String raw) {
        if (raw == null || raw.trim().isEmpty()) {
            return new Message("", new String[0]);
        }
        
        String[] parts = raw.split("\\" + SEP, -1); // -1 para incluir strings vacíos
        if (parts.length == 0) {
            return new Message("", new String[0]);
        }
        
        String command = parts[0];
        String[] params = new String[parts.length - 1];
        System.arraycopy(parts, 1, params, 0, params.length);
        
        return new Message(command, params);
    }

    /**
     * Validación básica
     */
    public static boolean isValid(String raw) {
        return raw != null && !raw.trim().isEmpty();
    }

    /**
     * Clase Message mejorada con múltiples parámetros
     */
    public static class Message {
        public final String command;
        public final String[] params;
        
        public Message(String command, String[] params) {
            this.command = command == null ? "" : command;
            this.params = params == null ? new String[0] : params;
        }
        
        // Compatibilidad con versión anterior (type + payload)
        @Deprecated
        public String getType() {
            return command;
        }
        
        @Deprecated
        public String getPayload() {
            return params.length > 0 ? params[0] : "";
        }
        
        /**
         * Obtiene un parámetro por índice (seguro)
         */
        public String getParam(int index) {
            if (index < 0 || index >= params.length) {
                return "";
            }
            return params[index];
        }
        
        /**
         * Obtiene un parámetro como entero
         */
        public int getParamAsInt(int index, int defaultValue) {
            try {
                return Integer.parseInt(getParam(index));
            } catch (NumberFormatException e) {
                return defaultValue;
            }
        }
        
        /**
         * Obtiene un parámetro como float
         */
        public float getParamAsFloat(int index, float defaultValue) {
            try {
                return Float.parseFloat(getParam(index));
            } catch (NumberFormatException e) {
                return defaultValue;
            }
        }
        
        /**
         * Número de parámetros
         */
        public int getParamCount() {
            return params.length;
        }
        
        /**
         * Verifica si tiene al menos N parámetros
         */
        public boolean hasParams(int count) {
            return params.length >= count;
        }
        
        @Override
        public String toString() {
            StringBuilder sb = new StringBuilder(command);
            for (String param : params) {
                sb.append(SEP).append(param);
            }
            return sb.toString();
        }
    }
}