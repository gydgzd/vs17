/*
 * MyHttpClient.h
 *
 *  Created on: Dec 9, 2019
 *      Author: root
 */

#ifndef MYHTTPCLIENT_H_
#define MYHTTPCLIENT_H_

#include <string>
#include <iostream>
#include "mongoose.h"
#include "MyURL.h"

using namespace std;
static int s_exit_flag = 0;
static int s_show_headers = 0;
static const char *s_show_headers_opt = "--show-headers";

class MyHttpClient {
public:
    MyHttpClient();
    virtual ~MyHttpClient();

    static void client_handler(struct mg_connection *nc, int ev, void *ev_data);
    int testHttpClient();

    int DownloadFile(const char *url);
private:
};

#endif /* MYHTTPCLIENT_H_ */
