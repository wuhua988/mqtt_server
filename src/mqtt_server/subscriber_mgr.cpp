#include "mqtt_server/subscriber_mgr.hpp"

namespace reactor
{

    void CTopicNode::update_retain_msg(CMbuf_ptr &retain_msg)
    {
	m_retian_msg_buf = retain_msg;
    }

    void CTopicNode::clean_retain_msg()
    {
	m_retian_msg_buf = nullptr;
    }

    CMbuf_ptr & CTopicNode::retain_msg()
    {
	return m_retian_msg_buf;
    }
    // end of cmbuf

    CONTEXT_SET & CTopicNode::client_context()
    {
	return m_subcriber_clients;
    }

    int CTopicNode::add_client(CMqttClientContext_ptr &cli_context)
    {
	if (m_subcriber_clients.find(cli_context) == m_subcriber_clients.end())
	{
	    m_subcriber_clients.insert(cli_context);
	    return 0;
	}

	LOG_DEBUG("Add client context has already existed.");
	return -1;
    }

    int CTopicNode::del_client(CMqttClientContext_ptr &cli_context)
    {
	auto it = m_subcriber_clients.find(cli_context);

	if (it == m_subcriber_clients.end())
	{
	    LOG_DEBUG("Del cli_context is not exist");
	    return -1;
	}

	m_subcriber_clients.erase(it);

	return 0;
    }

    void CTopicNode::print()
    {
	uint32_t i = 1;
	for (auto it = m_subcriber_clients.begin(); it != m_subcriber_clients.end(); it++, i++)
	{
	    LOG_DEBUG("[%d] %s", i, (*it)->client_id().c_str());
	}
    }


    int CSubscriberMgr::add_client_context(std::string str_topic_name, CMqttClientContext_ptr cli_context)
    {

	auto it = m_topic_mgr.find(str_topic_name);

	if (it == m_topic_mgr.end()) // topic is not exists
	{
	    CTopicNode_ptr topic_node = make_shared<CTopicNode>();

	    topic_node->add_client(cli_context);

	    m_topic_mgr[str_topic_name] = topic_node;
	}
	else
	{
	    it->second->add_client(cli_context);

	    // CTopicNode_ptr
	    CMbuf_ptr retain_msg = it->second->retain_msg();
	    auto mqtt_conn = cli_context->mqtt_connection();

	    // send retain msg
	    if ( (retain_msg.get() != nullptr) && (mqtt_conn != nullptr) )
	    {
		cli_context->add_send_msg(retain_msg);
		return mqtt_conn->put(retain_msg);
	    }
	}

	return 0;
    }

    int CSubscriberMgr::del_client_context(std::string &str_topic_name, CMqttClientContext_ptr & cli_context)
    {
	auto it = m_topic_mgr.find(str_topic_name);

	if (it != m_topic_mgr.end())
	{
	    it->second->del_client(cli_context);

	    return 0;
	}

	LOG_DEBUG("del client context not find client_id[%s] in topic [%s]",
		cli_context->client_id().c_str(), str_topic_name.c_str());

	return -1;
    }

    int CSubscriberMgr::find_client_context(std::string &str_topic_name, CONTEXT_SET &clients_set)
    {
	auto it = m_topic_mgr.find(str_topic_name);

	if (it != m_topic_mgr.end())
	{
	    clients_set =  it->second->client_context();
	    return 0;
	}

	LOG_DEBUG("No client on topic [%s]", str_topic_name.c_str());

	return -1;
    }

    int CSubscriberMgr::publish(std::string &str_topic_name, CMbuf_ptr &mbuf, CMqttPublish &publish_msg)
    {
	LOG_TRACE_METHOD(__func__);

	// store msg
	// MSG_MEM_STORE->add_msg(mbuf);

	bool no_sub_client = false;
	auto it = m_topic_mgr.find(str_topic_name);
	if (it == m_topic_mgr.end())
	{
	    no_sub_client = true;
	    // add topic name
	    CTopicNode_ptr topic_node = make_shared<CTopicNode>();
	    m_topic_mgr[str_topic_name] = topic_node;
	    LOG_DEBUG("No subscriber find here");
	}

	it = m_topic_mgr.find(str_topic_name); 

	// update retain msg
	CMqttFixedHeader fixed_header = publish_msg.fixed_header();

	if (fixed_header.retain_flag())
	{
	    CMbuf_ptr publish_retain_msg = make_shared<CMbuf>(mbuf->length());
	    publish_retain_msg->copy(mbuf->read_ptr(), mbuf->length());
	    publish_retain_msg->msg_id(mbuf->msg_id(), false); // not regist to mem db

	    // set retain flag
	    FixHeaderFlag *header_flag = (FixHeaderFlag *)publish_retain_msg->read_ptr();
	    header_flag->bits.retain = 1;


	    LOG_DEBUG("Publish msg retain flag is set, msg_id %ld", publish_retain_msg->msg_id());
	    if (publish_msg.payload().size() > 0)
	    {
		LOG_DEBUG("Update retain msg for topic [%s], buf len %d", 
					str_topic_name.c_str(), publish_retain_msg->length());

		it->second->update_retain_msg(publish_retain_msg);
	    }
	    else
	    {
		LOG_DEBUG("Clean retain msg for topic [%s]", str_topic_name.c_str());
		it->second->clean_retain_msg();
	    }
	}

	// end of retain msg

	if (no_sub_client)
	{
	    return -1;
	}

	int count = 0;
	CONTEXT_SET &client_context_set = it->second->client_context();
	
	LOG_DEBUG("Subscribe clients size %d", (uint32_t)client_context_set.size());

	for (auto it = client_context_set.begin(); it != client_context_set.end(); it++)
	{
	    // it mean client_context object
	    auto mqtt_conn = (*it)->mqtt_connection();
	    if (mqtt_conn != nullptr)
	    {        
		count++;
		mqtt_conn->put(mbuf);    
	    }
	    else
	    {
		LOG_DEBUG("Client Context may offline now [%s]", (*it)->client_id().c_str());
	    }

	    // only deal publish msg 
	    if ((*it).get() == nullptr)
	    {
		LOG_DEBUG("Client context not has valid ptr");
	    }
	    else
	    {
		(*it)->add_send_msg(mbuf);
	    }
	}

	return count;
    }

    void CSubscriberMgr::print()
    {
	int i = 1;

	LOG_DEBUG("\n");
	LOG_DEBUG("------- CSubscriberMgr -------------");
	for (auto it = m_topic_mgr.begin(); it != m_topic_mgr.end(); it++, i++)
	{
	    LOG_DEBUG("[%d] %s", i, it->first.c_str());
	    LOG_DEBUG("------------------------------------");
	    it->second->print();
	    LOG_DEBUG("------------------------------------");  
	}
	LOG_DEBUG("-------- CSubscriberMgr end ----------\n");  

    }


    std::unordered_map<std::string,CTopicNode_ptr>  & CSubscriberMgr::topic_mgr()
    {
	return m_topic_mgr;
    }


} // end of namespace


