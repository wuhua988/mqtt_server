#ifndef _reactor_poller_h_
#define _reactor_poller_h_

#include "reactor/define.hpp"


namespace reactor
{
    class CEventHandler;

    class CPoller
    {
    public:
        CPoller()
	{
	    LOG_TRACE_METHOD(__func__); 
	};

        virtual ~CPoller()
	{
	    LOG_TRACE_METHOD(__func__); 
	};
        
        virtual bool run(int32_t tm) = 0; 
        
        virtual int add_event(CEventHandler *event_handler, uint32_t event_mask) = 0;
        virtual int del_event(CEventHandler *event_handler, uint32_t event_mask) = 0;
        virtual int mod_event(CEventHandler *event_handler, uint32_t event_mask) = 0;
        
    protected:
        
        handle_t                                    	m_poller_handle;
        std::unordered_map<socket_t, CEventHandler *>    m_map_event_handlers;
        /*
         public:
         void increaseConnection()
         {
         this->_total_connection++;
         }
         
         void decreaseConnection()
         {
         if (this->_total_connection > 0)
         {
         this->_total_connection--;
         }
         }
         
         uint32_t totalConnection()
         {
         return this->_total_connection;
         }
         
         uint32_t totalRecvSize()
         {
         return this->_total_recv_size;
         }
         
         void totalRecvSize(uint32_t size)
         {
         this->_total_recv_size += size;
         }
         
         uint32_t totalSendSize()
         {
         return this->_total_send_size;
         }
         
         void totalSendSize(uint32_t size)
         {
         this->_total_send_size += size;
         }
         
         void increaseSendTimes()
         {
         this->_total_send_times++;
         }
         
         uint32_t totalSendTimes()
         {
         return this->_total_send_times;
         }
         
         void increaseRecvTimes()
         {
         this->_total_recv_times++;
         }
         
         uint32_t totalRecvTimes()
         {
         return this->_total_recv_times;
         }
         
         private:
         // stat related struct
         uint32_t	_total_connection = 0;
         uint32_t	_total_recv_size = 0;
         uint32_t	_total_send_size = 0;
         uint32_t	_total_recv_times = 0;
         uint32_t	_total_send_times = 0;
         uint32_t	_total_recv_queue = 0;
         */
    };
}


#endif
