//
//  persist.cpp
//  mqtt_server
//
//  Created by davad.di on 7/27/15.
//
//

#include "mqtt_server/persist.hpp"
#include "common/msg_mem_store.hpp"
#include "mqtt_server/mqtt_context.hpp"

namespace reactor
{
    int CPersist::write_str(std::string &str)
    {
        return write_len_buf((void *)str.c_str(), str.length());
    }
    
    int CPersist::write_len_buf(void *buf, uint32_t len)
    {
        ERROR_RETURN(this->write(&len, sizeof(uint32_t)), -1);
        ERROR_RETURN(this->write((void *)buf, len), -1);
        
        return 0;
    }
    
    /*
     int CPersist::write_buf(uint8_t *buf, uint32_t len)
     {
     // ERROR_RETURN(this->write(&len, sizeof(uint32_t)), -1);
     ERROR_RETURN(this->write(buf, len), -1);
     return len;
     }
     */
    
    int CPersist::write_uint8(uint8_t value)
    {
        ERROR_RETURN(this->write((void *)&value, sizeof(uint8_t)), -1);
        return 0;
    }
    
    
    int CPersist::write_uint64(uint64_t value)
    {
        ERROR_RETURN(this->write(&value, sizeof(uint64_t)), -1);
        return 0;
    }
    
    int CPersist::write_uint(uint32_t value)
    {
        ERROR_RETURN(this->write(&value, sizeof(uint32_t)), -1);
        
        return sizeof(value);
    }
    
    int CPersist::open(std::string str_file_name, const char *mode)
    {
        m_file_name = str_file_name;
        m_db_file = fopen(m_file_name.c_str(), mode);
        if (m_db_file == nullptr)
        {
            LOG_ERROR("Open file %s failed. errno %d, %s", m_file_name.c_str(), errno, strerror(errno));
            return -1;
        }
        
        // write file_header
        
        return 0;
    }
    
    /*
     CPersist::CPersist()
     {
     }
     */
    
    CPersist::CPersist(std::string file_name) : m_file_name(file_name)
    {
    }
    
    CPersist::~CPersist()
    {
        if (m_db_file != nullptr)
        {
            fclose(m_db_file);
        }
    }
    
    int CPersist::restore_db_info(uint8_t *buf, uint32_t len)
    {
        if (len != sizeof(uint64_t))
        {
            LOG_DEBUG("msg id msg should be uint64_t");
            return -1;
        }
        
        uint64_t last_msg_id = *((uint64_t *)buf);
        
        MSG_MEM_STORE->last_msg_id(last_msg_id);
        
        LOG_DEBUG("\t\t1. Restore db_msg_id [%ld]", last_msg_id);
        return 0;
    }
    
    int CPersist::store_db_info()
    {
        uint32_t chunk_len = sizeof(uint64_t);
        
        uint64_t last_msg_id = MSG_MEM_STORE->last_msg_id();
        ERROR_RETURN(this->write_chunk_info(CHUNK_TYPE::GLOBAL_INFO, chunk_len), -1);
        ERROR_RETURN(this->write((uint8_t *)&last_msg_id, sizeof(last_msg_id)), -1);
        
        LOG_DEBUG("\t\t1. Sotre db_msg id [%ld]", last_msg_id);
        return 0;
    }
    
    int CPersist::store_db_msg()
    {
        auto msg_db = MSG_MEM_STORE->msg_db();
        
        // uint64_t, CMbuf *
        //
        // chunk_type, chunk_len, msg_id, buf_len, buf
        //
        
        LOG_DEBUG("\t\t2. Store db msg");
        
        if (msg_db.empty())
        {
            LOG_DEBUG("\t\t\tNo db msg is here");
            return 0;
        }
        
        for (auto it = msg_db.begin(); it != msg_db.end(); it++)
        {
            uint64_t msg_id = it->first;
            CMbuf *mbuf = it->second;
            uint32_t buf_len = mbuf->length();
            
            uint32_t chunk_len = sizeof(msg_id) + buf_len + sizeof(buf_len);
            
            ERROR_RETURN(this->write_chunk_info(CHUNK_TYPE::DB_MSG, chunk_len), -1);
            ERROR_RETURN(this->write_uint64(msg_id), -1);
            ERROR_RETURN(this->write_len_buf(mbuf->read_ptr(), buf_len), -1);
        }
        
        LOG_DEBUG("\t\tTotal store [%d] msgs", (uint32_t) msg_db.size());
        
        return 0;
    }
    
    int CPersist::restore_db_msg(uint8_t *chunk_buf, int len)
    {
        uint8_t *buf = chunk_buf;
        
        // get msg id
        uint64_t msg_id = *((uint64_t *)buf);
        
        buf += sizeof(uint64_t);
        
        // get buf len
        uint32_t buf_len = *(uint32_t *)buf;
        
        buf += sizeof(uint32_t);
        
        if (buf_len != (len - sizeof(uint32_t) - sizeof(uint64_t)))
        {
            LOG_DEBUG("Get buf len wrong");
            return -1;
        }
        
        LOG_DEBUG("\t\t2. Restore db msg, msg_id [%ld], len [%d]", msg_id, buf_len);
        
        CMbuf_ptr mbuf =  make_shared<CMbuf>(buf_len);
        mbuf->copy(buf, buf_len);
        mbuf->msg_id(msg_id);
        mbuf->regist_mem_store(MSG_MEM_STORE);
        
        m_tmp_msg_db[msg_id] = mbuf;
        
        return 0;
    }
    
    int CPersist::restore_client_info(uint8_t *chunk_buf, uint32_t len)
    {
        uint8_t *buf = chunk_buf;
        int res = 0;
        
        uint32_t client_id_len = *((uint32_t *)buf);
        buf += sizeof(uint32_t);
        
        std::string client_id;
        client_id.assign((const char *)buf, client_id_len);
        
        buf += client_id_len;
        
        LOG_DEBUG("\t\t3. Restore client_id [%s]", client_id.c_str());
        
        CMqttConnection *con = nullptr;
        CMqttClientContext_ptr context = make_shared<CTMqttClientContext>(con);
        context->client_id(client_id);
        
        uint32_t topic_num = *((uint32_t *)buf);
        buf += sizeof(uint32_t);
        
        std::vector<CTopic> sub_topics;
        
        LOG_DEBUG("\t\t\tTopic num %d", topic_num);
        
        for (uint32_t i = 0; i < topic_num; i++)
        {
            ++i;
            uint32_t topic_name_len = *((uint32_t *)buf);
            buf += sizeof(uint32_t);
            
            if (topic_name_len > len)
            {
                res = -1;
                break;
            }
            
            std::string topic_name;
            topic_name.assign((const char *)buf, topic_name_len);
            buf += topic_name_len;
            
            uint8_t topic_qos = buf[0];
            buf += sizeof(uint8_t);
            
            LOG_DEBUG("\t\t\t\t[%d] %s", i, topic_name.c_str());
            
            CTopic topic(topic_name, topic_qos);
            sub_topics.push_back(topic);
            
            // add subject
            SUB_MGR->add_client_context(topic_name, context);
        }
        
        context->add_subcribe_topics(sub_topics);
        
        uint32_t msgs_num = *((uint32_t *)buf);
        buf += sizeof(uint32_t);
        
        LOG_DEBUG("\t\t\tMsg num %d", msgs_num);
        
        for (uint32_t i = 0; i < msgs_num; i++)
        {
            uint64_t msg_id = *((uint64_t *)buf);
            buf += sizeof(uint64_t);
            
            LOG_DEBUG("\t\t\t\t[%d] %ld", i+1, msg_id);
            
            auto it = m_tmp_msg_db.find(msg_id);
            if (it == m_tmp_msg_db.end())
            {
                LOG_DEBUG("Msg id(%ld) cann't find.", msg_id);
                continue;
            }
            
            context->add_send_msg(it->second);
        }
        
        CLIENT_ID_CONTEXT->add_client_context(client_id, context);
        
        LOG_DEBUG("\t\t[%s] topic num [%d], msg num [%d]",
                  client_id.c_str(), topic_num, msgs_num);
        
        return res;
    }
    
    int CPersist::store_client_info()
    {
        uint32_t chunk_len = 0;
        
        
        LOG_DEBUG("\t\t3. store client info");
        
        // std::unordered_map<std::string, CMqttClientContext_ptr>    m_client_msg;
        auto client_infos = CLIENT_ID_CONTEXT->m_client_msg;
        
        if (client_infos.empty())
        {
            LOG_DEBUG("\t\t\t No client info is here");
            return 0;
        }
        
        uint32_t i = 0;
        for (auto it = client_infos.begin(); it != client_infos.end(); it++)
        {
            //
            // chunk_type, chunk_len:
            //	    client_id, sub_topic_num, [buf_len, topic_name, qos], msg_number [msg_id, msg_id]
            //
            
            std::string client_id = it->first;
            CMqttClientContext_ptr context = it->second;
            
            if (context->clean_session())
            {
                continue;
            }
            
            ++i;
            
            long chunk_offset = ftell(this->m_db_file);
            ERROR_RETURN(this->write_chunk_info(CHUNK_TYPE::CLIENT_MSG, chunk_len), -1);
            
            chunk_len += client_id.length() + sizeof(uint32_t);
            ERROR_RETURN(this->write_str(client_id), -1);
            
            // std::vector<CTopic>  & subcribe_topics()
            auto topics = context->subcribe_topics();
            uint32_t topic_size = topics.size();
            
            chunk_len += sizeof(uint32_t);
            ERROR_RETURN(this->write_uint(topic_size), -1);
            
            LOG_DEBUG("\t\t\tTopic Number [%d]", topic_size);
            
            int i = 0;
            for (auto it1 = topics.begin(); it1 != topics.end(); it1++)
            {
                ++i;
                std::string &topic_name = it1->topic_name();
                uint8_t qos         = it1->qos();
                
                chunk_len += topic_name.length() + sizeof(uint32_t);
                ERROR_RETURN(this->write_str(topic_name), -1);
                
                chunk_len += sizeof(uint8_t);
                ERROR_RETURN(this->write_uint8(qos), -1);
                
                LOG_DEBUG("\t\t\t\t[%d] %s", i, topic_name.c_str());
            }
            
            //std::list<CMbuf_ptr> & send_msg()
            auto msgs = context->send_msg();
            uint32_t msg_num = msgs.size();
            
            chunk_len += sizeof(uint32_t);
            ERROR_RETURN(this->write_uint(msg_num), -1);
            
            LOG_DEBUG("\t\t\tSend queue msgs number [%d]", msg_num);
            
            i = 0;
            for (auto it2 = msgs.begin(); it2 != msgs.end(); it2++)
            {
                ++i;
                uint64_t msg_id = (*it2)->msg_id();
                
                // store msg_id -> mbuf
                chunk_len += sizeof(uint64_t);
                ERROR_RETURN(this->write_uint64(msg_id), -1);
                
                LOG_DEBUG("\t\t\t\t[%d] %ld",i, msg_id);
            }
            
            long last_offset = ftell(this->m_db_file);
            
            // rewirte chunk info
            fseek(this->m_db_file, chunk_offset, SEEK_SET);
            ERROR_RETURN(this->write_chunk_info(CHUNK_TYPE::CLIENT_MSG, chunk_len), -1);
            
            fseek(this->m_db_file, last_offset, SEEK_SET);
        }
        
        
        LOG_DEBUG("\t\tSummery: store [%d] cients info, all [%d]",
                  i, (uint32_t) client_infos.size());
        
        return 0;
    }
    
    int CPersist::store_retain_msg()
    {
        // chunk_type chunk_len topic_name(len + buf) msg_id retain_msg(len + buf)
        // std::unordered_map<std::string,CTopicNode_ptr>
        
        auto topic_mgr = SUB_MGR->topic_mgr();
        
        LOG_DEBUG("\t\t4. Store topic retain msg");
        
        if (topic_mgr.empty())
        {
            LOG_DEBUG("\t\t\tNo topic is here");
            return 0;
        }
        
        for (auto it = topic_mgr.begin(); it != topic_mgr.end(); it++)
        {
            std::string topic_name = it->first;
            CTopicNode_ptr &topic_node = it->second;
            
            if (topic_node.get() == nullptr || topic_node->retain_msg().get() == nullptr)
            {
                LOG_DEBUG("No retain msg in topic [%s]", topic_name.c_str());
                continue;
            }
            
            CMbuf_ptr retain_msg = topic_node->retain_msg();
            
            if (retain_msg.get() == nullptr || retain_msg->length() <= 0)
            {
                continue;
            }
            
            LOG_DEBUG("\t\t\tStore [%s] retain msg msg_id [%ld] data len [%d] ------",
                      topic_name.c_str(), retain_msg->msg_id(), retain_msg->length());
		          
            uint32_t chunk_len = topic_name.length() + sizeof(uint32_t);
            chunk_len += sizeof(uint64_t); // msg_id
            chunk_len += retain_msg->length() + sizeof(uint32_t);
            
            uint64_t msg_id = retain_msg->msg_id();
            
            ERROR_RETURN(this->write_chunk_info(CHUNK_TYPE::TOPIC_INFO, chunk_len), -1);
            ERROR_RETURN(this->write_str(topic_name), -1);
            ERROR_RETURN(this->write_uint64(msg_id), -1);
            ERROR_RETURN(this->write_len_buf(retain_msg->read_ptr(), retain_msg->length()), -1);
        }
        
        //LOG_DEBUG("-------------------------------------");
        
        return 0;
    }
    
    int CPersist::restore_retain_msg(uint8_t *chunk_buf, uint32_t len)
    {
        uint8_t *buf = chunk_buf;
        
        // get topic name
        uint32_t topic_name_len = *((uint32_t *)buf);
        if (topic_name_len > len)
        {
            LOG_DEBUG("Topic name len(%d) is larger than buf len (%d)", topic_name_len, len);
            return -1;
        }
        buf += sizeof(uint32_t);
        
        std::string topic_name;
        topic_name.assign((const char *)buf, topic_name_len);
        buf += topic_name_len;
        
        LOG_DEBUG("\t\t4. Restore [%s] retain msg", topic_name.c_str());
        
        // get msg_id
        uint64_t msg_id = *((uint64_t *)buf);
        buf += sizeof(uint64_t);
        
        // get mbuf
        uint32_t buf_len = *(uint32_t *)buf;
        buf += sizeof(uint32_t);
        
        if (buf_len != (len - (buf - chunk_buf)))
        {
            LOG_DEBUG("Get buf len wrong");
            return -1;
        }
        
        if (buf_len == 0)
        {
            LOG_DEBUG("Mbuf length is 0, something wrong");
        }
        else
        {
            CMbuf_ptr mbuf =  make_shared<CMbuf>(buf_len);
            mbuf->copy(buf, buf_len);
            
            mbuf->msg_id(msg_id);
            
            // std::unordered_map<std::string,CTopicNode_ptr>
            // add update_retain_msg(std::string topic_name, CMbuf_ptr mbuf) to SUB_MGR later
            auto topic_mgr = SUB_MGR->topic_mgr();
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
            
            LOG_DEBUG("\t\tTopic [%s] msg_id [%ld] len [%d] restore end",
                      topic_name.c_str(), msg_id, buf_len);
		          
        }
        
        
        return 0;
    }
    
    int CPersist::restore()
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
                    LOG_DEBUG("\t\t !!!!! Restore Finished (Reached file end) ");
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
        
        LOG_DEBUG("\t\tTotal Restore  [%d] db msgs", (uint32_t)m_tmp_msg_db.size());
        
        m_tmp_msg_db.clear();
        fclose(m_db_file);
        m_db_file = nullptr;
        
        return res;
        
    }
    
    
    int CPersist::store(bool force_flush)
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
        }
        
        if (m_db_file != nullptr)
        {
            fclose(m_db_file);
        }
        
        if (this->open(m_file_name, "wb") == -1)
        {
            return -1;
        }
        
        LOG_DEBUG("CPersist::store()");
        
        ERROR_RETURN(this->store_db_info(), -1);
        ERROR_RETURN(this->store_db_msg(), -1);
        ERROR_RETURN(this->store_client_info(), -1);
        ERROR_RETURN(this->store_retain_msg(), -1);
        
        fflush(m_db_file);
        
        fclose(m_db_file);
        m_db_file = nullptr;
        
        return 0;
    }
    
    int CPersist::write_chunk_info(CHUNK_TYPE chunk_type, uint32_t chunk_len)
    {
        uint8_t type = (uint8_t)chunk_type;
        uint8_t tag = 0xEE;
        
        ERROR_RETURN(this->write((void *) &tag, 1), -1);
        ERROR_RETURN(this->write((void *) &type, 1), -1);
        ERROR_RETURN(this->write((void *) &chunk_len, sizeof(uint32_t)), -1);
        
        return 0;
    }
    
    int CPersist::write(void *buf, uint32_t len)
    {
        if (fwrite(buf, 1, len, m_db_file) !=  len)
        {
            LOG_DEBUG("Write file %s failed. %d, %s", m_file_name.c_str(), errno, strerror(errno));
            return -1;
        }
        
        return 0;
    }
    
} // end of namespace

