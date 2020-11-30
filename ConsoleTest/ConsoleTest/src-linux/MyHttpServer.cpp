/*
 * MyHttpServer.cpp
 *
 *  Created on: Jun 12, 2019
 *      Author: gyd
 */

//#include <src/MyHttpServer.h>
#include "MyHttpServer.h"
//namespace MyClass {


static const char *s_http_port = "8000";
static struct mg_serve_http_opts s_http_server_opts;

void SendHttpRsp(mg_connection *connection, std::string rsp)
{
    mg_printf(connection, "%s", "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n");
    mg_printf_http_chunk(connection,  rsp.c_str());
    mg_send_http_chunk(connection, "", 0);
}
void HandleHttpEvent(mg_connection *connection, http_message *hm)
{
    std::string str_req    = std::string(hm->message.p, hm->message.len);
    std::string str_query  = std::string(hm->query_string.p, hm->query_string.len);
    std::string str_body   = std::string(hm->body.p,    hm->body.len);
    std::string str_method = std::string(hm->method.p,  hm->method.len);
    std::string str_uri    = std::string(hm->uri.p,     hm->uri.len);
    std::cout<< str_req << " \n"<< str_query <<" \n" << str_body << "\n" << str_method << "\n" <<str_uri<<std::endl;

    std::string retMsg = "";

    /*
    std::map<std::string, ReqHandler>::iterator it = s_handler_map.find(url);
    if (it != s_handler_map.end())
    {
        ReqHandler handle_func = it->second;
        printf("find url %s\n", url.c_str());
        handle_func(url, body, connection, &HttpServer::SendHttpRsp);
    }
    */
    //
    if (mg_vcmp(&hm->uri, "/") == 0 ) // index page
    {
        //mg_serve_http(connection, http_req, s_server_option);
    }
    else if (mg_vcmp(&hm->uri, "/ready") == 0 )
    {
        retMsg = "{\"errcode\": 0,\"errmsg\":\"ready.\"}";
        SendHttpRsp(connection, retMsg);
    }
    else if (mg_vcmp(&hm->uri, "/setParameter") == 0 )
    {
        char port[8] = "";
        mg_get_http_var(&hm->body, "n1", port, sizeof(port));
        retMsg = "{\"errcode\": 0,\"errmsg\":\"setParameter\"}";
        SendHttpRsp(connection, retMsg);
        printf("get port = %s\n", port);

    }
    else if (mg_vcmp(&hm->uri, "/api/sum") == 0 )
    {
        //
        char n1[100], n2[100];
        char buf[500];
        double result;

        /* Get form variables */
        mg_get_http_var(&hm->body, "n1", n1, sizeof(n1));
        mg_get_http_var(&hm->body, "n2", n2, sizeof(n2));

        /* Compute the result and send it back as a JSON object */
        result = strtod(n1, NULL) + strtod(n2, NULL);
        sprintf(buf, "%f", result);
        SendHttpRsp(connection, buf);
    }
    else
    {
        mg_printf(connection, "%s",
                "HTTP/1.1 501 Not Implemented\r\n"
                "Content-Length: 0\r\n\r\n");
    }
}

static void http_handler(struct mg_connection *nc, int ev, void *ev_data)
{
    switch (ev)
    {
        case MG_EV_ACCEPT:
        {
            char addr[32];
            mg_sock_addr_to_str(&nc->sa, addr, sizeof(addr), MG_SOCK_STRINGIFY_IP | MG_SOCK_STRINGIFY_PORT);
            printf("%p: Connection from %s\r\n", nc, addr);
            break;
        }
        case MG_EV_HTTP_REQUEST:
        {

            http_message *http_req = (http_message *)ev_data;
            HandleHttpEvent(nc, http_req);

            mg_serve_http(nc, http_req, s_http_server_opts);

            struct http_message *hm = (struct http_message *) ev_data;
            std::string str_req    = std::string(hm->message.p, hm->message.len);
            std::string str_query  = std::string(hm->query_string.p, hm->query_string.len);
            std::string str_body   = std::string(hm->body.p,    hm->body.len);
            std::string str_method = std::string(hm->method.p,  hm->method.len);
            std::string str_uri    = std::string(hm->uri.p,     hm->uri.len);
            std::cout<< str_req << " \n"<< str_query <<" \n" << str_body << "\n" << str_method << "\n" <<str_uri<<std::endl;

            char addr[32];
            mg_sock_addr_to_str(&nc->sa, addr, sizeof(addr), MG_SOCK_STRINGIFY_IP | MG_SOCK_STRINGIFY_PORT);

            mg_send_response_line(nc, 200, "Content-Type: text/html\r\n Connection: close");
            mg_printf(nc, "\r\n<h1>Hello, %s!</h1>\r\n You asked for %.*s\r\n", addr, (int) hm->uri.len, hm->uri.p);
            nc->flags |= MG_F_SEND_AND_CLOSE;
            break;
        }
        case MG_EV_CLOSE:
        {
            printf("%p: Connection closed\r\n", nc);
            break;
        }
    }
}
static void tcp_handler(struct mg_connection *nc, int ev, void *ev_data)
{
  struct mbuf *io = &nc->recv_mbuf;

  switch (ev) {
    case MG_EV_RECV:
      // This event handler implements simple TCP echo server
      mg_send(nc, io->buf, io->len);  // Echo received data back
      mbuf_remove(io, io->len);      // Discard data from recv buffer
      break;
    default:
      break;
  }
}
MyHttpServer::MyHttpServer() {
    // TODO Auto-generated constructor stub

}

MyHttpServer::~MyHttpServer() {
    // TODO Auto-generated destructor stub
}

int MyHttpServer::testHttp()
{
    struct mg_mgr mgr;
    struct mg_connection *nc;

    mg_mgr_init(&mgr, NULL);
    printf("Starting web server on port %s\n", s_http_port);
    nc = mg_bind(&mgr, s_http_port, http_handler);
    if (nc == NULL) {
      printf("Failed to create listener\n");
      return 1;
    }

    // Set up HTTP server parameters
    mg_set_protocol_http_websocket(nc);
    s_http_server_opts.document_root = ".";  // Serve current directory
    s_http_server_opts.enable_directory_listing = "yes";

    for (;;) {
      mg_mgr_poll(&mgr, 1000);
    }
    mg_mgr_free(&mgr);
    return 0;
}
int MyHttpServer::testTcpServer()
{
    struct mg_mgr mgr;
    struct mg_connection *c;

    mg_mgr_init(&mgr, NULL);
    char port[] = "3401";
    if ((c = mg_bind(&mgr, port, tcp_handler)) == NULL)
    {
      fprintf(stderr, "mg_bind(%s) failed\n", port);
      exit(EXIT_FAILURE);
    }
//    mg_set_protocol_socks(c);

    printf("Starting socks5 proxy server on %s\n", port);
    for (;;) {
      mg_mgr_poll(&mgr, 1000);
    }
    mg_mgr_free(&mgr);

    return 0;
}

//} /* namespace MyClass */
