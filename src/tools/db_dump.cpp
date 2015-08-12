#include "mqtt_server/persist_msg_pack.hpp"



#include <signal.h>

extern char *optarg;
extern int optind, opterr, optopt;
#include <getopt.h>



using namespace reactor;

static struct option long_options[] = {
    { "help",           no_argument,        NULL,   'h' },
    { "db_file",        required_argument,  NULL,   'f' },
    { NULL,             0,                  NULL,    0  }
};

const char *version = "0.1";

static char short_options[] = "hf:";

void usage(int , char* argv[])
{
    fprintf(stderr, "Usage: %s -f db_file\n", argv[0]);
    fprintf(stderr, "\n\tVersion %s\n\n", version); 
    
    exit(0);
}


int main(int argc, char *argv[])
{
    opterr = 0;
    std::string str_db_file_name("dup.db");
    std::string str_log_conf("log4cplus_debug.properties");
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
                str_db_file_name = optarg;
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
    CPersistMsgPack persist(str_db_file_name);
    
    persist.print_detail();
    persist.restore();

    return 0;
}
