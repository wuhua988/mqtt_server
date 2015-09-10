#include "http_server.hpp"
#include "tcp_client.hpp"
#include "reactor/reactor.hpp"

using namespace reactor;

int main(int argc, char *argv[])
{
    CLoggerMgr logger("log4cplus_debug.properties");

    http::HttpServer http_server(9090);

    http_server.open();

    http_server.svc();

    return 0;

    TCPClient client(CReactor::instance());

    client.connect("127.0.0.1", 9090);

    CReactor::instance()->run_event_loop();

    return 0;
}
