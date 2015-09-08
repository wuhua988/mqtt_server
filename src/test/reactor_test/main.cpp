#include "http_server.hpp"

int main(int argc, char *argv[])
{
    CLoggerMgr logger("log4cplus_debug.properties");

    http::HttpServer http_server(9090);

    http_server.open();

    http_server.svc();

    return 0;
}
