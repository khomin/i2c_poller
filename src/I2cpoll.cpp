/*
 * I2cpoll.cpp
 *
 *  Created on: Jan 11, 2018
 *      Author: khomin
 */

#include "../inc/I2cpoll.h"

I2c_poll::I2c_poll() {    
    db = new DbConnection();
    if(db->isConnecting()) {
        fprintf(stdout, "is connected\r\n");
    } else {
        fprintf(stderr, "db open -ERROR\r\n");
    }
    
    this->currentDeviceType = startDeviceType;
}

void* I2c_poll::pollExect() {
    int i = 0;
    uint16_t raw_data = 0;
    char reply_buff[64] = {0};
    int reply_len = 0;
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
                // отправляем в консоль
                // ...
                // проверяем ответ
                reply_buff[reply_len++] = 0xF0;
                reply_buff[reply_len++] = 0x1D;
                device_data.parameter.temp.temp = (reply_buff[1] * 256 + reply_buff[0]) / 16;
                if(device_data.parameter.temp.temp > 2047) {
                    device_data.parameter.temp.temp -= 4096;
                }
                device_data.parameter.temp.temp = device_data.parameter.temp.temp * 0.0625;
                fprintf(stdout, "Temp %4.2f", device_data.parameter.temp.temp);
            }
            break;
            
        case dev_i2c_ina260:
            device_addr = db->getDeviceAddrFromName(i2c_Dev_typeName.tmp112.name_text.c_str());
            if(device_addr != 0) {               
                for(i=0; i<3; i++) {
                    reply_len = 0;
                    switch(i) { // cur
                        case 0: //{i2cget 0 0x40 0x01 w}
                        sprintf(cmd_command, "i2cget 0 %X 0x01 w", device_addr);
                        // отправляем в консоль
                        // ...
                        // проверяем ответ                    
                        reply_buff[reply_len++] = 0x45;
                        reply_buff[reply_len++] = 0x01;
                        raw_data = (0xFF & reply_buff[0]) | ((0xFF & (reply_buff[1])) << 8);
                        device_data.parameter.power_monitor.currnetFlowing = raw_data * 1.25 / 1000;
                        fprintf(stdout, "Current %4.2f\r\n", device_data.parameter.power_monitor.currnetFlowing);
                        break;    
                        // voltage
                        case 1: //{i2cget 0 0x40 0x02 w}
                        sprintf(cmd_command, "i2cget 0 %X 0x02 w", device_addr);
                        // отправляем в консоль
                        // ...
                        // проверяем ответ          
                        reply_buff[reply_len++] = 0x8D;
                        reply_buff[reply_len++] = 0x24;
                        raw_data = (0xFF & reply_buff[0]) | ((0xFF & (reply_buff[1])) << 8);
                        device_data.parameter.power_monitor.voltage = raw_data * 1.25 / 1000;
                        fprintf(stdout, "Volage %4.2f\r\n", device_data.parameter.power_monitor.voltage);
                        break;
                                                    
                        case 2: //{i2cget 0 0x40 0x03 w}
                        //  power
                        sprintf(cmd_command, "i2cget 0 %X 0x03 w", device_addr);
                        // отправляем в консоль
                        // ...
                        // проверяем ответ                    
                        reply_buff[reply_len++] = 0xC1;
                        reply_buff[reply_len++] = 0x00;
                        raw_data = (0xFF & reply_buff[0]) | ((0xFF & (reply_buff[1])) << 8);
                        device_data.parameter.power_monitor.currentPower = raw_data * 1.25;
                        fprintf(stdout, "Power %4.2f\r\n", device_data.parameter.power_monitor.currentPower);
                        break;
                    }
                }
            }
            break;
            
            case dev_i2c_lis3dh:
            break;
            
        case dev_i2c_txs02324:
            break;
            
        default: 
            currentDeviceType = startDeviceType;
            break;
    }
       
    if(currentDeviceType >= (E_I2c_device)dev_i2c_txs02324) {
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

