/**
 * \file wifiSetup.hpp
 * \brief Archivo de cabecera
 * \author Joseph Santiago Portilla Martínez
 */ 
#ifndef WIFISETUP_HPP
#define WIFISETUP_HPP

/**
* @brief Función para conectar el ESP a WiFi
* @param useStaticIP: Opción para utilizar IP estatica
* @return Estado de la conexión. True=Se pudo conectar.
*/
bool wifiConnect(bool useStaticIP, bool debugging);



#endif //INC_BME280_HPP