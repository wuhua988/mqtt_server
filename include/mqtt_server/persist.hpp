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

namespace reactor
{
    class CPersist
    {
    public:
        enum class CHUNK_TYPE
        {
            DB_MSG = 1,
            CLIENT_MSG = 2,
            TOPIC_INFO = 3
        };
        
        int open(std::string str_file_name);
        
	CPersist();
        CPersist(std::string file_name);
        
        ~CPersist();
        
        int restore();
        int store();
        int write(uint8_t *buf, uint32_t len);
        
    protected:
        std::string     m_file_name;
        FILE *m_db_file = nullptr;
        
    };
    

} // end of namespace

#endif
