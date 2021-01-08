/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   BME280.cpp
 * Author: manfredjonsson
 * 
 * Created on September 26, 2017, 1:19 PM
 */

#include "BME280.h"
#include <wiringPiI2C.h>
#include <unistd.h> // usleep



//#define DO_FLOAT_CALCULATION



#define BME280_REG_CHIPID           0xD0
#define BME280_REG_VERSION          0xD1

#define BME280_REG_SOFTRESET        0xE0
#define BME280_REG_STATUS           0xF3

#define BME280_REG_CRTL_HUM         0xF2
#define BME280_REG_CRTL_MEAS        0xF4
#define BME280_REG_CONFIG           0xF5

#define BME280_REG_PRESSURE         0xF7
#define BME280_REG_TEMPERATURE      0xFA
#define BME280_REG_HUMIDITY         0xFD

#define BME280_REG_DIG_T1           0x88
#define BME280_REG_DIG_T2           0x8A
#define BME280_REG_DIG_T3           0x8C

#define BME280_REG_DIG_P1           0x8E
#define BME280_REG_DIG_P2           0x90
#define BME280_REG_DIG_P3           0x92
#define BME280_REG_DIG_P4           0x94
#define BME280_REG_DIG_P5           0x96
#define BME280_REG_DIG_P6           0x98
#define BME280_REG_DIG_P7           0x9A
#define BME280_REG_DIG_P8           0x9C
#define BME280_REG_DIG_P9           0x9E

#define BME280_REG_DIG_H1           0xA1
#define BME280_REG_DIG_H2           0xE1
#define BME280_REG_DIG_H3           0xE3
#define BME280_REG_DIG_H4           0xE4
#define BME280_REG_DIG_H5           0xE5
#define BME280_REG_DIG_H6           0xE7



BME280::BME280() : i2cfd(0), chipid(0), t_fine(0), adc_t(0), adc_p(0), adc_h(0) {
}

BME280::BME280(const BME280& orig) {
}

BME280::~BME280() {
}

bool BME280::init(int fd) {
    i2cfd = fd;
    
    // Check chip id
    chipid = wiringPiI2CReadReg8(i2cfd, BME280_REG_CHIPID);
    if((chipid != 0x58) && (chipid != 0x60))
        return false;

    // reset the device using soft-reset
    // this makes sure the IIR is off, etc.
    wiringPiI2CWriteReg8(i2cfd, BME280_REG_SOFTRESET, 0xB6);
    // and wait until NVM is copied to registers.
    uint8_t status = 1;
    for (int i = 0; i < 10; ++i) {
        status = wiringPiI2CReadReg8(i2cfd, BME280_REG_STATUS);
        if (not (status & 0x01))
            break;
        usleep(100000);
    }
    if (status & 0x01)
        return false;

    // Read calibration data
    dig_T1 = wiringPiI2CReadReg16(i2cfd, BME280_REG_DIG_T1);
    dig_T2 = wiringPiI2CReadReg16(i2cfd, BME280_REG_DIG_T2);
    dig_T3 = wiringPiI2CReadReg16(i2cfd, BME280_REG_DIG_T3);

    dig_P1 = wiringPiI2CReadReg16(i2cfd, BME280_REG_DIG_P1);
    dig_P2 = wiringPiI2CReadReg16(i2cfd, BME280_REG_DIG_P2);
    dig_P3 = wiringPiI2CReadReg16(i2cfd, BME280_REG_DIG_P3);
    dig_P4 = wiringPiI2CReadReg16(i2cfd, BME280_REG_DIG_P4);
    dig_P5 = wiringPiI2CReadReg16(i2cfd, BME280_REG_DIG_P5);
    dig_P6 = wiringPiI2CReadReg16(i2cfd, BME280_REG_DIG_P6);
    dig_P7 = wiringPiI2CReadReg16(i2cfd, BME280_REG_DIG_P7);
    dig_P8 = wiringPiI2CReadReg16(i2cfd, BME280_REG_DIG_P8);
    dig_P9 = wiringPiI2CReadReg16(i2cfd, BME280_REG_DIG_P9);

    if (chipid == 0x60) {
        dig_H1 = wiringPiI2CReadReg8(i2cfd, BME280_REG_DIG_H1);
        dig_H2 = wiringPiI2CReadReg16(i2cfd, BME280_REG_DIG_H2);
        dig_H3 = wiringPiI2CReadReg8(i2cfd, BME280_REG_DIG_H3);
        uint8_t Addr_E4 = wiringPiI2CReadReg8(i2cfd, BME280_REG_DIG_H4);
        uint8_t Addr_E5 = wiringPiI2CReadReg8(i2cfd, BME280_REG_DIG_H5);
        uint8_t Addr_E6 = wiringPiI2CReadReg8(i2cfd, BME280_REG_DIG_H5 + 1);
        dig_H4 = ((Addr_E4 & 0xFF) << 4) + (Addr_E5 & 0x0F);
        dig_H5 = ((Addr_E5 >> 4) & 0x0F) + ((Addr_E6 & 0xFF) << 4);
        dig_H6 = wiringPiI2CReadReg8(i2cfd, BME280_REG_DIG_H6);
    }

    // Choose 62.5 ms Standby, Filter off
    wiringPiI2CWriteReg8(i2cfd, BME280_REG_CONFIG, 0x20);
    if (chipid == 0x60) {
        //Choose 16X oversampling for humidity
        wiringPiI2CWriteReg8(i2cfd, BME280_REG_CRTL_HUM, 0x05);
    }
    //Choose 16X oversampling for temp and pressure. Set normal mode
    wiringPiI2CWriteReg8(i2cfd, BME280_REG_CRTL_MEAS, 0xB7);

    return true;
}

/*
void BME280::readRawTemperature() {
    int xlsb = wiringPiI2CReadReg8(i2cfd, BME280_REG_TEMPERATURE + 2) & 0xFF;
    int lsb = wiringPiI2CReadReg8(i2cfd, BME280_REG_TEMPERATURE + 1) & 0xFF;
    int msb = wiringPiI2CReadReg8(i2cfd, BME280_REG_TEMPERATURE) & 0xFF;
    adc_t = (msb << 12) + (lsb << 4) + (xlsb >> 4);
}

void BME280::readRawPressure() {
    int xlsb = wiringPiI2CReadReg8(i2cfd, BME280_REG_PRESSURE + 2) & 0xFF;
    int lsb = wiringPiI2CReadReg8(i2cfd, BME280_REG_PRESSURE + 1) & 0xFF;
    int msb = wiringPiI2CReadReg8(i2cfd, BME280_REG_PRESSURE) & 0xFF;
    adc_p = (msb << 12) + (lsb << 4) + (xlsb >> 4);
}

void BME280::readRawHumidity() {
    if (chipid == 0x60) {
        int lsb = wiringPiI2CReadReg8(i2cfd, BME280_REG_HUMIDITY + 1) & 0xFF;
        int msb = wiringPiI2CReadReg8(i2cfd, BME280_REG_HUMIDITY) & 0xFF;
        adc_h = (msb << 8) + lsb;
    } else {
        adc_h = 0x8000; // 0x8000 is used in the BME280 when humidity is disabled
    }
}
*/

void BME280::readRaw() {
    int msb, lsb, xlsb;
    wiringPiI2CWrite(i2cfd, BME280_REG_PRESSURE);

    msb = wiringPiI2CRead(i2cfd);
    lsb = wiringPiI2CRead(i2cfd);
    xlsb = wiringPiI2CRead(i2cfd);
    adc_p = (msb << 12) + (lsb << 4) + (xlsb >> 4);

    msb = wiringPiI2CRead(i2cfd);
    lsb = wiringPiI2CRead(i2cfd);
    xlsb = wiringPiI2CRead(i2cfd);
    adc_t = (msb << 12) + (lsb << 4) + (xlsb >> 4);

    if (chipid == 0x60) {
        msb = wiringPiI2CRead(i2cfd);
        lsb = wiringPiI2CRead(i2cfd);
        adc_h = (msb << 8) + lsb;
    } else {
        adc_h = 0x8000; // 0x8000 is used in the BME280 when humidity is disabled
    }
}

#ifdef DO_FLOAT_CALCULATION
void BME280::read() {
    // readRawTemperature();
    // readRawPressure();
    // readRawHumidity();

    readRaw();

    double var1, var2;
    var1  = (((double)adc_t)/16384.0 - ((double)dig_T1)/1024.0) * ((double)dig_T2);
    var2  = ((((double)adc_t)/131072.0 - ((double)dig_T1)/8192.0) * (((double)adc_t)/131072.0 - ((double)dig_T1)/8192.0)) * ((double)dig_T3);
    t_fine = (int32_t)(var1 + var2);
}

float BME280::getTemperature() {
    double T  = t_fine / 5120.0;
    return T;
}

float BME280::getPressure() {
    double var1, var2, p;
    var1 = ((double)t_fine/2.0) - 64000.0;
    var2 = var1 * var1 * ((double)dig_P6) / 32768.0;
    var2 = var2 + var1 * ((double)dig_P5) * 2.0;
    var2 = (var2 / 4.0)+(((double)dig_P4) * 65536.0);
    var1 = (((double)dig_P3) * var1 * var1 / 524288.0 + ((double)dig_P2) * var1) / 524288.0;
    var1 = (1.0 + var1 / 32768.0)*((double)dig_P1);
    if (var1 == 0.0) {
        return 0;
        // avoid exception caused by division by zero
    }
    p = 1048576.0 - (double)adc_p;
    p = (p - (var2 / 4096.0)) * 6250.0 / var1;
    var1 = ((double)dig_P9) * p * p / 2147483648.0;
    var2 = p * ((double)dig_P8) / 32768.0;
    p = p + (var1 + var2 + ((double)dig_P7)) / 16.0;
    return p;
}

float BME280::getHumidity() {
    if (adc_h == 0x8000)
        return -1.0;

    double var_H;
    var_H = (((double)t_fine) - 76800.0);
    var_H = (adc_h - (((double)dig_H4) * 64.0 + ((double)dig_H5) / 16384.0 * var_H))
          * (((double)dig_H2) / 65536.0 * (1.0 + ((double)dig_H6) / 67108864.0 * var_H * (1.0 + ((double)dig_H3) / 67108864.0 * var_H)));
    var_H = var_H * (1.0 - ((double)dig_H1) * var_H / 524288.0);
    if (var_H > 100.0)
        var_H = 100.0;
    else
        if (var_H < 0.0)
            var_H = 0.0;
    return var_H;
}
#else
void BME280::read() {
    //readRawTemperature();
    //readRawPressure();
    //readRawHumidity();
    readRaw();

    int32_t var1, var2;
    var1  = ((((adc_t>>3) - ((int32_t)dig_T1<<1))) * ((int32_t)dig_T2)) >> 11;
    var2  = (((((adc_t>>4) - ((int32_t)dig_T1)) * ((adc_t>>4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;
    t_fine = var1 + var2;
}

float BME280::getTemperature() {
    int32_t T = (t_fine * 5 + 128) >> 8;
    return T / 100.0;
}

float BME280::getPressure() {
    int64_t var1, var2, p;
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)dig_P6;
    var2 = var2 + ((var1*(int64_t)dig_P5)<<17);
    var2 = var2 + (((int64_t)dig_P4)<<35);
    var1 = ((var1 * var1 * (int64_t)dig_P3)>>8) + ((var1 * (int64_t)dig_P2)<<12);
    var1 = (((((int64_t)1)<<47)+var1))*((int64_t)dig_P1)>>33;
    if (var1 == 0) {
        return 0;
        // avoid exception caused by division by zero
    }
    p = 1048576 - adc_p;
    p = (((p<<31) - var2) * 3125) / var1;
    var1 = (((int64_t)dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7) << 4);
    return p / 256.0;
}

float BME280::getHumidity() {
    if (adc_h == 0x8000)
        return -1.0;

    int32_t v_x1_u32r;
    v_x1_u32r = (t_fine - ((int32_t)76800));
    v_x1_u32r = (((((adc_h << 14) - (((int32_t)dig_H4) << 20) - (((int32_t)dig_H5) * v_x1_u32r)) + ((int32_t)16384)) >> 15) * (((((((v_x1_u32r * ((int32_t)dig_H6)) >> 10) * (((v_x1_u32r * ((int32_t)dig_H3)) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) * ((int32_t)dig_H2) + 8192) >> 14));
    v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((int32_t)dig_H1)) >> 4));
    v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r); 
    v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
    return (v_x1_u32r >> 12) / 1024.0;
}
#endif