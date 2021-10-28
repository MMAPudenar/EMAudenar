/**
 * \file wifiConfig.hpp
 * \brief Archivo de ajustes para conectividad WiFi
 * \author Joseph Santiago Portilla Martínez
 */
// const char* ssid     = "Adriana";
// const char* password = "59829864est";
const char* ssid     = "PORTILLA ";
const char* password = "12958394";
const char* hostname = "ESP32";
const uint16_t wifiTimeout = 20000;
const uint8_t channel = 7;

IPAddress ip(192, 168, 0, 70);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);