/*
 * I2cpoll.h
 *
 *  Created on: Jan 11, 2018
 *      Author: khomin
 */

#ifndef SRC_I2CPOLL_H_
#define SRC_I2CPOLL_H_

#include <stdio.h>
#include <string>
#include "global.h"
#include "../inc/dbConnection.h"
#include <thread>

using namespace std;

class I2cPoll {
public:
    I2cPoll();
    virtual ~I2cPoll();

    void pollExect();

private:
        
    DbConnection * db = new DbConnection();

    static const E_I2c_device startDeviceType = dev_i2c_ina260;
    static const E_I2c_device endTypeDeviceType = dev_i2c_ina260;
    int currentDeviceType;

    std::string getDeviceNameOfType(E_I2c_device typeDev);
    
    bool readWord(char *cmd_command, uint16_t *p_word);
};

#endif /* SRC_I2CPOLL_H_ */
