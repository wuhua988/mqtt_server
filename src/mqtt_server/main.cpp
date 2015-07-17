#include "mqtt_server/tcp_server.hpp"
#include <signal.h>

int g_run = 1;

using namespace reactor;

/* Signal handler for SIGINT and SIGTERM - just stop gracefully. */
void handle_sigint(int signal)
{
    if (signal == SIGINT || signal == SIGTERM)
    {
        g_run = 0;
    }
}

int main()
{
    CLoggerMgr logger("log4cplus.properties");
    
    LOG_INFO("EPOLLIN 0x%x, EPOLLOUT 0x%x", EPOLLIN, EPOLLOUT);
    signal(SIGINT, handle_sigint);
    signal(SIGTERM, handle_sigint);
    
    // ignore sigpipe
    signal(SIGPIPE, SIG_IGN);
    
    TCPServer server;
    server.open();
    
    server.loop();
}
