/**
 * \file microSD.hpp
 * \brief Archivo de cabecera para el almacenamiento de lecturas en microSD
 * \author Joseph Santiago Portilla Martínez - Karen Stefania Mirama Eraso
 */ 
#ifndef MICROSD_HPP
#define MICROSD_HPP

#include <RTClib.h>
//---------------------------------------------------------
// PROTOTIPOS DE FUNCIONES
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
bool writeBuffer(DateTime fecha, float temp, float hr, uint16_t dv, float vv, float rad, bool wifiStatus, bool serialDebug);

#endif