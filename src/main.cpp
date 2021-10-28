/**
 * \file main.cpp
 * \brief Lectura de Temperatura [°C], Humedad Relativa [%], Radiación solar [w/m^2] y Velocidad del viento [m/s]
 * \author Joseph Santiago Portilla Martínez - Karen Stefania Mirama Eraso
 */ 
#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <SPI.h>
#include <SD.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <RTClib.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_ADS1X15.h>
#include "DAVIS6410.hpp"
#include "SHT3X.hpp"
#include "wifiSetup.hpp"
#include "mqttConfig.hpp"
#include "microSD.hpp"

// Habilitar o deshabilitar monitor serial
const bool serialDebug = true;
// Habilitar o deshabilitar debugging de WiFi
const bool debugWifi = true;
// Habilitar o deshabilitar pantalla OLED
const bool oledDisplay = false;

// ------------------------ DEFINICION DE CONSTANTES ------------------------
// Definicion de pines
// Sensor DAVIS 6410
const uint8_t vvPin = 23; // Cable Negro
const uint8_t dvPin = 34; // Cable Verde
const uint8_t sampleTime = 1; // Periodo de muestreo del sensor. [s]
const uint16_t dvOffset = 0; // Offset en el montaje de la veleta [°]
// Sensor SHT30
const uint8_t SDA_PIN = 21; // i2c SDA 
const uint8_t SCL_PIN = 22; // i2c SCL 
// Comunicación SPI
const uint8_t hspi_cs = 13;
const uint8_t hspi_mosi = 15;
const uint8_t hspi_sck = 14;
const uint8_t hspi_miso = 2;
// Display OLED
const uint8_t screenWidth = 128;
const uint8_t screenHeight = 64;
const uint8_t oledI2Caddress = 0x3C;
// Ganancia de ADS lectura de Radiación solar
const float adsGain = 0.25; // [mV/paso]
const float calFactor = 5.0; // [W/m2*mV]
// Ajustes de Wifi
const bool staticIP = false; // Ajuste para habilitar IP estatica
// Definicion de tiempos
const uint8_t DEEP_SLEEP_TIME = 1; // [Min]
const uint16_t Ts = 5000; // [ms]
const uint8_t samples = 1; // Número de muestras a tomar por los sensores

// ------------------------ DEFINICION DE VARIABLES ------------------------
// JSON
char buffer[1024]; // Buffer JSON a enviar por MQTT
// Definicion de tiempos
unsigned long tFinal = 0; // Tiempo actual de millis()
unsigned long t0 = 0; // Valor de tiempo previo
// Estado de la conexión wifi
bool wifiStatus = false;
// Estado de la escritura en la SD
bool sdStatus = false;
// Contador de intentos de conexión a broker MQTT
uint8_t mqttAttempts = 0;
// ------------------------ DECLARACIÓN DE OBJETOS ------------------------
// MQTT
WiFiClient wifiClient;
PubSubClient client(wifiClient);
// RTC
RTC_DS3231 rtc;
DateTime now;
// ADS
Adafruit_ADS1015 ads;
// OLED
Adafruit_SSD1306 display(screenWidth, screenHeight, &Wire);

// ------------------------ DEFINICION DE INTERRUPCIONES ------------------------
volatile int count = 0;
volatile int freq;
hw_timer_t * timer = NULL;
portMUX_TYPE synch = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
// Función de interrupción
void IRAM_ATTR isr() {
  portENTER_CRITICAL(&synch);
  count++; // Cada vez que se llama a la función el contador se incrementa
  portEXIT_CRITICAL(&synch);
}
// Función de interrupción de timer
void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  freq = count; // Se almacena el valor de la frecuencia
  count = 0; // Se reinicia el contador.
  portEXIT_CRITICAL_ISR(&timerMux);
}

// ------------------------ PROTOTIPOS DE FUNCIONES ------------------------
/**
 * \brief Conexión al broker y suscripción a topicos. Se verifica si hay conexión Wi-Fi;
 * se crea una ID unica para el cliente; se suscribe a los topicos.
*/
void reconnect();
/**
* @brief Se llamará a esta funcion siempre que haya un nuevo mensaje 
* de un topic al que se haya suscrito el cliente.
*/
void callback(char* topic, byte* payload, unsigned int length);

/**
 * \brief Función responsable de ajustar el timer despertador y entrar a deep sleep
*/
void goToDeepSleep () {
  if (serialDebug) Serial.println("\nEntrando a deep sleep");
  esp_sleep_enable_timer_wakeup(DEEP_SLEEP_TIME * 60 * 1000000);
  esp_deep_sleep_start();
}

// ------------------------ FUNCIÓN SETUP ------------------------
void setup() {
  delay(1000); // Tiempo de asentamiento
  t0 = millis();

  // ---------- AJUSTES ----------
	// Debugging por serial
  if (serialDebug) Serial.begin(115200);
  // Comunicación SPI
  SPI.begin(hspi_sck, hspi_miso, hspi_mosi, hspi_cs);
  // Comunicación I2C
  Wire.begin(SDA_PIN, SCL_PIN);
  // Display OLED
  if (oledDisplay) {
    display.begin(SSD1306_SWITCHCAPVCC, oledI2Caddress);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
  }
  // Configuración de Pines  
	pinMode(vvPin, INPUT);
	pinMode(dvPin, INPUT);
  // Ajuste de ganancia ADS
  ads.setGain(GAIN_EIGHT); // Ganancia = 8x, VDD = +/- 0.512V, Resolución: 1 bit = 0.25mV
  // Comprobación ADS
  if (!ads.begin()) {
    if (serialDebug) Serial.println("Error ADS");
    if (oledDisplay) {display.println("Error ADS"); display.display();};
  }
  // Comprobación SD
  if (!SD.begin(hspi_cs)){
    if (serialDebug) Serial.println("Error SD");
    if (oledDisplay) {display.println("Error SD"); display.display();}; 
  }
  // Comprobación RTC
  if (!rtc.begin()) {
    if (serialDebug) Serial.println("Error RTC");
    if (oledDisplay) {display.println("Error RTC"); display.display();};
  }
  // Calibración RTC
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // Comentar después de haber calibrado el RTC
  // Debugging de Fecha
  if (serialDebug) {
    DateTime fecha = rtc.now(); Serial.print("Hora = ");
    Serial.print(fecha.hour()); Serial.print(":"); Serial.print(fecha.minute()); Serial.print(":"); Serial.println(fecha.second());
  }
  if (oledDisplay) {
    DateTime fecha = rtc.now();
    display.print(fecha.day()); display.print("/"); display.print(fecha.month()); display.print("/");
    display.print(fecha.year()); display.print(", "); display.print(fecha.hour()); display.print(":");
    display.print(fecha.minute()); display.print(":"); display.println(fecha.second());
  }
  // Definicion e inicizalización de variables
  float vel = 0, velSum = 0; // Anemometro
  float dir = 0, dirSum = 0; // Veleta
  float tem = 0, rh = 0, temSum = 0, rhSum = 0; // Sensor SHT30
  float ghi = 0, ghiSum = 0; // Sensor SP110
  // Ajuste de Interruociones
  attachInterrupt(vvPin, isr, FALLING);
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 1000000, true);
  timerAlarmEnable(timer);  

  delay(1000); // Tiempo de asentamiento

  // ---------- MEDICIÓN ----------
  // Lectura acumulada
  for(int i=0; i<samples; i++){
    velSum += getSpeed(freq, sampleTime); // Lectura de Velocidad del Viento 
    dirSum += getDirection(dvPin, dvOffset); // Lectura de Dirección del Viento
    readData(tem, rh); // Lectura de Temperatura y Humedad Relativa
    temSum += tem;
    rhSum += rh;
    // ghiSum += ads.readADC_Differential_0_1() * adsGain * calFactor; // Lectura de Radiación solar

    delay(Ts);
  }

  // Lectura promedio
  vel = velSum / samples;
  dir = dirSum / samples;
  tem = temSum / samples;
  rh = rhSum / samples;
  ghi = ghiSum / samples;

  // Marcado de medida erronea.
  // Ante cualquier percance que genere medidas erroneas de los sensores,
  // se marca el dato como -100 para identificarlo en el procesamiento posterior.
  if (vel < 0) vel = -100; // Si la vel. viento es menor a 0 m/s
  if (tem < -10) tem = -100; // Si la temperatura es menor a -10 °C
  if (rh < 0) rh = -100; // Si la Hum. Relativa es menor a 0 %
  if (ghi < 0) ghi = -100;  // Si la Radiación es menos a 0 W/m^2

  // Debugging de mediciones
  if (serialDebug) {
    Serial.print("Vel. [m/s]: ");
    Serial.println(vel);    
    Serial.print("Dir.[°]: ");
    Serial.println(dir);    
    Serial.print("Temp. [°C]: ");
    Serial.println(tem);
    Serial.print("H. R. [%]: ");
    Serial.println(rh);
    Serial.print("Rad. [w/m^2]: ");
    Serial.println(ghi);
  }
  
  // Creación del buffer JSON con los datos de la medición tomada
  StaticJsonDocument<768> doc;
  JsonObject datosEMA = doc.createNestedObject("datosEMA");
  datosEMA["nombre"] = "EMA_estadero";
  datosEMA["ubicación"] = "Tuquerres";
  JsonObject temp = doc.createNestedObject("temp");
  temp["variable"] = "Temperatura_[°C]";
  temp["sensor"] = "Sensirion_SHT30";
  temp["valor"] = tem;
  JsonObject hum = doc.createNestedObject("hum");
  hum["variable"] = "Humedad_Relativa_[%]";
  hum["sensor"] = "Sensirion_SHT30";
  hum["valor"] = rh;
  JsonObject dv = doc.createNestedObject("dv");
  dv["variable"] = "Direccion_Viento_[°]";
  dv["sensor"] = "Davis_6410";
  dv["valor"] = dir;
  JsonObject vv = doc.createNestedObject("vv");
  vv["variable"] = "Velocidad_Viento_[m/s]";
  vv["sensor"] = "Davis_6410";
  vv["valor"] = vel;
  JsonObject rad = doc.createNestedObject("rad");
  rad["variable"] = "Radiacion_Solar_[W/m^2]";
  rad["sensor"] = "Apogee_SP-110";
  rad["valor"] = ghi;
  serializeJson(doc, buffer);  

  delay(1000); // Tiempo de asentamiento

  // ---------- CONEXIÓN Y ENVIO DE INFORMACIÓN ----------
  // Conectividad WiFi
  wifiStatus = wifiConnect(staticIP, debugWifi);
  if (oledDisplay == true && wifiStatus == false) {display.println("Error WiFi"); display.display();};
  // Protocolo MQTT
  client.setServer(mqtt_broker, mqtt_port);
  client.setBufferSize(1024);
  client.setCallback(callback);
  // Se valida la conexión.
  if (!client.connected()) {
    reconnect(); 
  }
  // Publicación del buffer en el broker
  if (client.connected()) {
    client.publish("MMAP/tuquerres1", buffer);
  }

  //---------- ESCRITURA EN LA MICROSD----------
  now = rtc.now();
  sdStatus = writeBuffer(now, tem, rh, dir, vel, ghi, wifiStatus, serialDebug);
  // Comprobación de escritura en la microSD
  if (serialDebug == true && sdStatus == false) Serial.println("\nError al abrir microSD");
  if (oledDisplay == true && sdStatus == false) {display.println("Error al abrir microSD"); display.display();};

  tFinal = millis();
  long delta = (20 * 1000) - (tFinal - t0);
  Serial.println(tFinal);
  Serial.println(delta);
  if (delta > 0) // Si la conexión y envio de datos se realizo en menos de 10 segundos
    delay(delta); // Delay para completar 60 segundos exactos

  //---------- ENTRADA A DEEP SLEEP ----------
  goToDeepSleep();

}

// ------------------------ LOOP ------------------------
void loop() {
  Serial.println("Función Loop N/A");
  delay(2000);
}

void reconnect() {
	// Si el dispositivo esta conectado a Wi-Fi -> Intentar conectar a MQTT
	if(WiFi.status() == WL_CONNECTED) {
		while (client.connected() == false && mqttAttempts < mqttMaxAttempts) {
      mqttAttempts += 1; // Contador de intentos de conexión al broker MQTT
			// Se crea una ID unica para el cliente.
			String clientId = "ESP32_MMAP-"; 			 // Sección fija del nombre
			clientId += String(random(0xffff), HEX); // Sección aleatoria concatenada

			if (serialDebug) Serial.print("Intentando conexión MQTT:");
      unsigned long startAttemptTime = millis();
      while(client.connect(clientId.c_str(), mqtt_user, mqtt_pass) != true && millis() < startAttemptTime + mqttTimeout) {
          if (serialDebug == true) Serial.print(".");
          delay(1000);
      }
			
			//Si hay conexión -> Suscribirse a los topicos sobre los que se quiere ser notificado
			if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
				if (serialDebug) Serial.println("MQTT Conectado");
			}
			else { // Si no hay conexión -> Enviar un mensaje de error
        if (serialDebug) { Serial.print("Error MQTT: "); Serial.println(client.state());}
			}
		}
	}
	// Si el dispositivo no esta conectado a Wi-Fi ->
	else {
		// connectToWiFi();
	}
}

void callback(char* topic, byte* payload, unsigned int length){
	//----------PROCESAMIENTO DE MENSAJE----------
	String msg = ""; //Se crea un string vacio para recibir el mensaje

  if (serialDebug == true) {
    Serial.print("Mensaje recibido en [");
    Serial.print(topic);
    Serial.print("]");
  }

	//Se recupera el mensaje caracter por caracter
	for (int i = 0; i < length; i++) {
		msg += (char)payload[i]; // Se concatenan los caracteres
	}

	msg.trim();// Se eliminan espacios innecesarios al inicio o al final
	if (serialDebug) Serial.println(": " + msg);

}


