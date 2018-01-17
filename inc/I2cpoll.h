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
#include "../inc/global.h"
#include "DbConnection.h"

using namespace std;

class I2c_poll {
public:
    I2c_poll();
    virtual ~I2c_poll();
    void* pollExect();
          
private:
        
    DbConnection * db = new DbConnection();

    E_I2c_device startDeviceType = dev_i2c_tmp112;
    const E_I2c_device endTypeDeviceType = dev_i2c_tmp112;
    int currentDeviceType;

    std::string getDeviceNameOfType(E_I2c_device typeDev);

    bool parceReply(S_insertData* data, char *repply_buff, int len);
    
};

#endif /* SRC_I2CPOLL_H_ */
