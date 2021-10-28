/**
 * \file microSD.cpp
 * \brief Archivo de implementación para el almacenamiento de lecturas en microSD
 * \author Joseph Santiago Portilla Martínez - Karen Stefania Mirama Eraso
 */ 
#include <Arduino.h>
#include "microSD.hpp"
#include <SD.h>
#include <RTClib.h>

//---------------------------------------------------------
// IMPLEMENTACIÓN DE FUNCIONES
//---------------------------------------------------------

/**
 * \brief Creación de buffer a escribir en la microSD
 * \param temp: Lectura de Temperatura
 * \param hr: Lectura de Hum. Relativa
 * \param dv: Lectura de Dir. Viento
 * \param vv: Lectura de Vel. Viento
 * \param rad: Lectura de Radiación
 * \param fecha: Fecha en la que se tomo las lecturas
 * \param wifiStatus: Estado de conexión en la fecha
 * \param serialDebug: Debugging de buffer por serial
*/
bool writeBuffer(DateTime fecha, float temp, float hr, uint16_t dv, float vv, float rad, bool wifiStatus, bool serialDebug){
  char buffersd[64]; // Buffer a registrar en la MicroSD
  File datalogger;
  bool status;

  // ---------- CREACIÓN DE BUFFER ----------  

  char date[12];
  sprintf(date, "%02d/%02d/%04d,", fecha.day(), fecha.month(), fecha.year());
  strcpy(buffersd, date);

  char time[12];
  sprintf(time, "%02d:%02d:%02d,", fecha.hour(), fecha.minute(), fecha.second());
  strcat(buffersd, time);

  char tempChar[7];
  dtostrf(temp, 4, 2, tempChar);
  strcat(buffersd, tempChar);
  strcat(buffersd,",");

  char humChar[7];
  dtostrf(hr, 4, 2, humChar);
  strcat(buffersd, humChar);
  strcat(buffersd,",");

  char dvChar[7];
  dtostrf(dv, 4, 2, dvChar);
  strcat(buffersd, dvChar);
  strcat(buffersd,",");
  
  char vvChar[7];
  dtostrf(vv, 4, 2, vvChar);
  strcat(buffersd, vvChar);
  strcat(buffersd,",");
  
  char radChar[7];
  dtostrf(rad, 4, 2, radChar);
  strcat(buffersd, radChar);
  strcat(buffersd,",");

  if (wifiStatus == true) {
    strcat(buffersd,"1");
  }
  else {
    strcat(buffersd,"0");
  }

  if (serialDebug) Serial.print("Buffer a escribir en la MicroSD: "); Serial.println(buffersd);

  // ---------- ESCRITURA DE BUFFER ----------  
  datalogger = SD.open("/EMAtuq.txt", FILE_WRITE);

  if (datalogger) {
    datalogger.println(buffersd);
    datalogger.close();
    status = true;
  }
  else {
    status = false;
  }

  return status;

}