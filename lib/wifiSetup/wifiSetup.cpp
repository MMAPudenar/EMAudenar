/**
 * \file wifiSetup.cpp
 * \brief Archivo de implementación
 * \author Joseph Santiago Portilla Martínez
 */ 
#include <Arduino.h>
#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif
#include "wifiConfig.hpp"
#include "wifiSetup.hpp"

bool wifiConnect(bool useStaticIP, bool debugging) {
    bool status;

    if (debugging == true) { Serial.print("Conectando a red WiFi: "); Serial.println(ssid);}
    
    /* Configure el ESP para que sea un cliente WiFi, de lo contrario, por defecto, actúa tanto como cliente
    como punto de acceso y puede causar problemas de red con otros dispositivos WiFi en la red.*/
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    if(useStaticIP) WiFi.config(ip, gateway, subnet);

    unsigned long startAttemptTime = millis();
    while(WiFi.status() != WL_CONNECTED && millis() < startAttemptTime + wifiTimeout) {
        if (debugging == true) Serial.print(".");
        delay(1000);
    }

    if (WiFi.status() != WL_CONNECTED) { status = false; } else { status = true; }

    if (debugging == true && status == false) {Serial.println("\nError en la conexión");}

    if (debugging == true && status == true)
        {Serial.print("\nIP: "); Serial.print(WiFi.localIP()); Serial.print("\nRSSI: "); Serial.println(WiFi.RSSI());}

    return status;
}