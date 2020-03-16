#include "stdafx.h"
#include "getDate.h"
#include "sql_conn_cpp.h"  //my sql class
/*
 * return the date and time,correct to second
 */
string getLocalTime()
{ 
	char tmp[32]={};
    time_t t = time(0);
    string date_str="";
#ifdef __LINUX
	strftime( tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S",localtime(&t) );
#endif
#ifdef WINVER
	tm timeinfo;
	localtime_s(&timeinfo, &t);
	strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", &timeinfo);
#endif
	date_str=tmp;

    return date_str;
}
/*
 * get time from mysql server
 */
string getSqlTime()
{
	string date_str = "";
	string dbname = "websiteman";
	string sql = "select date_format(now(),'%Y-%m-%d %H:%i:%s') ;";
	try
	{
	   Csql_conn_cpp  ss;
	   ss.sql_connect("10.1.122.140");
	   ss.setDBName(dbname);
	   ss.sql_execute(sql,1);  //put result into ss.res-> and there might be an exception here either
	   if (ss.m_res->next())
	   {
		   date_str = ss.m_res->getString(1).c_str() ;  //getString()
	   }
	}
	catch(sql::SQLException &e)
	{
		date_str = getLocalTime();    // get local time when failed
	}
	return date_str;
}

/*
 * return like yyyy-mm-dd
 */
string getYMD()
{
	char tmp[32] = {};
	time_t t = time(0);
	string date_str = "";
#ifdef __LINUX
	strftime(tmp, sizeof(tmp), "%Y-%m-%d", localtime(&t));
#endif
#ifdef WINVER
	tm timeinfo;
	localtime_s(&timeinfo, &t);
	strftime(tmp, sizeof(tmp), "%Y-%m-%d", &timeinfo);
#endif
	date_str = tmp;

	return date_str;

}
string getYear()
{
	char tmp[32] = {};
	time_t t = time(0);
	string date_str = "";
#ifdef __LINUX
	strftime(tmp, sizeof(tmp), "%Y", localtime(&t));
#endif
#ifdef WINVER
	tm timeinfo;
	localtime_s(&timeinfo, &t);
	strftime(tmp, sizeof(tmp), "%Y", &timeinfo);
#endif
	date_str = tmp;

	return date_str;
}