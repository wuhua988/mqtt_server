#include "mqtt_server/tcp_server.hpp"
#include <signal.h>

extern char *optarg;
extern int optind, opterr, optopt;
#include <getopt.h>

int g_stop_flag = 0;

using namespace reactor;

/* Signal handler for SIGINT and SIGTERM - just stop gracefully. */
void handle_sigint(int signal)
{
    if (signal == SIGINT || signal == SIGTERM)
    {
        g_stop_flag = 1;
    }
}

#define version "1.0"

static struct option long_options[] = {
    { "help",           no_argument,        NULL,   'h' },
    { "server_ip",      required_argument,  NULL,   's' },
    { "port",           required_argument,  NULL,   'p' },
    { "log_conf",       required_argument,  NULL,   'f' },
    { "thread_num",     required_argument,  NULL,   'n' },
    { NULL,             0,                  NULL,    0  }
};

static char short_options[] = "hs:p:f:n:";

void usage(int , char* argv[])
{
    fprintf(stderr, "Usage: %s [-s server_bind_ip -p server_port -f log_conf_file -n thread_num]\n", argv[0]);
    fprintf(stderr, "\n\tVersion %s\n\n", version); 
    fprintf(stderr, "\t -s default: 0.0.0.0\n");
    fprintf(stderr, "\t -p default: 5050\n");
    fprintf(stderr, "\t -f default: log4cplus.properties\n");
    fprintf(stderr, "\t -n default: 1, for reserved\n\n");
            
    exit(0);
}


int main(int argc, char *argv[])
{
    opterr = 0;
    std::string str_server_ip("0.0.0.0");
    std::string str_log_conf("log4cplus.properties");
    
    uint16_t    server_port = 5050;
    uint32_t    thread_num  = 1; // used later
    
    for (;;)
    {
        int c; // value;
        c = getopt_long(argc, argv, short_options, long_options, NULL);
        if (c == -1)
        {
            /* no more options */
            break;
        }
        
        switch (c)
        {
            case 's':
                str_server_ip = optarg;
                break;
                
            case 'p':
                server_port = atoi(optarg);
                break;
                
            case 'f':
                str_log_conf = optarg;
                break;
                
            case 'n':
                thread_num =  atoi(optarg);
                break;
                
            case '?':
            case 'h':
                usage(argc, argv);
                break;
                
            default:
                usage(argc, argv);
                break;
        }
    }
    
    CLoggerMgr logger(str_log_conf.c_str());
    CSockAddress server_addr(str_server_ip, server_port);

    LOG_INFO("Server will start at [%s:%d], thread_num [%d]....", 
		    str_server_ip.c_str(), server_port, thread_num);
    
    signal(SIGINT, handle_sigint);
    signal(SIGTERM, handle_sigint);
    
    // ignore sigpipe
    signal(SIGPIPE, SIG_IGN);
    
    TCPServer server;
    server.open(server_addr);
    
    server.loop(&g_stop_flag);
    
    return 0;
}
