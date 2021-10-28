/**
 * \file SP110.hpp
 * \brief Archivo de cabecera para el piranómetro APOGEE SP110
 * \author Joseph Santiago Portilla Martínez - Karen Stefania Mirama Eraso
 */ 
#ifndef SP110_HPP
#define SP110_HPP

//---------------------------------------------------------
// DEFINICION DE CONSTANTES
//---------------------------------------------------------
// ADC ESP32
const float vRef = 3.3; // [V]
const float resolution = 4095.0; // bits
const float scaleFactor = vRef / resolution; // [V] / bits

// Ajuste de PWM
const uint16_t pwmFreq = 5000;
const uint8_t pwmChannel = 0;
const uint8_t pwmResolution = 8;
const uint8_t dutyCycle = 128; // 50% de PWM

//---------------------------------------------------------
// PROTOTIPOS DE FUNCIONES
//---------------------------------------------------------

/**
 * \brief Leer la medida analogica del piranometro y convertirla en Radiación
 * \param PiranometroPin: Pin de conexión del piranómetro
 * \param opampGain: Ganancia del amplificador operacional
 * \return Radiación en W/m^2
*/
float getRadiation(uint8_t PiranometroPin, float opampGain);


#endif