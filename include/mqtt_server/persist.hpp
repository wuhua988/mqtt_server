//
//  persist.hpp
//  mqtt_server
//
//  Created by davad.di on 7/27/15.
//
//

#ifndef mqtt_server_persist_hpp
#define mqtt_server_persist_hpp

#include "mqtt_server/subscriber_mgr.hpp"
#include "mqtt_server/client_id_db.hpp"

#define  ERROR_RETURN(a, b) \
        if ( (a) < 0 )        \
    {                   \
	        return b;       \
	    }                   \


enum class CHUNK_TYPE 
{
    GLOBAL_INFO = 1,
    DB_MSG = 2,
    CLIENT_MSG = 3,
    TOPIC_INFO = 4
};

namespace reactor
{
    class CPersist
    {
    public:
        
        int open(std::string str_file_name);
        
	CPersist();
        CPersist(std::string file_name);
        
        ~CPersist();

	int write_str(std::string &str);
	int write_int(uint32_t value);
	int write_buf(uint8_t *buf, uint32_t len);

        int restore();
        int store();
       
	int store_db_info();
	int store_db_msg();
	int store_client_info();
	int store_retain_msg();

	int write_chunk_info(CHUNK_TYPE type, uint32_t chunk_len);
	int write(void *buf, uint32_t len);
        
    protected:
        std::string     m_file_name;
        FILE *m_db_file = nullptr;

	// std::unordered_map<uint64_t, CMbuf_ptr>     m_msg_db;  // for store msg and restore
    };
    

} // end of namespace

#endif
