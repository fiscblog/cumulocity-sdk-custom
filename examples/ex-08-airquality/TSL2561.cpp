/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   TSL2561.cpp
 * Author: manfredjonsson
 * 
 * Created on September 27, 2017, 3:41 PM
 */

#include "TSL2561.h"
#include <wiringPiI2C.h>

#define TSL2561_REG_CONTROL           0x80
#define TSL2561_REG_TIMING            0x81
#define TSL2561_REG_INTERRUPT         0x86
#define TSL2561_REG_ID                0x8A
#define TSL2561_REG_DATA0LOW          0x8C
#define TSL2561_REG_DATA0HIGH         0x8D
#define TSL2561_REG_DATA1LOW          0x8E
#define TSL2561_REG_DATA1HIGH         0x8F



TSL2561::TSL2561() : i2cfd(0), partno(0), lum(0.0) {
}

TSL2561::TSL2561(const TSL2561& orig) {
}

TSL2561::~TSL2561() {
}

bool TSL2561::init(int fd) {
    i2cfd = fd;
    
    int id = wiringPiI2CReadReg8(i2cfd, TSL2561_REG_ID);
    
    partno = (id >> 4) & 0x0F;
    //lum = id & 0x0F;
    lum = id;
    
    if((partno & 0x0B) != 0x01) // 0b0101 and 0b0001 are ok
        return false;

    wiringPiI2CWriteReg8(i2cfd, TSL2561_REG_CONTROL, 0x03); // Power up
    wiringPiI2CWriteReg8(i2cfd, TSL2561_REG_TIMING, 0x00); //Gain 1x, integration time 13ms
    wiringPiI2CWriteReg8(i2cfd, TSL2561_REG_INTERRUPT, 0x00);
    wiringPiI2CWriteReg8(i2cfd, TSL2561_REG_CONTROL, 0x00); // Power down

    return true;
}

void TSL2561::powerOn(bool state) {
    wiringPiI2CWriteReg8(i2cfd, TSL2561_REG_CONTROL, (state) ? 0x03 : 0x00);
}

void TSL2561::read() {
    int lsb = wiringPiI2CReadReg8(i2cfd, TSL2561_REG_DATA0LOW);
    int msb = wiringPiI2CReadReg8(i2cfd, TSL2561_REG_DATA0HIGH);
    ch0 = (msb << 8) | lsb;
    lsb = wiringPiI2CReadReg8(i2cfd, TSL2561_REG_DATA1LOW);
    msb = wiringPiI2CReadReg8(i2cfd, TSL2561_REG_DATA1HIGH);
    ch1 = (msb << 8) | lsb;
}

#define LUX_SCALE 14 // scale by 2^14
#define RATIO_SCALE 9 // scale ratio by 2^9
#define CH_SCALE 10 // scale channel values by 2^10
#define CHSCALE_TINT0 0x7517 // 322/11 * 2^CH_SCALE
#define CHSCALE_TINT1 0x0fe7 // 322/81 * 2^CH_SCALE

#define K1T 0x0040 // 0.125 * 2^RATIO_SCALE
#define B1T 0x01f2 // 0.0304 * 2^LUX_SCALE
#define M1T 0x01be // 0.0272 * 2^LUX_SCALE
#define K2T 0x0080 // 0.250 * 2^RATIO_SCALE
#define B2T 0x0214 // 0.0325 * 2^LUX_SCALE
#define M2T 0x02d1 // 0.0440 * 2^LUX_SCALE
#define K3T 0x00c0 // 0.375 * 2^RATIO_SCALE
#define B3T 0x023f // 0.0351 * 2^LUX_SCALE
#define M3T 0x037b // 0.0544 * 2^LUX_SCALE
#define K4T 0x0100 // 0.50 * 2^RATIO_SCALE
#define B4T 0x0270 // 0.0381 * 2^LUX_SCALE
#define M4T 0x03fe // 0.0624 * 2^LUX_SCALE
#define K5T 0x0138 // 0.61 * 2^RATIO_SCALE
#define B5T 0x016f // 0.0224 * 2^LUX_SCALE
#define M5T 0x01fc // 0.0310 * 2^LUX_SCALE
#define K6T 0x019a // 0.80 * 2^RATIO_SCALE
#define B6T 0x00d2 // 0.0128 * 2^LUX_SCALE
#define M6T 0x00fb // 0.0153 * 2^LUX_SCALE
#define K7T 0x029a // 1.3 * 2^RATIO_SCALE
#define B7T 0x0018 // 0.00146 * 2^LUX_SCALE
#define M7T 0x0012 // 0.00112 * 2^LUX_SCALE
#define K8T 0x029a // 1.3 * 2^RATIO_SCALE
#define B8T 0x0000 // 0.000 * 2^LUX_SCALE
#define M8T 0x0000 // 0.000 * 2^LUX_SCALE

#define K1C 0x0043 // 0.130 * 2^RATIO_SCALE
#define B1C 0x0204 // 0.0315 * 2^LUX_SCALE
#define M1C 0x01ad // 0.0262 * 2^LUX_SCALE
#define K2C 0x0085 // 0.260 * 2^RATIO_SCALE
#define B2C 0x0228 // 0.0337 * 2^LUX_SCALE
#define M2C 0x02c1 // 0.0430 * 2^LUX_SCALE
#define K3C 0x00c8 // 0.390 * 2^RATIO_SCALE
#define B3C 0x0253 // 0.0363 * 2^LUX_SCALE
#define M3C 0x0363 // 0.0529 * 2^LUX_SCALE
#define K4C 0x010a // 0.520 * 2^RATIO_SCALE
#define B4C 0x0282 // 0.0392 * 2^LUX_SCALE
#define M4C 0x03df // 0.0605 * 2^LUX_SCALE
#define K5C 0x014d // 0.65 * 2^RATIO_SCALE
#define B5C 0x0177 // 0.0229 * 2^LUX_SCALE
#define M5C 0x01dd // 0.0291 * 2^LUX_SCALE
#define K6C 0x019a // 0.80 * 2^RATIO_SCALE
#define B6C 0x0101 // 0.0157 * 2^LUX_SCALE
#define M6C 0x0127 // 0.0180 * 2^LUX_SCALE
#define K7C 0x029a // 1.3 * 2^RATIO_SCALE
#define B7C 0x0037 // 0.00338 * 2^LUX_SCALE
#define M7C 0x002b // 0.00260 * 2^LUX_SCALE
#define K8C 0x029a // 1.3 * 2^RATIO_SCALE
#define B8C 0x0000 // 0.000 * 2^LUX_SCALE
#define M8C 0x0000 // 0.000 * 2^LUX_SCALE


float TSL2561::getLux() {
    
    unsigned int integrationTimeEnum = 0;
    // Scale on integration time
    unsigned long chScale; 
    switch (integrationTimeEnum) {
        case 0: // 13.7 msec 
            chScale = CHSCALE_TINT0;
            break;
        case 1: // 101 msec
            chScale = CHSCALE_TINT1;
            break;
        default: // assume no scaling (402 msec)
            chScale = (1 << CH_SCALE);
            break;
    }
    
    bool gain = false; 
    // Scale if gain is NOT 16X
    if (!gain) chScale = chScale << 4; // scale 1X to 16X
    
    // scale the channel values
    unsigned long channel0 = (ch0 * chScale) >> CH_SCALE;
    unsigned long channel1 = (ch1 * chScale) >> CH_SCALE;
    
    // find the ratio of the channel values (Channel1/Channel0) and protect against divide by zero 
    unsigned long ratio1 = 0;
    if (channel0 != 0)
        ratio1 = (channel1 << (RATIO_SCALE+1)) / channel0;
// round the ratio value 
    unsigned long ratio = (ratio1 + 1) >> 1;
    
    // is ratio <= eachBreak ?
    unsigned int b, m;
    
    int iType = 0;
    
    switch (iType) {
        case 0: // T package
            if ((ratio >= 0) && (ratio <= K1T)) {b=B1T; m=M1T;}
            else if (ratio <= K2T) {b=B2T; m=M2T;}
            else if (ratio <= K3T) {b=B3T; m=M3T;}
            else if (ratio <= K4T) {b=B4T; m=M4T;}
            else if (ratio <= K5T) {b=B5T; m=M5T;}
            else if (ratio <= K6T) {b=B6T; m=M6T;}
            else if (ratio <= K7T) {b=B7T; m=M7T;}
            else if (ratio > K8T) {b=B8T; m=M8T;}
            break;
        case 1:// CS package
            if ((ratio >= 0) && (ratio <= K1C)) {b=B1C; m=M1C;}
            else if (ratio <= K2C) {b=B2C; m=M2C;}
            else if (ratio <= K3C) {b=B3C; m=M3C;}
            else if (ratio <= K4C) {b=B4C; m=M4C;}
            else if (ratio <= K5C) {b=B5C; m=M5C;}
            else if (ratio <= K6C) {b=B6C; m=M6C;}
            else if (ratio <= K7C) {b=B7C; m=M7C;}
            else if (ratio > K8C) {b=B8C; m=M8C;}
            break;
    }
    unsigned long temp;
    temp = ((channel0 * b) - (channel1 * m));
    // do not allow negative lux value
    if (temp < 0) temp = 0;
    // round lsb (2^(LUX_SCALE−1))
    temp += (1 << (LUX_SCALE - 1));
    // strip off fractional portion
    unsigned long lux = temp >> LUX_SCALE;
    return(lux);
}