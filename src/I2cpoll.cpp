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
#include "../inc/logger.h"

static logger& loggerInstance = logger::Instance();

I2cPoll::I2cPoll() {
	db = new DbConnection();
	if(db->isConnecting()) {
		loggerInstance.appendToLog("I2cPoll: db-connected-OK\r\n");
	} else {
		loggerInstance.appendToLog("I2cPoll: db opening -ERROR\r\n");
	}
	this->currentDeviceType = startDeviceType;
}

void I2cPoll::pollExect() {
	uint8_t *p_data = NULL;
	uint16_t word = 0;
	bool res = false;
	char cmd_command[128] = {0};
	S_insertData device_data;
	int device_addr = 0;

	// формируем запрос
	switch(currentDeviceType) {

	case dev_i2c_tmp112: {
		device_addr = db->getDeviceAddrFromName(i2c_Dev_typeName.tmp112.name_text.c_str());
		if(device_addr != 0) {
			sprintf(cmd_command, "i2cget -y 0 0x%X 0x00 w\r\n", device_addr);
			res = readWord(cmd_command, &word);
			if(res) {  // проверяем ответ
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
	}
	break;

	case dev_i2c_ina260: {
		device_addr = db->getDeviceAddrFromName(i2c_Dev_typeName.ina260.name_text.c_str());
		if(device_addr != 0) {
			for(int i=0; i<3; i++) {
				switch(i) {
				// curent
				case 0:
					sprintf(cmd_command, "i2cget -y 0 0x%X 0x01 w\r\n", device_addr);
					// отправляем в консоль
					res = readWord(cmd_command, &word);
					if(res) {  // проверяем ответ
						p_data = (uint8_t*)&word;
						float raw_data;
						fprintf(stdout, "Raw u16 %x\r\n", word);
						raw_data = (0xFF & p_data[1]) | ((0xFF & (p_data[0])) << 8);
						device_data.parameter.power_monitor.currnetFlowing = raw_data * 1.25 / 1000;
						fprintf(stdout, "Current %4.2f\r\n", device_data.parameter.power_monitor.currnetFlowing);
					}
					break;
					// voltage
				case 1:
					sprintf(cmd_command, "i2cget -y 0 0x%X 0x02 w", device_addr);
					// отправляем в консоль
					res = readWord(cmd_command, &word);
					if(res) {  // проверяем ответ
						p_data = (uint8_t*)&word;
						float raw_data;
						fprintf(stdout, "Raw u16 %x\r\n", word);
						raw_data = (0xFF & p_data[1]) | ((0xFF & (p_data[0])) << 8);
						device_data.parameter.power_monitor.voltage = raw_data * 1.25 / 1000;
						fprintf(stdout, "Volage %4.2f\r\n", device_data.parameter.power_monitor.voltage);
					}
					break;

				case 2:
					//  power
					sprintf(cmd_command, "i2cget -y 0 0x%X 0x03 w\r\n", device_addr);
					// отправляем в консоль
					res = readWord(cmd_command, &word);
					if(res) {  // проверяем ответ
						p_data = (uint8_t*)&word;
						float raw_data;
						fprintf(stdout, "Raw u16 %x\r\n", word);
						raw_data = (0xFF & p_data[1]) | ((0xFF & (p_data[0])) << 8);
						device_data.parameter.power_monitor.currentPower = raw_data * 1.25;
						fprintf(stdout, "Power %4.2f\r\n", device_data.parameter.power_monitor.currentPower);
					}
					break;
				}
			}
			if(res) {
				db->insertData(device_data);
			}
		}
	}
	break;

	default :
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
}

bool I2cPoll::readWord(char *cmd_command, uint16_t *p_word) {
	int res = false;
	int word = 0;
	FILE *stream;
	// отправляем в консоль
	stream = popen(cmd_command, "r");
	fprintf(stdout, "cmd_command - %s\r\n", cmd_command);

	std::string data;
	if (stream) {
		char reply_buff[128] = {0};
		while (!feof(stream))
			if(fgets(reply_buff, sizeof(reply_buff), stream) != NULL) {
				data.append(reply_buff);
			}
		pclose(stream);
	}

	// проверяем ответ
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

I2cPoll::~I2cPoll() {
	// TODO Auto-generated destructor stub
}
