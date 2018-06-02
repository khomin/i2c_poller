/*
 * DbConnection.cpp
 *
 *  Created on: Jan 11, 2018
 *      Author: khomin
 */
#include  <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../inc/dbConnection.h"
#include "postgresql/libpq-fe.h"

DbConnection::DbConnection()
{
    this->dbSettings.dbConn = PQsetdbLogin(m_dbhost.c_str(), 
            std::to_string(m_dbport).
            c_str(), nullptr, nullptr, m_dbname.c_str(), 
            m_dbuser.c_str(), m_dbpass.c_str());

    this->dbSettings.dbHostName = m_dbhost.c_str();
    this->dbSettings.dbName = m_dbhost.c_str();
    this->dbSettings.port =  to_string(m_dbport);
    this->dbSettings.dbName = m_dbname.c_str(), 
    this->dbSettings.dbUser = m_dbuser.c_str();
    this->dbSettings.dbPass = m_dbpass.c_str();
    if(!isConnecting()) {
        exeptConnectError(this->dbSettings.dbConn);
    }
}

int DbConnection::getDeviceIdFromName(std::string name_device) {
    char query_buffer[256] = {0};
    int id = 0;
    sprintf(query_buffer, "SELECT i2c_device_id, i2c_device_brief, i2c_device_addr FROM i2c_device WHERE i2c_device_name = '%s';", name_device.c_str());
    if(isConnecting()) {
        PGresult *res = PQexec(this->dbSettings.dbConn, query_buffer);
        fprintf(stdout, "SQL quety:%s\r\n", query_buffer);
        ExecStatusType exeResult = PQresultStatus(res);
        if(exeResult != PGRES_TUPLES_OK) {
            fprintf(stderr, "ERROR:%s\r\n",  PQerrorMessage(this->dbSettings.dbConn));
        } else {
            if(PQnfields(res) >0) {
                id = atoi(PQgetvalue(res, 0, 0));
                fprintf(stdout, "id sensor %d\r\n", id);        
            }
        }
    }
    return id;
}

int DbConnection::getDeviceAddrFromName(std::string name_device) {
    char query_buffer[256] = {0};
    int addr = 0;
    sprintf(query_buffer, "SELECT i2c_device_addr FROM i2c_device WHERE i2c_device_name = '%s';", name_device.c_str());
    
    if(isConnecting()) {
        PGresult *res = PQexec(this->dbSettings.dbConn, query_buffer);
        fprintf(stdout, "SQL quety:%s\r\n", query_buffer);
        ExecStatusType exeResult = PQresultStatus(res);
        if(exeResult != PGRES_TUPLES_OK) {
            fprintf(stderr, "ERROR:%s\r\n",  PQerrorMessage(this->dbSettings.dbConn));
        } else {
            if(PQnfields(res) >0) {
                addr = atoi(PQgetvalue(res, 0, 0));
                fprintf(stdout, "addr sensor %x\r\n", addr);        
            }
        }
    }
    return addr;
}

bool DbConnection::insertData(S_insertData data) {
    bool result = false;
    int device_id = 0;
    char quere_buf[512] = {0};
    char tBuff[512] = {0};
    // получить id имени типа
    // сформировать данные по типу
    switch(data.device_type) {
        case dev_i2c_tmp112: 
            fprintf(stderr, "Db:i2c dev type %s\r\n",
                i2c_Dev_typeName.tmp112.name_text.c_str());
            device_id = getDeviceIdFromName(i2c_Dev_typeName.tmp112.name_text);
            strcpy((char*)tBuff, "INSERT INTO i2c_data(i2c_data_device_id, i2c_data_json) VALUES (%d, '{\"temp\":%4.2f}'::json);");
            sprintf((char*)quere_buf, tBuff, device_id, data.parameter.temp.temp);
            break;
            
        case dev_i2c_lis3dh: 
            fprintf(stderr, "Db:i2c dev type %s\r\n", 
                i2c_Dev_typeName.lis3dh.name_text.c_str());
            device_id = getDeviceIdFromName(i2c_Dev_typeName.lis3dh.name_text);
            strcpy((char*)tBuff, "INSERT INTO i2c_data(i2c_data_device_id, i2c_data_json) VALUES (%d, '{\"x\":%4.2f,\"y\":%4.2f,\"z\":%4.2f,\"velocityX\":%4.2f,\"velocityY\":%4.2f,\"velocityZ\":%4.2f,\"isInited\":%d}'::json);");
            sprintf((char*)quere_buf, tBuff, device_id, data.parameter.accel.x, data.parameter.accel.y, data.parameter.accel.z,
                    data.parameter.accel.velocityX, data.parameter.accel.velocityY, data.parameter.accel.velocityZ,
                    data.parameter.accel.isInited);
        break;
        case dev_i2c_ina260: 
            fprintf(stderr, "Db:i2c dev type %s\r\n",
                i2c_Dev_typeName.ina260.name_text.c_str());
            device_id = getDeviceIdFromName(i2c_Dev_typeName.ina260.name_text);
            strcpy((char*)tBuff, "INSERT INTO i2c_data(i2c_data_device_id, i2c_data_json) VALUES (%d, '{\"currentPower\":%4.2f,\"currentFlowing\":%4.2f,\"voltage\":%4.2f}'::json);");
            sprintf((char*)quere_buf, tBuff, device_id,
                    data.parameter.power_monitor.currentPower,
                    data.parameter.power_monitor.currnetFlowing,
                    data.parameter.power_monitor.voltage);
        break;
        case dev_i2c_txs02324:             
            fprintf(stderr, "Db:i2c dev type %s\r\n", 
                i2c_Dev_typeName.txs02324.name_text.c_str());
            device_id = getDeviceIdFromName(i2c_Dev_typeName.txs02324.name_text);
            strcpy((char*)tBuff, "INSERT INTO i2c_data(i2c_data_device_id, i2c_data_json) VALUES (%d,'{\"simCardPowerIsUp\":%d,\"currentSimSlot\":%d}'::json);");
            sprintf((char*)quere_buf, tBuff, device_id, data.parameter.sim_switcher.cardPowerIsUp,
                    device_id, data.parameter.sim_switcher.currentSimSlot);
        break;
        case dev_i2c_unknow:
            fprintf(stderr, "Db:i2c dev type %s\r\n",
                i2c_Dev_typeName.unknow.name_text.c_str());
        default: break;
    };
    
    if((device_id != 0) && (strlen(quere_buf))) {
        // записать данные в бд    
        PGresult *res = PQexec(this->dbSettings.dbConn, quere_buf);
        fprintf(stdout, "SQL quety:%s\r\n", quere_buf);
        ExecStatusType exeResult = PQresultStatus(res);
        if(exeResult != PGRES_COMMAND_OK) {
            fprintf(stderr, "ERROR:%s\r\n",  PQerrorMessage(this->dbSettings.dbConn));
        } else {
            fprintf(stdout, "Inser ok\r\n");
            result = true;
        }
    }
    return result;
}

bool DbConnection::isConnecting() {
    if((PQstatus(this->dbSettings.dbConn) != CONNECTION_OK ) 
            && (PQsetnonblocking(dbSettings.dbConn, 1) != 0 )) {
       fprintf(stderr, "Connection to database failed: %s\r\n", 
               PQerrorMessage(this->dbSettings.dbConn));
       return false;
    }
    fprintf(stdout, "Connection to database Ok\r\n");
    return true;
}

void DbConnection::exeptConnectError(PGconn *pConn) {
	PQfinish(pConn);
	exit(1);
}

bool DbConnection::disconnect() {
    fprintf(stdout, "Connection closed\r\n");
//    delete dbSettings.dbConn;
    return true;
}

DbConnection::~DbConnection() {
	// TODO Auto-generated destructor stub
}

