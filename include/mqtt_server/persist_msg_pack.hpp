//
//  persist_msg_pack.hpp
//  mqtt_server
//
//  Created by davad.di on 7/27/15.
//
//

#ifndef mqtt_server_persist_msg_pack_hpp
#define mqtt_server_persist_msg_pack_hpp

#include "mqtt_server/subscriber_mgr.hpp"
#include "mqtt_server/client_id_db.hpp"


enum class CHUNK_TYPE 
{
    GLOBAL_INFO = 1,
    DB_MSG = 2,
    CLIENT_MSG = 3,
    TOPIC_INFO = 4
};

namespace reactor
{
    class CPersistMsgPack
    {
    public:
        
        CPersistMsgPack(std::string file_name);
        
        ~CPersistMsgPack();

        int store(bool force_flush = false);
	int store_db_info();
	int store_db_msg();
	int store_client_info();
	int store_retain_msg();

	int restore(); 
	int restore_db_info(uint8_t *chunk_buf, uint32_t len);
	int restore_db_msg(uint8_t *chunk_buf, int len);
	int restore_client_info(uint8_t *chunk_buf, uint32_t len);
	int restore_retain_msg(uint8_t *chunk_buf, uint32_t len);

	int write_chunk_info(CHUNK_TYPE type, uint32_t chunk_len);
	int write(void *buf, uint32_t len);

	void print_detail();

	CMbuf_ptr restore_mbuf(msgpack::object &obj);
    private:
	int open(std::string str_file_name, const char *mode);

    protected:
        std::string     m_file_name;
        FILE *m_db_file = nullptr;
	std::unordered_map<uint64_t, CMbuf_ptr>     m_tmp_msg_db;  // for store msg and restore
	
	bool   m_print_detail;
    };
    

} // end of namespace

#endif
