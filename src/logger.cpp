#include "../inc/logger.h"
#include <stdarg.h>
#include <sys/stat.h>

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

void logger::appendToLog(const std::string format, ...)  {
	va_list args;
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
