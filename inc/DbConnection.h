/*
 * DbConnection.h
 *
 *  Created on: Jan 11, 2018
 *      Author: khomin
 */

#ifndef DBCONNECTION_H_
#define DBCONNECTION_H_

#include <libpq-fe.h>
#include <iostream>
#include <string.h>
#include <memory>
#include <mutex>
#include <vector>
#include <mutex>
#include <cctype>
#include <sstream>
#include <string>
#include <algorithm>
#include <unistd.h>

#include "../inc/global.h"

using namespace std;

class DbConnection {
public:
	DbConnection();

	virtual ~DbConnection();

	std::shared_ptr<PGconn> connection() const;

private:

	typedef struct {
		std::string dbHostName;
		std::string dbName;
		std::string dbUser;
		std::string dbPass;
		std::string port;
		PGconn *dbConn;
	}S_ConnectionSettings;

	S_ConnectionSettings dbSettings;

	std::string m_dbhost = "192.168.15.124";
	std::string m_dbname = "sensors";
	std::string m_dbuser = "postgres";
	std::string m_dbpass = "123456789";
	int m_dbport = 5432;
        
public:
   
        bool insertData(S_insertData data);
        int getDeviceIdFromName(std::string name_device);
        int getDeviceAddrFromName(std::string name_device);
        bool isConnecting();
	bool disconnect();
	void exeptConnectError(PGconn *pConn);
};

#endif /* DBCONNECTION_H_ */
