/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   BME280.h
 * Author: manfredjonsson
 *
 * Created on September 26, 2017, 1:19 PM
 * 
 * https://www.bosch-sensortec.com/bst/products/all_products/bme280
 * https://ae-bst.resource.bosch.com/media/_tech/media/datasheets/BST-BME280_DS001-11.pdf
 * https://www.bosch-sensortec.com/bst/products/all_products/bme280
 * https://ae-bst.resource.bosch.com/media/_tech/media/datasheets/BST-BMP280-DS001-18.pdf
 * 
 */

#ifndef BME280_H
#define BME280_H

#include <cstdint>



class BME280 {
public:
    BME280();
    BME280(const BME280& orig);
    virtual ~BME280();

    bool init(int fd);

    void read();
    
    float getTemperature();
    float getPressure();
    float getHumidity();

private:
    // void readRawTemperature();
    // void readRawPressure();
    // void readRawHumidity();
    void readRaw();

    int i2cfd; // file descriptor to I2C device
    int chipid; //Value from the chip id register

    int32_t t_fine;
    int32_t adc_t;
    int32_t adc_p;
    int32_t adc_h;

    // Calibration values from the sensor
    uint16_t dig_T1;
    int16_t dig_T2;
    int16_t dig_T3;

    uint16_t dig_P1;
    int16_t dig_P2;
    int16_t dig_P3;
    int16_t dig_P4;
    int16_t dig_P5;
    int16_t dig_P6;
    int16_t dig_P7;
    int16_t dig_P8;
    int16_t dig_P9;

    uint8_t dig_H1;
    int16_t dig_H2;
    uint8_t dig_H3;
    int16_t dig_H4;
    int16_t dig_H5;
    int8_t dig_H6;
};

#endif /* BME280_H */

