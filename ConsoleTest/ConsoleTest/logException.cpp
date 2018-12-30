#include "stdafx.h"
#include <fstream>
#include <sstream>
#include <string.h> 
#include "sql_conn_cpp.h"  // my sql class
#include <time.h>
#ifdef WINVER
#include <direct.h>        // _mkdir
#endif // WINVER
string getLocalDate();
int logException(sql::SQLException &e, const char* file, const char* func, const int& line)
{
	//open log file
	_mkdir("./log");
	//string fullPath = "./log"+logFileName;
	ofstream ofs("log/log.txt", ios::app);
	if (ofs.fail())
	{
		cerr << "Failed to open log file: " << strerror(errno) << endl;
		return -1;
	}
	string mytime = getLocalDate();
	ofs << mytime << "  ";
	ofs << "# ERR: SQLException in " << file;
	ofs << "(" << func << ") on line " << line << endl;
	ofs << "                     # ERR: " << e.what();
	ofs << " (MySQL error code: " << e.getErrorCode();
	//	ofs << ", SQLState: " << e.getSQLState() << " )" << endl;
	ofs << endl;
	ofs.close();

	return 0;
}

string getLocalDate()
{
	time_t t = time(0);
	tm tmp_time;
	string date_str = "";
	char tmp[32] = {};
	localtime_s(&tmp_time, &t);
	strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", &tmp_time);
	date_str = tmp;
	//puts( tmp );
	return date_str;
}