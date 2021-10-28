/**
 * \file SHT3X.hpp
 * \brief Archivo de cabecera para el sensor SHT3X
 * \author Joseph Santiago Portilla Martínez - Karen Stefania Mirama Eraso
 */ 
#ifndef SHT3X_HPP
#define SHT3X_HPP

//---------------------------------------------------------
// DEFINICION DE CONSTANTES
//---------------------------------------------------------

#define SHT3X_I2C_ADDRESS 0x44 // ADDR -> GND
//#define SHT3X_I2C_ADDRESS 0x45 // ADDR -> VDD

// DS TABLA 8
// ------COMMAND MSB------
const uint8_t MSB_CSON = 0x2C;
const uint8_t MSB_CSOFF = 0x24;

// ------COMMAND LSB------
const uint8_t LSB_CSON_RH = 0x06;
const uint8_t LSB_CSON_RM = 0x0D;
const uint8_t LSB_CSON_RL = 0x10;
const uint8_t LSB_CSOFF_RH = 0x00;
const uint8_t LSB_CSOFF_RM = 0x0B;
const uint8_t LSB_CSOFF_RL = 0x16;

//---------------------------------------------------------
// PROTOTIPOS DE FUNCIONES
//---------------------------------------------------------

/**
 * \brief Enviar registros en comunicación I2C
 * \param address: Direccion de Registro
 * \param msb: Bit más representativo
 * \param lsb: Bit menos representativo
 * \return status: Indica el exito (0) o fallo de la transmisión (!=0)
*/
int sendReg(uint8_t address, uint8_t msb, uint8_t lsb);

/**
 * \brief Calculo de CheckSum (DS 4.12)
 * \param msbData: Bit más representativo
 * \param lsbData: Bit menos representativo
 * \param crcData: CRC checksum
 * \return status: Indica el exito (0) o fallo de la transmisión (!=0)
*/
bool CRC8(uint8_t msbData, uint8_t lsbData, uint8_t crcData);

/**
 * \brief Lectura, validación CRC y procesamiento de datos de T[°C] y RH[%]
 * \param temp: Puntero de variable de Temperatura
 * \param rh: Puntero de Variable de Humedad Relativa
*/
void readData(float& temp, float& rh);

/**
 * \brief Calcula la tolerancia de la temperatura dependiendo del valor medido
 * \param temp: Puntero de la Medición de Temperatura
 * \param tolerance: Puntero de la Tolerancia, obtenida en función de la temperatura
 * \param sensorType: Referencia del Sensor SHT3x (x=0, x=1, x=5)
 */
void tempTolerance(float& temp, float& tolerance, uint8_t sensorType);

/**
 * \brief Calcula la tolerancia de la Humedad Relativa dependiendo del valor medido
 * \param rh: Puntero de la Medición de Humedad Relativa
 * \param tolerance: Puntero de la Tolerancia, obtenida en función de la Humedad Relativa
 * \param sensorType: Referencia del Sensor SHT3x (x=0, x=1, x=5)
 */
void rhTolerance(float& rh, float& tolerance, uint8_t sensorType);


#endif