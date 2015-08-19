//
//  persist_msg_pack.cpp
//  mqtt_server
//
//  Created by davad.di on 7/27/15.
//
//

#include "mqtt_server/persist_msg_pack.hpp"
#include "common/msg_mem_store.hpp"
#include "mqtt_server/mqtt_context.hpp"
#include "mqtt_server/xml_config.hpp"

namespace reactor
{
    int CPersistMsgPack::open(std::string str_file_name, const char *mode)
    {
        m_file_name = str_file_name;
        m_db_file = fopen(m_file_name.c_str(), mode);
        if (m_db_file == nullptr)
        {
            LOG_ERROR("Open file %s failed. errno %d, %s", m_file_name.c_str(), errno, strerror(errno));
            return -1;
        }
        
        // write file header
        
        return 0;
    }
    
    CPersistMsgPack::CPersistMsgPack(std::string file_name)
    : m_file_name(file_name), m_print_detail(false)
    {
    }
    
    void CPersistMsgPack::print_detail()
    {
        m_print_detail = true;
    }
    
    CPersistMsgPack::~CPersistMsgPack()
    {
        if (m_db_file != nullptr)
        {
            fclose(m_db_file);
        }
    }
    
    int CPersistMsgPack::restore_db_info(uint8_t *buf, uint32_t len)
    {
        
        msgpack::unpacked unpack;
        msgpack::unpack(&unpack, (const char *)buf,len);
        
        CMsgMemStore *mem_store = MSG_MEM_STORE;
        unpack.get().convert(mem_store);
        
        if (m_print_detail)
        {
            MSG_MEM_STORE->print();
        }
        
        LOG_DEBUG("\t Restore db info finised.");
        return 0;
    }
    
    int CPersistMsgPack::store_db_info()
    {
        CMsgMemStore *mem_store = MSG_MEM_STORE;
        msgpack::sbuffer sbuf;
        
        msgpack::pack(sbuf, *mem_store);
        uint32_t chunk_len = sbuf.size();
        
        ERROR_RETURN(this->write_chunk_info(CHUNK_TYPE::GLOBAL_INFO, chunk_len), -1);
        ERROR_RETURN(this->write((uint8_t *)sbuf.data(), sbuf.size()), -1);
        
        LOG_DEBUG("\t Store db info finised.");
        return 0;
    }
    
    int CPersistMsgPack::store_db_msg()
    {
        auto msg_db = MSG_MEM_STORE->msg_db();
        
        for (auto it = msg_db.begin(); it != msg_db.end(); it++)
        {
            CMbuf *mbuf = it->second;
            msgpack::sbuffer sbuf;
            msgpack::pack(sbuf, *mbuf);
            uint32_t chunk_len = sbuf.size();
            
            ERROR_RETURN(this->write_chunk_info(CHUNK_TYPE::DB_MSG, chunk_len), -1);
            ERROR_RETURN(this->write((uint8_t *)sbuf.data(), sbuf.size()), -1);
        }
        
        LOG_DEBUG("\t Store db msg finished. num [%d]", (uint32_t)msg_db.size());
        
        return 0;
    }
    
    CMbuf_ptr CPersistMsgPack::restore_mbuf(msgpack::object &obj)
    {
        CMbuf_tmp tmp;
        obj.convert(&tmp);
        
        int buf_len = tmp.m_write_pos - tmp.m_read_pos;
        uint64_t msg_id = tmp.m_msg_id;
        
        CMbuf_ptr mbuf =  make_shared<CMbuf>(buf_len);
        mbuf->msg_id(msg_id);
        mbuf->copy((const uint8_t*)(tmp.m_data.ptr + tmp.m_read_pos), buf_len);
        
        return mbuf;
    }
    int CPersistMsgPack::restore_db_msg(uint8_t *buf, int len)
    {
        msgpack::unpacked unpack;
        msgpack::unpack(&unpack, (const char *)buf,len);
        
        msgpack::object obj = unpack.get();
        
        CMbuf_ptr mbuf = this->restore_mbuf(obj);
        
        uint64_t msg_id = mbuf->msg_id();
        if (msg_id != 0)
        {
            mbuf->regist_mem_store(MSG_MEM_STORE);
            m_tmp_msg_db[msg_id] = mbuf;
        }
        
        return 0;
    }
    
    int CPersistMsgPack::restore_client_info(uint8_t *buf, uint32_t len)
    {
        msgpack::unpacked unpack;
        msgpack::unpack(&unpack, (const char *)buf,len);
        
        CMqttConnection *con = nullptr;
        CMqttClientContext_ptr context = make_shared<CTMqttClientContext>(con);
        
        unpack.get().convert(context.get());
        
        std::string client_id = context->client_id();
        CLIENT_ID_CONTEXT->add_client_context(client_id, context);
        
        // topic subscriber
        //std::vector<CTopic>  & subcribe_topics()
        auto sub_topics = context->subcribe_topics();
        for (auto it = sub_topics.begin(); it != sub_topics.end(); it++)
        {
            SUB_MGR->add_client_context(it->topic_name(), context);
        }
        
        // convert msg_id -> CMbuf_ptr
        auto msgs = context->send_msg_ids();
        for (auto it = msgs.begin(); it != msgs.end(); it++)
        {
            uint64_t msg_id = *it;
            auto pos = m_tmp_msg_db.find(msg_id);
            if (pos == m_tmp_msg_db.end())
            {
                LOG_DEBUG("Msg id(%ld) cann't find.", msg_id);
                continue;
            }
            
            context->add_send_msg(pos->second);
        }
        
        LOG_DEBUG("\t Restore client_id [%s] topic_num [%d], msgs [%d]",
                  client_id.c_str(), (int32_t)sub_topics.size(), (uint32_t)msgs.size());
        
        if (m_print_detail)
        {
            context->print();
        }
        
        return 0;
    }
    
    int CPersistMsgPack::store_client_info()
    {
        auto client_infos = CLIENT_ID_CONTEXT->client_context();
        
        uint32_t i = 0;
        for (auto it = client_infos.begin(); it != client_infos.end(); it++)
        {
            std::string client_id = it->first;
            CMqttClientContext_ptr context = it->second;
            
            // check timeout
            if (context->mqtt_connection() != nullptr)
            {
                std::time_t now = std::time(nullptr);
                std::time_t last_msg_in = context->mqtt_connection()->last_msg_time();
                
                if (now - last_msg_in > CONFIG->get_max_idle_timeout())
                {
                    context->mqtt_connection()->handle_close();
                }
            }
            
            if (context->clean_session())
            {
                continue;
            }
            ++i;
            
            context->prepare_store(); // list -> vector
            
            msgpack::sbuffer sbuf;
            msgpack::pack(sbuf, *context.get());
            uint32_t chunk_len = sbuf.size();
            
            ERROR_RETURN(this->write_chunk_info(CHUNK_TYPE::CLIENT_MSG, chunk_len), -1);
            ERROR_RETURN(this->write((uint8_t *)sbuf.data(), sbuf.size()), -1);
        }
        
        LOG_DEBUG("\t Store client info finised. num [%d]", i);
        
        return 0;
    }
    
    int CPersistMsgPack::store_retain_msg()
    {
        std::map<std::string, CMbuf *> map_retain_msg;
        
        auto topic_mgr = SUB_MGR->topic_mgr();
        for (auto it = topic_mgr.begin(); it != topic_mgr.end(); it++)
        {
            std::string topic_name = it->first;
            CTopicNode_ptr &topic_node = it->second;
            
            if (topic_node.get() == nullptr || topic_node->retain_msg().get() == nullptr)
            {
                // LOG_DEBUG(" No retain msg in topic [%s]", topic_name.c_str());
                continue;
            }
            
            CMbuf_ptr retain_msg = topic_node->retain_msg();
            
            if (retain_msg.get() == nullptr || retain_msg->length() <= 0)
            {
                continue;
            }
            
            map_retain_msg[topic_name] = (CMbuf *)retain_msg.get();
        }
        
        if (map_retain_msg.empty())
        {
            LOG_DEBUG("\t Store retain msg finished. num [0]");
            return 0;
        }
        
        msgpack::sbuffer sbuf;
        msgpack::packer<msgpack::sbuffer> pk(&sbuf);
        
        pk.pack_map(map_retain_msg.size());
        
        for (auto it = map_retain_msg.begin(); it != map_retain_msg.end(); it++)
        {
            pk.pack(it->first);
            pk.pack(*(it->second));
        }
        
        uint32_t chunk_len = sbuf.size();
        
        ERROR_RETURN(this->write_chunk_info(CHUNK_TYPE::TOPIC_INFO, chunk_len), -1);
        ERROR_RETURN(this->write((uint8_t *)sbuf.data(), sbuf.size()), -1);
        
        LOG_DEBUG("\t Store retain msg finished. num [%d]", (uint32_t)map_retain_msg.size());
        
        return 0;
    }
    
    int CPersistMsgPack::restore_retain_msg(uint8_t *buf, uint32_t len)
    {
        msgpack::unpacked unpack;
        msgpack::unpack(&unpack, (const char *)buf, len);
        
        msgpack::object obj = unpack.get();
        
        msgpack::object_kv*  pkv;
        msgpack::object_kv*  pkv_end;
        msgpack::object pk, pv;
        
        uint32_t num = 0;
        if(obj.via.map.size > 0)
        {
            pkv = obj.via.map.ptr;
            pkv_end = obj.via.map.ptr + obj.via.map.size;
            
            uint32_t i = 0;
            auto topic_mgr = SUB_MGR->topic_mgr();
            do
            {
                pk = pkv->key;
                pv = pkv->val;
                
                std::string topic_name;
                pk.convert(&topic_name);
                
                CMbuf_ptr mbuf = this->restore_mbuf(pv);
                
                if (m_print_detail)
                {
                    LOG_DEBUG("\t [%d] %s, id (%ld), len (%d)",
                              ++i, topic_name.c_str(), mbuf->msg_id(), mbuf->length());
                }
                
                auto it = topic_mgr.find(topic_name);
                if (it != topic_mgr.end())
                {
                    if ( it->second.get() != nullptr)
                    {
                        it->second->update_retain_msg(mbuf);
                    }
                    else
                    {
                        LOG_DEBUG("Topic [%s] topic node is nullptr", topic_name.c_str());
                    }
                }
                
                ++num;
                ++pkv;
            }
            while (pkv < pkv_end);
        }
        
        LOG_DEBUG("\t Restore retian msg finised. num [%d]", num);
        
        return 0;
    }
    
    int CPersistMsgPack::restore()
    {
        if ( access(m_file_name.c_str(), R_OK) == -1) // file is not exsits
        {
            LOG_DEBUG("Restore file is not exist");
            return 0;
        }
        
        ERROR_RETURN(this->open(m_file_name, "rb"), -1);
        
        fseek(m_db_file, 0L, SEEK_END);
        long file_size = ftell(m_db_file);
        
        if (file_size == 0)
        {
            LOG_DEBUG("Restore file is empty");
            return 0;
        }
        
        rewind(m_db_file);
        
        uint8_t tag = 0;
        uint8_t chunk_type = 0;
        uint32_t chunk_len = 0;
        
        int res = 0;
        while(1)
        {
            if (fread((void *)&tag, 1, sizeof(tag), m_db_file) != sizeof(tag))
            {
                if (feof(m_db_file)) // reached file end
                {
                    LOG_DEBUG("\t !!!!! Restore Finished (Reached file end) ");
                    break;
                }
                
                LOG_DEBUG("Read chunk tag failed. %d, %s", errno, strerror(errno));
                res =  -1;
                
                break;
            }
            
            if (tag != 0xEE)
            {
                LOG_DEBUG("Chunk tag is wrong");
                res =  -1;
                break;
            }
            
            if (fread((void *)&chunk_type, 1, sizeof(chunk_type), m_db_file) != sizeof(chunk_type))
            {
                LOG_DEBUG("Read chunk type failed. %d, %s", errno, strerror(errno));
                res = -1;
                break;
            }
            
            if (fread((void *)&chunk_len, 1, sizeof(chunk_len), m_db_file) != sizeof(chunk_len))
            {
                LOG_DEBUG("Read chunk len failed. %d, %s", errno, strerror(errno));
                res = -1;
                break;
            }
            
            static uint8_t buf[1024*1024];
            
            if (chunk_len > 1024*1024)
            {
                LOG_DEBUG("Chunk len is too large (%d), max 1M", chunk_len);
                res =  -1;
                break;
            }
            
            if (fread((void *)buf, 1, chunk_len, m_db_file) != chunk_len)
            {
                LOG_DEBUG("Read chunk buf failed. %d, %s", errno, strerror(errno));
                res = -1;
                break;
            }
            
            switch ((CHUNK_TYPE)chunk_type)
            {
                case CHUNK_TYPE::GLOBAL_INFO:
                    res = this->restore_db_info((uint8_t *)buf, chunk_len);
                    break;
                    
                case CHUNK_TYPE::DB_MSG:
                    res = this->restore_db_msg((uint8_t *)buf, chunk_len);
                    break;
                    
                case CHUNK_TYPE::CLIENT_MSG:
                    res = this->restore_client_info((uint8_t *)buf, chunk_len);
                    break;
                    
                case CHUNK_TYPE::TOPIC_INFO:
                    res = this->restore_retain_msg((uint8_t *)buf, chunk_len);
                    break;
                    
                default:
                    LOG_ERROR("Unknown chunk type %d", chunk_type);
                    res = -1;
                    break;
            }
            
            if (res == -1)
            {
                break;
            }
        }
        
        LOG_DEBUG("\t Total Restore  [%d] db msgs", (uint32_t)m_tmp_msg_db.size());
        
        uint32_t i = 0;
        if (m_print_detail)
        {
            for (auto it = m_tmp_msg_db.begin(); it != m_tmp_msg_db.end(); it++)
            {
                LOG_DEBUG("\t [%d] %ld -> %d", ++i, it->first, it->second->length());
            }
        }
        
        m_tmp_msg_db.clear();
        fclose(m_db_file);
        m_db_file = nullptr;
        
        return res;
        
    }
    
    
    int CPersistMsgPack::store(bool force_flush)
    {
        if (!force_flush) // not force flush
        {
            static uint32_t last_db_update_time = 0;
            uint32_t db_update_time = MSG_MEM_STORE->last_update_time();
            
            // don't need to update now
            if ( db_update_time <= last_db_update_time)
            {
                LOG_DEBUG("No data change so skip store");
                return 0;
            }
            
            last_db_update_time = db_update_time;
        }
        
        if (m_db_file != nullptr)
        {
            fclose(m_db_file);
        }
        
        if (this->open(m_file_name, "wb") == -1)
        {
            return -1;
        }
        
        LOG_DEBUG("CPersistMsgPack::store()");
        
        ERROR_RETURN(this->store_db_info(), -1);
        ERROR_RETURN(this->store_db_msg(), -1);
        ERROR_RETURN(this->store_client_info(), -1);
        ERROR_RETURN(this->store_retain_msg(), -1);
        
        fflush(m_db_file);
        fclose(m_db_file);
        
        m_db_file = nullptr;
        
        return 0;
    }
    
    int CPersistMsgPack::write_chunk_info(CHUNK_TYPE chunk_type, uint32_t chunk_len)
    {
        uint8_t type = (uint8_t)chunk_type;
        uint8_t tag = 0xEE;
        
        ERROR_RETURN(this->write((void *) &tag, 1), -1);
        ERROR_RETURN(this->write((void *) &type, 1), -1);
        ERROR_RETURN(this->write((void *) &chunk_len, sizeof(uint32_t)), -1);
        
        return 0;
    }
    
    int CPersistMsgPack::write(void *buf, uint32_t len)
    {
        if (fwrite(buf, 1, len, m_db_file) !=  len)
        {
            LOG_DEBUG("Write file %s failed. %d, %s", m_file_name.c_str(), errno, strerror(errno));
            return -1;
        }
        
        return 0;
    }
    
} // end of namespace

