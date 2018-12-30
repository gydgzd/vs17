#include "stdafx.h"
#include "sql_conn_cpp.h"
#include <fstream>
/*
 * connect to database
 * there might be an exception, but won't be handled here
 * throw the exception to the call function
 */
Csql_conn_cpp::Csql_conn_cpp()   //throw(sql::SQLException)
{
	mdriver = get_driver_instance();
	m_con = NULL;
	m_stmt = NULL;
	m_prepstmt = NULL;
	m_res = NULL;
	mnIsConnected = 0;
	//while(mnIsConnected == 0)
}

/*
 * connect to MySql database
 */
int Csql_conn_cpp::sql_connect(string host, string username, string passwd)
{
	if (0 == mnIsConnected )
	{
		try
		{
			m_con = mdriver->connect(host.c_str(), username.c_str(), passwd.c_str());  //Internal IP
			mstr_host = host;
			mstr_username = username;
			mstr_passwd = passwd;
			mnIsConnected = 1;
		}
		catch (sql::SQLException &e)
		{
			throw e;
		}
		catch (...)
		{
			throw;
		}
	}
	//cout<<"sql conn OK!"<<endl;
	return 0;
}

//disconnect and prepare to quit
int Csql_conn_cpp::sql_disconn()
{
	
	if (m_res)	{
		delete m_res;
		m_res = NULL;
	}
	if (m_stmt)	{
		delete m_stmt;
		m_stmt = NULL;
	}
	if (m_prepstmt)	{
		delete m_prepstmt;
		m_prepstmt = NULL;
	}
	if (m_con)	{
		try {
			m_con->close();
		}
		catch (...) {
		//	throw;
		}
		delete  m_con;
		m_con = NULL;
	}
	mnIsConnected = 0;
    return 0;
}

int Csql_conn_cpp::sql_reconn()
{
	if (m_con != NULL)
	{
		try {
			m_con->reconnect();
			m_con->setSchema(mstr_DBname.c_str());
		}catch (sql::SQLException &e)
		{		
			throw e;
		}
	}
	return 0;
}


Csql_conn_cpp::~Csql_conn_cpp()
{
	sql_disconn();
}

int Csql_conn_cpp::setDBName(string dbname)
{
	try {
		m_con->setSchema(dbname.c_str());
		mstr_DBname = dbname;
	}
	catch (sql::SQLException &e)
	{
		throw e;
	}
	return 0;
}

/*
  execute the sql statement 
 get a result after a query when flag is not 0
 */
int Csql_conn_cpp::sql_execute( string sql_str, int flag=0)  throw(sql::SQLException)
{
	try
	{
		if (!m_con->isValid())
			this->sql_reconn();
		if(0 == flag){
			m_stmt = m_con->createStatement();
			m_stmt->execute(sql_str.c_str());
			delete m_stmt;
			m_stmt = NULL;
		}
		else{
			m_stmt = m_con->createStatement();
			m_res = m_stmt->executeQuery(sql_str.c_str());
		}
		return 0;
	}catch (sql::SQLException &e){
		throw e;
	}
	return -1;
}

int Csql_conn_cpp::freeStmtAndRes()
{
	if (m_res)
	{
		delete m_res;
		m_res = NULL;
	}
	if (m_stmt)
	{
		delete m_stmt;
		m_stmt = NULL;
	}
	if (m_prepstmt)
	{
		delete m_prepstmt;
		m_prepstmt = NULL;
	}
	return 0;
}
