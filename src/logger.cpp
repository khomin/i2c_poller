#include "../inc/logger.h"
#include <stdarg.h>
#include <sys/stat.h>
#include <time.h>

logger::logger() {
}

logger::~logger() {
}

void logger::init(const std::string fileName) {
	this->logFileName = fileName;

	if (mkpath(this->logFileDir) == true) {
		logPath = logFileDir + logFileName;
	} else {
		logPath = logFileName;
	}
}

void logger::appendToLog(std::string format, ...)  {
	va_list args;

	time_t rawtime;
	struct tm * timeinfo;
	char buffer[120] = {0};

	time (&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(buffer,sizeof(buffer)," %d-%m-%Y %I:%M:%S ", timeinfo);
	std::string str_date(buffer);
	for(uint8_t i=0; i<sizeof(buffer); i++){
		if(buffer[i] != '\0') {
			format.push_back(buffer[i]);
		}
	}

	va_start(args, format);

	file = fopen(logPath.c_str(), "a");
	if(file != NULL) {
		vfprintf(file, format.c_str(), args);
		fclose(file);
	}
	va_end(args);
}

bool logger::mkpath( std::string path ) {
	bool bSuccess = false;
	int nRC = ::mkdir( path.c_str(), 0775 );
	if( nRC == -1 )
	{
		switch( errno )
		{
		case ENOENT:
			//parent didn't exist, try to create it
			if( mkpath( path.substr(0, path.find_last_of('/')) ) )
				//Now, try to create again.
				bSuccess = 0 == ::mkdir( path.c_str(), 0775 );
			else
				bSuccess = false;
			break;
		case EEXIST:
			//Done!
			bSuccess = true;
			break;
		default:
			bSuccess = false;
			break;
		}
	} else {
		bSuccess = true;
	}
	return bSuccess;
}
