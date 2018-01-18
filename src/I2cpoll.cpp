/*
 * I2cpoll.cpp
 *
 *  Created on: Jan 11, 2018
 *      Author: khomin
 */

#include "../inc/I2cpoll.h"
#include <iostream>
#include <stdlib.h>
#include <stdio.h>

I2c_poll::I2c_poll() {    
    db = new DbConnection();
    if(db->isConnecting()) {
        fprintf(stdout, "is connected\r\n");
    } else {
        fprintf(stderr, "db open -ERROR\r\n");
    }
    this->currentDeviceType = startDeviceType;
}

bool I2c_poll::readWord(char *cmd_command, uint16_t *p_word) {
    int res = false;
    int word = 0;
    FILE *stream;
    // отправляем в консоль
    stream = popen(cmd_command, "r");
    stream = popen("\r", "r");
    std::string data;
    if (stream) {
        char reply_buff[128] = {0};
        while (!feof(stream))
        if(fgets(reply_buff, sizeof(reply_buff), stream) != NULL) data.append(reply_buff);
            pclose(stream);
    }
    // проверяем ответ
//    data.insert(0, "I will read from device file /dev/i2c-0,\r\n0x401d\r\n");
    fprintf(stdout, "shell result :\r\n%s\r\n", data.c_str());
    if(data.length() != 0) {
        char *p_start = strstr((char*)data.c_str(), "0x");
        if(p_start != NULL) {
            res = sscanf(p_start, "%x", &word);
            if(res) {
                *p_word = (uint16_t)word;
            }
            fprintf(stdout, "getWord %x\r\n", *p_word);
        }
    }
    return res;
}

void* I2c_poll::pollExect() {
//    FILE *stream;
    uint8_t *p_data = NULL;
    uint16_t word = 0;
    char cmd_command[128] = {0};
    S_insertData device_data;    
    int device_addr = 0;
    // формируем запрос
    device_data.device_type = (E_I2c_device)currentDeviceType;
    switch(currentDeviceType) {
        
        case dev_i2c_tmp112:
            
            device_addr = db->getDeviceAddrFromName(i2c_Dev_typeName.tmp112.name_text.c_str());
            if(device_addr != 0) {
                sprintf(cmd_command, "i2cget 0 %X 0x00 w", device_addr); //i2cget 0 0x48 0x00 w
                if(readWord(cmd_command, &word)) {  // проверяем ответ
                    p_data = (uint8_t*)&word;
                    device_data.parameter.temp.temp = (p_data[0] * 256 + p_data[1]) / 16;
                    if(device_data.parameter.temp.temp > 2047) {
                        device_data.parameter.temp.temp -= 4096;
                    }
                    device_data.parameter.temp.temp = device_data.parameter.temp.temp * 0.0625;
                    fprintf(stdout, "Temp %4.2f", device_data.parameter.temp.temp);
                    db->insertData(device_data);
                }
            }
            break;
            
        case dev_i2c_ina260:
            device_addr = db->getDeviceAddrFromName(i2c_Dev_typeName.tmp112.name_text.c_str());
            if(device_addr != 0) {                
                for(int i=0; i<3; i++) {                    
                        switch(i) { // cur
                        case 0: //{i2cget 0 0x40 0x01 w}
                        sprintf(cmd_command, "i2cget 0 %X 0x01 w", device_addr);
                        // отправляем в консоль
                        if(readWord(cmd_command, &word)) {  // проверяем ответ
                            p_data = (uint8_t*)&word;
                            float raw_data; 
                            raw_data = (0xFF & p_data[1]) | ((0xFF & (p_data[0])) << 8);
                            device_data.parameter.power_monitor.currnetFlowing = raw_data * 1.25 / 1000;
                            fprintf(stdout, "Current %4.2f\r\n", device_data.parameter.power_monitor.currnetFlowing);
                        }
                        break;    
                        // voltage
                        case 1: //{i2cget 0 0x40 0x02 w}
                        sprintf(cmd_command, "i2cget 0 %X 0x02 w", device_addr);
                        // отправляем в консоль
                        if(readWord(cmd_command, &word)) {  // проверяем ответ
                            p_data = (uint8_t*)&word;
                            float raw_data;
                            raw_data = (0xFF & p_data[1]) | ((0xFF & (p_data[0])) << 8);
                            device_data.parameter.power_monitor.voltage = raw_data * 1.25 / 1000;
                            fprintf(stdout, "Volage %4.2f\r\n", device_data.parameter.power_monitor.voltage);
                        }
                        break;
                                                    
                        case 2: //{i2cget 0 0x40 0x03 w}
                        //  power
                        sprintf(cmd_command, "i2cget 0 %X 0x03 w", device_addr);
                        // отправляем в консоль
                        if(readWord(cmd_command, &word)) {  // проверяем ответ
                            p_data = (uint8_t*)&word;
                            float raw_data;
                            raw_data = (0xFF & p_data[1]) | ((0xFF & (p_data[0])) << 8);
                            device_data.parameter.power_monitor.currentPower = raw_data * 1.25;
                            fprintf(stdout, "Power %4.2f\r\n", device_data.parameter.power_monitor.currentPower);
                        }
                        break;
                    }   
                }
                db->insertData(device_data);
            }
            break;
            
            case dev_i2c_lis3dh:
//                i16_t value;
//                u8_t *valueL = (u8_t *)(&value);
//                u8_t *valueH = ((u8_t *)(&value)+1);
//
//                if( !LIS3DH_ReadReg(LIS3DH_OUT_X_L, valueL) )
//                return MEMS_ERROR;
//
//                if( !LIS3DH_ReadReg(LIS3DH_OUT_X_H, valueH) )
//                return MEMS_ERROR;
//
//                buff->AXIS_X = value;
//
//                if( !LIS3DH_ReadReg(LIS3DH_OUT_Y_L, valueL) )
//                return MEMS_ERROR;
//
//                if( !LIS3DH_ReadReg(LIS3DH_OUT_Y_H, valueH) )
//                return MEMS_ERROR;
//
//                buff->AXIS_Y = value;
//
//                if( !LIS3DH_ReadReg(LIS3DH_OUT_Z_L, valueL) )
//                return MEMS_ERROR;
//
//                if( !LIS3DH_ReadReg(LIS3DH_OUT_Z_H, valueH) )
//                return MEMS_ERROR;
//
//                buff->AXIS_Z = value;
//
//                return MEMS_SUCCESS; 
//                
//                if(response != MEMS_ERROR) {
//                    //DBGLog("Accel: X=%6d Y=%6d Z=%6d", data.AXIS_X, data.AXIS_Y, data.AXIS_Z);				
//
//                    accX = ((float)data.AXIS_X)*2*9.81/32768; 
//                    accY = ((float)data.AXIS_Y)*2*9.81/32768;
//                    accZ = ((float)data.AXIS_Z)*2*9.81/32768; 				
//
//                    accelTestStruct.data.AXIS_X = data.AXIS_X;
//                    accelTestStruct.data.AXIS_Y = data.AXIS_Y;
//                    accelTestStruct.data.AXIS_Z = data.AXIS_Z;
//                    accelTestStruct.accX = accX;
//                    accelTestStruct.accY = accY;
//                    accelTestStruct.accZ = accZ;
//                    if(state != WORKING) {
//                            accelTestStruct.enabled = false;
//                    }else{
//                            accelTestStruct.enabled = true;
//                    }
//                }            
            break;
            
        case dev_i2c_txs02324:
            break;
            
        default: 
            currentDeviceType = startDeviceType;
            break;
    }
       
    if(currentDeviceType >= (E_I2c_device)endTypeDeviceType) {
        currentDeviceType = startDeviceType;
        fprintf(stdout, "I2c parce -endDev\r\n");
    } else {
        currentDeviceType++;
        fprintf(stdout, "I2c parce -nextDev\r\n");
    }
    return 0;
}

I2c_poll::~I2c_poll() {
	// TODO Auto-generated destructor stub
}

