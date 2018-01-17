/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   global.h
 * Author: khomin
 *
 * Created on January 17, 2018, 12:30 PM
 */

#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdio.h>
#include <string>

using namespace std;

    typedef enum {
        dev_i2c_unknow      = 0,
        dev_i2c_tmp112      = 1,
        dev_i2c_lis3dh      = 2, 
        dev_i2c_ina260      = 3,
        dev_i2c_txs02324    = 4,
    }E_I2c_device;
    
    typedef struct {
        E_I2c_device type;
        std::string name_text;
    }S_Dev_typeName;
        
    struct {
        S_Dev_typeName unknow = {dev_i2c_unknow, "unknow"};
        S_Dev_typeName tmp112 = {dev_i2c_tmp112, "TMP112"};
        S_Dev_typeName lis3dh = {dev_i2c_lis3dh, "LIS3DH"};
        S_Dev_typeName ina260 = {dev_i2c_ina260, "INA260"};
        S_Dev_typeName txs02324 = {dev_i2c_txs02324, "TXS02324"};
    }i2c_Dev_typeName;    
    
    typedef struct {
        E_I2c_device device_type;
        union {
            struct {
                float x;
                float y;
                float z;
                float velocityX;
                float velocityY;
                float velocityZ;
                bool isInited;
            }accel;
                
            struct{
                float temp;
            }temp;

            struct {
                float currentPower;
                float currnetFlowing;
                float voltage;
            }power_monitor;

            struct {
                bool cardPowerIsUp;
                int currentSimSlot;
            }sim_switcher;         
        }parameter;
    }S_insertData;

#endif /* GLOBAL_H */

