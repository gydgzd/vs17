/*
 * testMysqlclient.h
 *
 *  Created on: May 5, 2019
 *      Author: gyd
 */

#ifndef TESTMYSQLCLIENT_H_
#define TESTMYSQLCLIENT_H_

#include <stdio.h>
#include <iostream>     // std::cout, std::hex, std::endl
#include <iomanip>      // std::setiosflags
#include <string>
#include <mysql.h>

using namespace std;

class testMysqlclient {
public:
    testMysqlclient();
    virtual ~testMysqlclient();

    int mysqlconnect(const char *host, const char *user, const char *passwd, const char *db, const int port=3306);
    int mysql_execute(const char *strsql, int isResNeeded);


    void mysqlclose();

private:
    MYSQL m_mysql;

};

#endif /* TESTMYSQLCLIENT_H_ */
