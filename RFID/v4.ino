#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <HTTPClient.h>

// Definición de pines
#define RST_PIN 22
#define SS_PIN 21
#define MISO_PIN 19
#define MOSI_PIN 23
#define SCK_PIN 18

// Configuración de Wi-Fi
const char* ssid = "HITRON-2DE0";
const char* password = "RicardoArjona@8090";

// URL del servidor
const char* serverUrl = "http://192.168.0.11/Tele/index.php";

unsigned long previousMillis = 0;  // Almacena el último tiempo que se ejecutó
const long interval = 5000;        // Intervalo de 5 segundos
MFRC522 rfid(SS_PIN, RST_PIN);     // Se cambió de mfrc522 a rfid para ser consistente
MFRC522::MIFARE_Key key; 

// Arrays para almacenar el estado de hasta 10 tarjetas
const int maxTarjetas = 10;
String tarjetasID[maxTarjetas];  // Array para almacenar los UIDs
int tarjetasEstado[maxTarjetas]; // Array para almacenar el estado de cada tarjeta

void setup() {
  Serial.begin(115200);
  Serial.println("Conexión con el servidor");

  // Iniciar el lector RFID
  SPI.begin();
  rfid.PCD_Init();
  Serial.println("Lector RFID iniciado.");

  // Inicializar clave
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  // Inicializar el array de estados en -1 (sin estado)
  for (int i = 0; i < maxTarjetas; i++) {
    tarjetasEstado[i] = -1;  // -1 significa que la tarjeta no ha sido registrada
  }

  Serial.println(F("Este código escanea el NUID de una tarjeta MIFARE Clásica."));
  Serial.print(F("Usando la clave: "));
  printHex(key.keyByte, MFRC522::MF_KEY_SIZE);

  // Conectar a la red Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Conectando a la red Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConexión exitosa!");
  Serial.print("IP Local: ");
  Serial.println(WiFi.localIP());
}

void loop() {
   // Buscar nuevas tarjetas
  if (!rfid.PICC_IsNewCardPresent())
    return;

  // Verificar si la tarjeta ha sido leída
  if (!rfid.PICC_ReadCardSerial())
    return;

  Serial.print(F("Tipo de tarjeta: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  // Comprobar si la tarjeta es del tipo MIFARE Classic
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
      piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
      piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("La etiqueta no es de tipo MIFARE Classic."));
    return;
  }

  Serial.println(F("Tarjeta detectada."));

  // Convertir el UID de la tarjeta a String
  String cardID = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    cardID += String(rfid.uid.uidByte[i], HEX);
  }

  Serial.println(F("El NUID de la tarjeta es:"));
  Serial.print(F("En hexadecimal: "));
  printHex(rfid.uid.uidByte, rfid.uid.size);
  Serial.println();
  Serial.print(F("En decimal: "));
  printDec(rfid.uid.uidByte, rfid.uid.size);
  Serial.println();

  // Verifica si ha pasado el intervalo de tiempo
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;  // Guardar el tiempo actual

    // Buscar la tarjeta en el array
    int index = buscarTarjeta(cardID);

    if (index == -1) {
      // Si la tarjeta no está registrada, registrar el ingreso
      index = registrarTarjeta(cardID);
      tarjetasEstado[index] = 1;  // Primer paso, es un ingreso
    } else {
      // Si la tarjeta ya está registrada, alternar el estado
      tarjetasEstado[index] = tarjetasEstado[index] == 1 ? 0 : 1;
    }

    // Enviar el estado actual de ingreso o salida al servidor
    sendData(cardID, tarjetasEstado[index]);
  }

  // Detener la tarjeta PICC
  rfid.PICC_HaltA();

  // Detener cifrado en el lector
  rfid.PCD_StopCrypto1();
}

/**
 * Función auxiliar para mostrar un array de bytes en hexadecimal
 */
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

/**
 * Función auxiliar para mostrar un array de bytes en decimal
 */
void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}

// Función para buscar una tarjeta en el array de tarjetas registradas
int buscarTarjeta(String cardID) {
  for (int i = 0; i < maxTarjetas; i++) {
    if (tarjetasID[i] == cardID) {
      return i;  // Retorna el índice donde está la tarjeta
    }
  }
  return -1;  // Retorna -1 si no se encontró la tarjeta
}

// Función para registrar una nueva tarjeta
int registrarTarjeta(String cardID) {
  for (int i = 0; i < maxTarjetas; i++) {
    if (tarjetasID[i] == "") {
      tarjetasID[i] = cardID;
      return i;  // Retorna el índice donde se registró la nueva tarjeta
    }
  }
  Serial.println(F("No se pueden registrar más tarjetas, límite alcanzado."));
  return -1;  // Retorna -1 si no hay espacio para registrar más tarjetas
}

// Función para enviar datos al servidor
void sendData(String cardID, int ingreso) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;

        Serial.println("Enviando datos al servidor...");

        http.begin(serverUrl);
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");

        // Enviar el UUID de la tarjeta y el estado de ingreso/salida
        String dataToSend = "uuid=" + cardID + "&ingreso=" + String(ingreso);
        Serial.println(dataToSend);

        int httpCode = http.POST(dataToSend);

        if (httpCode > 0) {
            Serial.print("Código HTTP: ");
            Serial.println(httpCode);
            if (httpCode == HTTP_CODE_OK) {
                String response = http.getString();
                Serial.println("Respuesta del servidor:");
                Serial.println(response);
            }
        } else {
            Serial.print("Error al enviar POST, código: ");
            Serial.println(httpCode);
            Serial.println(http.errorToString(httpCode));
        }

        http.end();
    } else {
        Serial.println("Error en la conexión Wi-Fi");
    }
}
