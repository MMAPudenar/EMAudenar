/**
 * \file SHT3X.cpp
 * \brief Archivo de implementación para el sensor SHT3X
 * \author Joseph Santiago Portilla Martínez - Karen Stefania Mirama Eraso
 */ 
#include <Arduino.h>
#include <Wire.h> // Para protocolo I2C
#include "SHT3X.hpp"

//---------------------------------------------------------
// IMPLEMENTACIÓN DE FUNCIONES
//---------------------------------------------------------

/**
 * \brief Enviar registros en comunicación I2C
 * \param address: Direccion de Registro
 * \param msb: Bit más representativo
 * \param lsb: Bit menos representativo
 * \return status: Indica el exito (0) o fallo de la transmisión (!=0)
*/
int sendReg(uint8_t address, uint8_t msb, uint8_t lsb) {
    uint8_t status;

    Wire.beginTransmission(address);
    Wire.write(msb);            
    Wire.write(lsb);
    status = Wire.endTransmission();

    return status;
}

/**
 * \brief Calculo de CheckSum (DS 4.12)
 * \param msbData: Bit más representativo
 * \param lsbData: Bit menos representativo
 * \param crcData: CRC checksum
 * \return status: Indica el exito (0) o fallo de la transmisión (!=0)
*/
bool CRC8(uint8_t msbData, uint8_t lsbData, uint8_t crcData) {

	uint8_t crc = 0xFF, i;
	
	crc ^= msbData; // Xor Bit a Bit

	for (i = 0; i < 8; i++) {
		crc = crc & 0x80 ? (crc << 1) ^ 0x31 : crc << 1;
	}

	crc ^= lsbData;

	for (i = 0; i < 8; i++) {
		crc = crc & 0x80 ? (crc << 1) ^ 0x31 : crc << 1;
	}

	if (crc == crcData) {
		return true;
	}
	else {
		return false;
	}
}

/**
 * \brief Lectura, validación CRC y procesamiento de datos de T[°C] y RH[%]
 * \param temp: Puntero de variable de Temperatura
 * \param rh: Puntero de Variable de Humedad Relativa
*/
void readData(float& temp, float& rh){
    uint8_t i2cstatus, data[6], i = 0;
    uint16_t rawTemp, rawRH;

    i2cstatus = sendReg(SHT3X_I2C_ADDRESS, MSB_CSON, LSB_CSON_RH);

    if (i2cstatus == 0){
		Wire.requestFrom(SHT3X_I2C_ADDRESS, 6);
		while(Wire.available()){
			data[i] = Wire.read();
			i++;
		}        

        if ( (CRC8(data[0], data[1], data[2])) && (CRC8(data[3], data[4], data[5])) ){
            rawTemp = (data[0] << 8) + (data[1] << 0);
            rawRH = (data[3] << 8) + (data[4] << 0);
			// DS 4.13
            temp = -45 + (rawTemp * 175.0 / 65535.0);
            rh = rawRH * 100.0 / 65535.0;
        }
        else{
            // Datos dañados
        }
    }
    else{
		// Error en la comunicación I2C
    }

}

/**
 * \brief Calcula la tolerancia de la temperatura dependiendo del valor medido
 * \param temp: Puntero de la Medición de Temperatura
 * \param tolerance: Puntero de la Tolerancia, obtenida en función de la temperatura
 * \param sensorType: Referencia del Sensor SHT3x (x=0, x=1, x=5)
 */
void tempTolerance(float& temp, float& tolerance, uint8_t sensorType) {

	switch (sensorType) {
        case 0: {
			if ((0 <= temp) && (temp <= 65)) {
				tolerance = 0.2;
			}
			else if (temp > 65)	{
				//Lineal de 0.2 (a 65 °C) a 0.6 (a 125 °C).
				tolerance = 0.0067 * temp - 0.2333;
			}
			else { // (temp < 0.)
				//Lineal de 0.6 (a -40 °C) a 0.2 (a 0 °C).
				tolerance = -0.01 * temp + 0.2;
			}
			break;
		}
		
        case 1: {
			if ((0 <= temp) && (temp <= 90)) {
				tolerance = 0.2;
			}
			else if (temp > 65)	{
				//Lineal de 0.2 (a 90 °C) a 0.5 (a 125 °C).
				tolerance = 0.0086 * temp - 0.5714;
			}
			else { // (temp < 0.)
				//Lineal de 0.3 (a -40 °C) a 0.2 (a 0 °C).
				tolerance = -0.0025 * temp + 0.2;
			}
			break;
		}
		
        case 5: {
			if (temp <= 0) {
				tolerance = 0.2;
			}
			else if ((0 < temp) && (temp <= 20)) {
				//Lineal de 0.2 (a 0 °C) a 0.1 (a 20 °C).
				tolerance = -0.005 * temp + 0.2;
			}
			else if ((20 < temp) && (temp <= 60)) {
				tolerance = 0.1;
			}
			else if ((60 < temp) && (temp <= 90)) {
				//Lineal de 0.1 (a 60 °C) a 0.2 (a 90 °C).
				tolerance = -0.0033 * temp - 0.1;
			}
			else { // (90. < temp)
				//Lineal de 0.2 (a 90 °C) a 0.4 (a 125 °C).
				tolerance = 0.0057 * temp - 0.3143;
			}
			break;
		}
	}	
}

/**
 * \brief Calcula la tolerancia de la Humedad Relativa dependiendo del valor medido
 * \param rh: Puntero de la Medición de Humedad Relativa
 * \param tolerance: Puntero de la Tolerancia, obtenida en función de la Humedad Relativa
 * \param sensorType: Referencia del Sensor SHT3x (x=0, x=1, x=5)
 */
void rhTolerance(float& rh, float& tolerance, uint8_t sensorType) {
	switch (sensorType) {
		case 0: {
			if ((10<= rh) && (rh <= 90)) {
				tolerance =  2.;
			}
			else if (rh < 10.) {
				//Lineal de 4 (a 0%) a 2 (a 10%).
				tolerance = -0.2 * rh + 4;
			}
			else {
				//Lineal de 2 (a 90%) a 4 (a 100%).
				tolerance =  0.2 * rh - 16;
			}
			break;
		}
		
        case 1: {
			tolerance = 2;
			break;
		}
		
        case 5: {
			if (rh <= 80) {
				tolerance = 1.5;
			}
			else { //if (80 < rh)
				//Lineal de 0.5 (a 80%) a 2 (a 100%).
				tolerance = 0.025 * rh - 0.5;
			}
			break;	
		}
	}
}
