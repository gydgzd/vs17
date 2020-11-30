/*
 * testMysqlclient.cpp
 *
 *  Created on: May 5, 2019
 *      Author: gyd
 */

#include "testMysqlclient.h"

testMysqlclient::testMysqlclient() {
    // TODO Auto-generated constructor stub

}

testMysqlclient::~testMysqlclient() {
    // TODO Auto-generated destructor stub
    mysqlclose();
}

int testMysqlclient::mysqlconnect(const char *host, const char *user, const char *passwd, const char *db, const int port)
{
    if (0 == mysql_library_init(0, NULL, NULL))
        cout << "mysql_library_init successful" << endl;
    else
    {
        cout << "mysql_library_init failed" << endl;
    }

    mysql_init(&m_mysql);
    if(NULL != mysql_real_connect(&m_mysql, host, user, passwd, db, port, NULL, 0))
    {
        printf("Connected to Mysql successful!\n");
    }
    else
    {
        printf("Error connecting to Mysql:%s\n", mysql_error(&m_mysql));
        return -1;
    }


    return 0;
}
/*
 * execute the strsql
 * return the result if isResNeeded = 1
 */
int testMysqlclient::mysql_execute(const char *strsql, int isResNeeded)
{
    MYSQL_RES *result;
    MYSQL_ROW sql_row;
    int res = 0;
    int num_rows = 0;
    //mysql_query(&mydata, "SET NAMES GBK"); //设置编码格式
    res = mysql_query(&m_mysql, strsql);     //return Zero for success. Nonzero if an error occurred
    if (0 == res)
    {
        result = mysql_store_result(&m_mysql);

    //  result = mysql_use_result(&mydata);//使用这句的时候，必须使用mysql_fetch_row读完全部的行，直到为NULL
        if (result)
        {
            unsigned int num_fields = mysql_num_fields(result);
            cout << "num_fields: " << num_fields << endl;

            MYSQL_FIELD *field;
        //  mysql_field_seek(result, 1);
            while ((field = mysql_fetch_field(result)))
            {
                cout << setiosflags(ios::left)<< setw(15) << field->name;
            }
            cout << endl;
            //mysql_data_seek(result, 2);
            while(NULL != (sql_row = mysql_fetch_row(result)) )//获取具体的数据
            {
                for (unsigned int i = 0; i < mysql_num_fields(result); i++)
                {
                    if (sql_row[i])
                        cout << setiosflags(ios::left)<< setw(15) << sql_row[i];
                }
                cout << endl;
            }
        }
        else // mysql_store_result return nothing
        {
            if(mysql_field_count(&m_mysql) == 0)//return an unsigned integer, the number of columns in a result set
            {
                // query does not return data
                num_rows = mysql_affected_rows(&m_mysql);
                printf("No result returned, %d rows affected.", num_rows);
            }
            else // mysql_store_result() should have returned data
            {
                printf( "Error: %s\n", mysql_error(&m_mysql));
            }
        }
        mysql_free_result(result);
    }
    else
    {
        cout << "query sql failed!" << endl;
    }
    return 0;
}
void testMysqlclient::mysqlclose()
{
    /*关闭连接*/
    mysql_close(&m_mysql);
    mysql_library_end();
    cout << "Close Mysql." << endl;
}
