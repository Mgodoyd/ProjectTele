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

  Serial.println(F("El NUID de la tarjeta es:"));
  Serial.print(F("En hexadecimal: "));
  printHex(rfid.uid.uidByte, rfid.uid.size);
  Serial.println();
  Serial.print(F("En decimal: "));
  printDec(rfid.uid.uidByte, rfid.uid.size);
  Serial.println();

  // Convertir el UID a String para el envío
  String cardID = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    cardID += String(rfid.uid.uidByte[i], HEX);
  }
  
  // Verifica si ha pasado el intervalo de tiempo
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;  // Guardar el tiempo actual

    // Definir el valor de ingreso (puedes cambiar la lógica según sea necesario)
    int ingreso = 1;  // Por defecto lo dejamos como 1

    // Enviar datos al servidor
    sendData(cardID, ingreso);  // Se utiliza cardID y el valor de ingreso
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

// Función para enviar datos al servidor
void sendData(String cardID, int ingreso) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;

        Serial.println("Enviando datos al servidor...");

        http.begin(serverUrl);
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");

        String dataToSend = "uuid=" + cardID + "&ingreso=" + ingreso;
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
