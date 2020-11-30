/*
 * sql_conn_cpp.h
 *
 *  Created on: Jul 14, 2016
 *      Author: Gyd
 */
/* Standard C++ includes */
#pragma once

#include <stdlib.h>
#include <iostream>

/*
  Include directly the different headers from cppconn/ and mysql_driver.h + mysql_util.h
  (and mysql_connection.h). This will reduce your build time!
*/
#include "mysql_connection.h"

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

#include <string>
using namespace std;

#ifndef SQL_CONN_CPP_H_
#define SQL_CONN_CPP_H_

class Csql_conn_cpp
{
private:
	sql::Driver *mdriver;
	string mstr_host;
	string mstr_username;
	string mstr_passwd;
	string mstr_DBname;
public:
    int mnIsConnected;  //whether the connection to database is open successfully:1 means connect;0 means disconnect
	sql::Connection *m_con;
	sql::Statement *m_stmt;
	sql::PreparedStatement *m_prepstmt;
    sql::ResultSet *m_res;
	string mstr_sql;
	Csql_conn_cpp() ;
    ~Csql_conn_cpp();

	int sql_connect(string host = "192.168.126.130", string username = "mysqlcluster", string passwd = "Grid2016");
	int setDBName(std::string dbname);
    int sql_execute(string , int ) throw(sql::SQLException);
	int freeStmtAndRes();
    int sql_disconn();
	int sql_reconn();
};

#endif /* SQL_CONN_CPP_H_ */
