#include "mqtt_server/tcp_server.hpp"
#include "mqtt_server/tcp_client.hpp"
#include "reactor/poller_epoll.hpp"

#include "mqtt_server/xml_config.hpp" 
#include <signal.h>

extern char *optarg;
extern int optind, opterr, optopt;
#include <getopt.h>


//int g_stop_flag = 0;

using namespace reactor;

/* Signal handler for SIGINT and SIGTERM - just stop gracefully. */
/*
move sig handle to epoll loop
void handle_sigint(int signal)
{
    if (signal == SIGINT || signal == SIGTERM)
    {
        g_stop_flag = 1;
    }
}
 */

#define version "1.1.1"

static struct option long_options[] = {
    { "help",           no_argument,        NULL,   'h' },
    { "conf_file",        required_argument,  NULL,   'f' },
    { NULL,             0,                  NULL,    0  }
};

static char short_options[] = "hf:";

void usage(int , char* argv[])
{
    fprintf(stderr, "Usage: %s -f config_file\n", argv[0]);
    fprintf(stderr, "\n\tVersion %s\n\n", version); 
    
    exit(0);
}


int main(int argc, char *argv[])
{
    opterr = 0;
    std::string str_server_ip("0.0.0.0");
    std::string str_conf_file_name("setting.xml");
    std::string str_db_file_name;

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
            case 'f':
                str_conf_file_name = optarg;
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

    ERROR_RETURN(CONFIG->open(str_conf_file_name), -1);
    
    str_server_ip   = CONFIG->get_server_listen_ip();
    server_port	    = CONFIG->get_server_listen_port();
    thread_num	    = CONFIG->get_thread_number();
    str_db_file_name = CONFIG->get_db_file_name();

    CONFIG->print();

    // CLoggerMgr logger(str_log_conf.c_str()); -> CONFIG->open()
    CPollerEpoll poller_epoll;

    if (poller_epoll.open() == -1)
    {
        LOG_ERROR("Epoll open faild. %s", strerror(errno));
        return -1;
    }

    // mqtt client
    CSockAddress server_addr(str_server_ip, server_port);
    LOG_INFO("Server will start at [%s:%d], thread_num [%d]....", 
			str_server_ip.c_str(), server_port, thread_num);

    std::string str_client_id	       = CONFIG->get_parent_user_name();
    std::string str_parent_topic_name  = CONFIG->get_parent_topic_name();
    std::string str_parent_server_addr = CONFIG->get_parent_server_ip();
    uint16_t	parent_server_port     = CONFIG->get_parent_server_port(); 

    TCPClient client((reactor::CPoller *)&poller_epoll,str_parent_topic_name, str_client_id);
    CSockAddress addr(str_parent_server_addr, parent_server_port);
    client.open((void *)&addr);
    
    //  client start finished
    
    TCPServer server(str_db_file_name, &poller_epoll); // db file name
    server.open(server_addr);
    server.loop();
    
    client.stop();

    return 0;
}
