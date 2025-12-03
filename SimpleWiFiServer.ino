#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h> // Necesario para hacer peticiones HTTP/HTTPS

// ⭐️ 1. CONFIGURACIÓN DE TELEGRAM Y WIFI ⭐️

// --- PARÁMETROS DE TELEGRAM (¡MODIFICA ESTOS!) ---
const char* BOT_TOKEN = "8587312904:AAE-4k1CM9cPDmDa2JOdi98Z0kO7VJydAQE"; // El token que te dio BotFather (ej. 123456:ABC-DEF1234ghIkl-7890)
const char* CHAT_ID   = "5746177016";   // El ID del chat o grupo (ej. -123456789)

// --- PARÁMETROS DE WIFI (¡MODIFICA ESTOS!) ---
const char *ssid = "S25 Ultra de Moises";
const char *password = "Moyoloco09."; 

// 2. Variables Globales y Servidor
WebServer server(80);
String encryptedResult = "";

// 3. Función de Envío a Telegram (Usando HTTPS)
void sendTelegramMessage(String message) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Error: WiFi no conectado. No se puede enviar el mensaje.");
        return;
    }

    HTTPClient http;
    
    // Codificar el mensaje para la URL: reemplazar espacios por '+'
    String encodedMessage = "";
    for (int i = 0; i < message.length(); i++) {
        if (message.charAt(i) == ' ') {
            encodedMessage += "+";
        } else {
            encodedMessage += message.charAt(i);
        }
    }
    
    // Construir la URL completa de la API
    String telegramUrl = "https://api.telegram.org/bot";
    telegramUrl += BOT_TOKEN;
    telegramUrl += "/sendMessage?chat_id=";
    telegramUrl += CHAT_ID;
    telegramUrl += "&text=El%20resultado%20Enigma%20es:%20"; // Mensaje fijo (URL encoded: "El resultado Enigma es: ")
    telegramUrl += encodedMessage;
    
    // Iniciar la conexión HTTPS
    http.begin(telegramUrl); 
    
    Serial.print("Intentando enviar mensaje a Telegram... ");
    int httpResponseCode = http.GET(); // Enviar la solicitud GET

    if (httpResponseCode == HTTP_CODE_OK) { // HTTP_CODE_OK es 200
        Serial.println("Mensaje enviado con éxito.");
    } else {
        Serial.print("Error en la solicitud HTTP. Código: ");
        Serial.println(httpResponseCode);
        Serial.println("Revisa el Token y el Chat ID.");
    }

    http.end(); // Liberar recursos de conexión
}


// 4. Contenido HTML Estático del Emulador Enigma
// Incluye el JavaScript modificado para enviar el resultado a /guardar_resultado
const char* HTML_ENIGMA = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Emulador Enigma I (1930)</title>
    <style>
        :root {
            --bg-color: #2c3e50;
            --panel-color: #34495e;
            --text-color: #ecf0f1;
            --accent-color: #e67e22;
            --button-hover: #d35400;
        }

        body {
            font-family: 'Courier New', Courier, monospace;
            background-color: var(--bg-color);
            color: var(--text-color);
            display: flex;
            justify-content: center;
            align-items: center;
            min-height: 100vh;
            margin: 0;
        }

        .container {
            background-color: var(--panel-color);
            padding: 2rem;
            border-radius: 10px;
            box-shadow: 0 10px 25px rgba(0,0,0,0.5);
            width: 90%;
            max-width: 600px;
            text-align: center;
            border: 2px solid #7f8c8d;
        }

        h1, h2 {
            border-bottom: 1px solid #7f8c8d;
            padding-bottom: 10px;
            margin-bottom: 20px;
        }

        .hidden {
            display: none;
        }

        .control-group {
            margin-bottom: 1.5rem;
        }

        label {
            display: block;
            margin-bottom: 0.5rem;
            font-weight: bold;
        }

        select, input[type="text"], button {
            padding: 10px;
            border-radius: 5px;
            border: none;
            font-family: inherit;
            font-size: 1rem;
        }

        select, input[type="text"] {
            width: 100%;
            box-sizing: border-box;
            margin-bottom: 10px;
        }

        button {
            background-color: var(--accent-color);
            color: white;
            cursor: pointer;
            width: 100%;
            font-weight: bold;
            transition: background 0.3s;
        }

        button:hover {
            background-color: var(--button-hover);
        }

        .rotor-selector-container {
            display: flex;
            gap: 10px;
            justify-content: center;
            flex-wrap: wrap;
        }

        .rotor-select-wrapper {
            flex: 1;
            min-width: 80px;
        }

        #outputLabel {
            background-color: #222;
            padding: 15px;
            border: 1px dashed #7f8c8d;
            margin-top: 20px;
            word-break: break-all;
            min-height: 24px;
            font-size: 1.2rem;
            color: #f1c40f; 
        }

        .error {
            color: #e74c3c;
            font-size: 0.9rem;
            margin-top: 5px;
            display: none;
        }
    </style>
</head>
<body>

    <div class="container">
        <div id="view-setup">
            <h1>Emulador Enigma I (1930)</h1>
            <p>Bienvenido al simulador.</p>
            
            <div class="control-group">
                <label for="numRotors">Seleccione número de rotores:</label>
                <select id="numRotors">
                    <option value="3">3 Rotores</option>
                    <option value="4">4 Rotores</option>
                    <option value="5">5 Rotores</option>
                </select>
            </div>

            <button onclick="goToMainView()">Continuar</button>
        </div>

        <div id="view-main" class="hidden">
            <h2>Configuración de Máquina</h2>
            
            <div id="rotorsContainer" class="control-group">
                <label>Seleccione Rotores (Izq a Der):</label>
                <div class="rotor-selector-container" id="rotorSelectors">
                    </div>
                <div id="rotorError" class="error">No puede repetir rotores.</div>
            </div>

            <div class="control-group">
                <label for="inputText">Texto a Encriptar:</label>
                <input type="text" id="inputText" placeholder="Ingrese texto (A-Z)..." oninput="this.value = this.value.toUpperCase()">
            </div>

            <button onclick="runEncryption()">ENCRIPTAR</button>

            <div class="control-group">
                <label>Resultado:</label>
                <div id="outputLabel"></div>
            </div>
            
            <button onclick="resetView()" style="background-color: #7f8c8d; margin-top: 10px;">Reiniciar Configuración</button>
        </div>
    </div>

    <script>
        // --- DATA DE LA MÁQUINA ENIGMA ---
        const alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        
        // Definición de Rotores (Wiring y Notch)
        const rotorsData = {
            'I':   { wiring: "EKMFLGDQVZNTOWYHXUSPAIBRCJ", notch: 'Q' },
            'II':  { wiring: "AJDKSIRUXBLHWTMCQGZNPYFVOE", notch: 'E' },
            'III': { wiring: "BDFHJLCPRTXVZNYEIWGAKMUSQO", notch: 'V' },
            'IV':  { wiring: "ESOVPZJAYQUIRHXLNFTGKDCMWB", notch: 'J' },
            'V':   { wiring: "VZBRGITYUPSDNHLXAWMJQOFEKC", notch: 'Z' }
        };

        // Reflector B (Hardcoded según requerimiento: YRUHQSLDPXNGOKMIEBFZCWVJAT)
        const reflectorWiring = "YRUHQSLDPXNGOKMIEBFZCWVJAT";

        let selectedRotorCount = 3;

        // --- LÓGICA DE UI ---

        function goToMainView() {
            const select = document.getElementById('numRotors');
            selectedRotorCount = parseInt(select.value);
            
            generateRotorSelectors(selectedRotorCount);

            document.getElementById('view-setup').classList.add('hidden');
            document.getElementById('view-main').classList.remove('hidden');
        }

        function resetView() {
            document.getElementById('inputText').value = "";
            document.getElementById('outputLabel').innerText = "";
            document.getElementById('view-main').classList.add('hidden');
            document.getElementById('view-setup').classList.remove('hidden');
        }

        function generateRotorSelectors(count) {
            const container = document.getElementById('rotorSelectors');
            container.innerHTML = '';
            
            const romanNumerals = ['I', 'II', 'III', 'IV', 'V'];

            for (let i = 0; i < count; i++) {
                const wrapper = document.createElement('div');
                wrapper.className = 'rotor-select-wrapper';
                
                const select = document.createElement('select');
                select.className = 'rotor-choice';
                select.id = `rotor-${i}`;
                
                romanNumerals.forEach(roman => {
                    const option = document.createElement('option');
                    option.value = roman;
                    option.text = `Rotor ${roman}`;
                    // Preseleccionar diferentes para evitar colisiones por defecto
                    if (romanNumerals.indexOf(roman) === i) option.selected = true;
                    select.appendChild(option);
                });

                wrapper.appendChild(select);
                container.appendChild(wrapper);
            }
        }

        // --- LÓGICA ENIGMA ---

        // Función auxiliar: Convertir Letra a Índice (0-25)
        function l2i(letter) {
            return letter.charCodeAt(0) - 65;
        }

        // Función auxiliar: Convertir Índice a Letra
        function i2l(index) {
            // Asegurar que el índice sea positivo (módulo correcto)
            const n = 26;
            return String.fromCharCode(((index % n) + n) % n + 65);
        }

        function runEncryption() {
            const input = document.getElementById('inputText').value;
            const errorDiv = document.getElementById('rotorError');
            const outputDiv = document.getElementById('outputLabel');
            
            // 1. Obtener rotores seleccionados
            const selects = document.querySelectorAll('.rotor-choice');
            let selectedRotorsKeys = [];
            selects.forEach(s => selectedRotorsKeys.push(s.value));

            // 2. Validar duplicados
            const unique = new Set(selectedRotorsKeys);
            if (unique.size !== selectedRotorsKeys.length) {
                errorDiv.style.display = 'block';
                outputDiv.innerText = "ERROR";
                return;
            } else {
                errorDiv.style.display = 'none';
            }

            // 3. Preparar estado de los rotores
            let activeRotors = selectedRotorsKeys.map(key => {
                return {
                    wiring: rotorsData[key].wiring,
                    notch: l2i(rotorsData[key].notch),
                    position: 0 // Posición inicial 'A'
                };
            });

            activeRotors.reverse();

            let result = "";

            // 4. Procesar caracter por caracter
            for (let i = 0; i < input.length; i++) {
                let char = input[i];
                if (!/[A-Z]/.test(char)) {
                    continue; // Ignorar espacios o símbolos
                }

                // A. AVANZAR ROTORES (Stepping)
                stepRotors(activeRotors);

                // B. FLUJO DE SEÑAL
                let signal = l2i(char);

                // 1. Pasada de Ida (Derecha a Izquierda) -> Rotores
                for (let r = 0; r < activeRotors.length; r++) {
                    signal = passThroughRotor(signal, activeRotors[r], false);
                }

                // 2. Reflector
                signal = passThroughReflector(signal, reflectorWiring);

                // 3. Pasada de Vuelta (Izquierda a Derecha) -> Rotores Inversos
                for (let r = activeRotors.length - 1; r >= 0; r--) {
                    signal = passThroughRotor(signal, activeRotors[r], true);
                }

                result += i2l(signal);
            }

            outputDiv.innerText = result;

            // ⭐ LÓGICA AGREGADA: Enviar resultado al ESP32 ⭐
            sendResultToESP32(result);
        }
        
        // ⭐ FUNCIÓN JS PARA ENVIAR EL RESULTADO AL SERVIDOR ⭐
        function sendResultToESP32(data) {
            var encodedData = encodeURIComponent(data);
            var url = '/guardar_resultado?data=' + encodedData;

            // Enviamos la petición en segundo plano (AJAX)
            fetch(url)
                .then(response => {
                    if (response.status === 200) {
                        console.log("Resultado enviado y guardado en ESP32.");
                    } else {
                        console.error("Error al guardar el dato en ESP32.");
                    }
                })
                .catch(error => console.error('Error de red:', error));
        }


        function stepRotors(rotors) {
            let rotateFlags = new Array(rotors.length).fill(false);
            rotateFlags[0] = true;

            for (let i = 0; i < rotors.length - 1; i++) {
                if (rotors[i].position === rotors[i].notch) {
                    if (rotateFlags[i]) {
                        rotateFlags[i+1] = true;
                    }
                }
            }

            for (let i = 0; i < rotors.length; i++) {
                if (rotateFlags[i]) {
                    rotors[i].position = (rotors[i].position + 1) % 26;
                }
            }
        }

        function passThroughRotor(inputIndex, rotor, isInverse) {
            const offset = rotor.position;
            
            let indexIn = (inputIndex + offset) % 26;
            
            let indexOut;
            if (!isInverse) {
                const charMap = rotor.wiring[indexIn];
                indexOut = l2i(charMap);
            } else {
                const charToFind = i2l(indexIn);
                indexOut = rotor.wiring.indexOf(charToFind);
            }

            return (indexOut - offset + 26) % 26;
        }

        function passThroughReflector(inputIndex, wiring) {
            const charMap = wiring[inputIndex];
            return l2i(charMap);
        }

    </script>
</body>
</html>
)rawliteral";


// --- 5. Funciones de Manejo de Rutas ---

// Función para servir la página HTML
void handleRoot() {
  server.send(200, "text/html", HTML_ENIGMA);
}

// FUNCIÓN DE MANEJO PARA GUARDAR Y ENVIAR EL RESULTADO
void handleSaveResult() {
  if (server.hasArg("data")) {
    encryptedResult = server.arg("data"); 
    
    // ⭐ ENVÍA EL RESULTADO A TELEGRAM EN LUGAR DEL SERIAL
    sendTelegramMessage(encryptedResult);
    
    server.send(200, "text/plain", "OK. Dato guardado y enviado a Telegram.");
  } else {
    server.send(400, "text/plain", "Error: No se proporcionó el parámetro 'data'.");
  }
}

// Función para manejar cualquier ruta no definida (Error 404)
void handleNotFound() {
  server.send(404, "text/plain", "404: Not found");
}


// --- 6. Setup y Loop ---

void setup() {
  Serial.begin(9600); 
  
  // 1. Conexión Wi-Fi
  Serial.print("Conectando a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi conectado.");
  Serial.print("Direccion IP del Servidor: ");
  Serial.println(WiFi.localIP());
  
  // 2. Definición de Rutas del Servidor
  server.on("/", HTTP_GET, handleRoot);            
  server.on("/guardar_resultado", HTTP_GET, handleSaveResult); 
  server.onNotFound(handleNotFound);        

  // 3. Inicio del Servidor
  server.begin();
  Serial.println("Servidor HTTP iniciado.");
}

void loop() {
  server.handleClient();
}