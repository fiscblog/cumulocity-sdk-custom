/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   TSL2561.h
 * Author: manfredjonsson
 *
 * Created on September 27, 2017, 3:41 PM
 */

#ifndef TSL2561_H
#define TSL2561_H

class TSL2561 {
public:
    TSL2561();
    TSL2561(const TSL2561& orig);
    virtual ~TSL2561();
    
    bool init(int fd);
    
    void powerOn(bool state);
    
    void read();
    
    float getLux();
    
private:
    int i2cfd; // file descriptor to I2C device
    int partno;
    int ch0;
    int ch1;
    float lum;

};

#endif /* TSL2561_H */