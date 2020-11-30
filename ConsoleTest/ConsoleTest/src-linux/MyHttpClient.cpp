/*
 * MyHttpClient.cpp
 *
 *  Created on: Dec 9, 2019
 *      Author: root
 */

#include "MyHttpClient.h"

MyHttpClient::MyHttpClient() {
    // TODO Auto-generated constructor stub

}

MyHttpClient::~MyHttpClient() {
    // TODO Auto-generated destructor stub
}

void MyHttpClient::client_handler(struct mg_connection *nc, int ev, void *ev_data)
{
    struct http_message *hm = (struct http_message *) ev_data;
    struct mbuf *io = &nc->recv_mbuf;
    switch (ev)
    {
        case MG_EV_CONNECT:
        {
            if (*(int *) ev_data != 0)
            {
                fprintf(stderr, "connect() failed: %s\n", strerror(*(int *) ev_data));
                s_exit_flag = 1;
            }
            break;
        }

        case MG_EV_HTTP_REPLY:
        {
            nc->flags |= MG_F_CLOSE_IMMEDIATELY;

            std::string str_req    = std::string(hm->message.p, hm->message.len);
            std::string str_query  = std::string(hm->query_string.p, hm->query_string.len);
            std::string str_body   = std::string(hm->body.p,    hm->body.len);
            std::string str_method = std::string(hm->method.p,  hm->method.len);
            std::string str_uri    = std::string(hm->uri.p,     hm->uri.len);
            std::cout<< str_req << " \n"<< str_query <<" \n" << str_body << "\n" << str_method << "\n" <<str_uri<<std::endl;

            if (s_show_headers)
            {
                fwrite(hm->message.p, 1, hm->message.len, stdout);
            }
            else
            {
                fwrite(hm->body.p, 1, hm->body.len, stdout);
            }
            putchar('\n');
            s_exit_flag = 1;
            break;
        }
        case MG_EV_RECV:
        {

            std::string str    = std::string(io->buf, io->len);
       //     std::cout << "recv sth." << str << endl;

            struct mg_str *s;
            const int is_req = (nc->listener != NULL);
            int req_len = mg_parse_http(io->buf, io->len, hm, is_req);
            if (req_len > 0)
               //     && (s = mg_get_http_header(hm, "Transfer-Encoding")) != NULL
               //     && mg_vcasecmp(s, "chunked") == 0)
            {
            //    hm = (struct http_message *) io->buf;
                std::string str_body   = std::string(hm->body.p,    req_len); // hm->body.len
                std::cout << str_body;
            }

            /* TODO(alashkin): refactor this ifelseifelseifelseifelse */
            if ((req_len < 0 || (req_len == 0 && io->len >= MG_MAX_HTTP_REQUEST_SIZE)))
            {
              printf(("invalid request"));
              nc->flags |= MG_F_CLOSE_IMMEDIATELY;
            }
            else if (req_len == 0)
            {
              /* Do nothing, request is not yet fully buffered */
            }
            else if (hm->message.len <= io->len)
            {

            }
            break;
        }
        case MG_EV_CLOSE:
        {
            if (s_exit_flag == 0)
            {
                printf("Server closed connection\n");
                s_exit_flag = 1;
            }
            break;
        }
        default:
            break;
    }
}

int MyHttpClient::testHttpClient()
{
    struct mg_mgr mgr;
    int i;
    char url[256] = "http://172.18.10.129:8081/trace1.data";
    mg_mgr_init(&mgr, NULL);

    mg_connect_http(&mgr, client_handler, url, NULL, NULL);

    while (s_exit_flag == 0) {
      mg_mgr_poll(&mgr, 1000);
    }
    mg_mgr_free(&mgr);

    return 0;
}

int MyHttpClient::DownloadFile(const char *url)
{
    MyURL myurl;
    myurl.downloadFile(url, "data1");
    return 0;
}
