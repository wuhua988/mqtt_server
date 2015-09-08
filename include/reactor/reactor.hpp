#ifndef _reactor_h__
#define _reactor_h__

#include "reactor/define.hpp"
#include "common/singleton.hpp"
#include "common/mbuf.hpp"

namespace reactor
{
    class CPoller;
    class CTimerMgr;
    class CEventHandler;

    class CReactor : public CSingleton<CReactor>
    {
        public:
            CReactor();
            CReactor(CPoller *poller, CTimerMgr *timer_mgr); 
            ~CReactor();

            int open();

            CPoller* poller();

            // poller interface
            bool run_event_loop();
            void end_event_loop();

            int add_event(CEventHandler *event_handler, uint32_t event_mask);
            int del_event(CEventHandler *event_handler, uint32_t event_mask);
            int mod_event(CEventHandler *event_handler, uint32_t event_mask);

            void regist_notify(CEventHandler *notify);
            void unregist_notify();
            int notify(CMbuf_ptr &mbuf);

            int  pop_front(CMbuf_ptr &msg);
            int  push_back(CMbuf_ptr &msg);

            // timer mgr interface
            int regist_timer(CEventHandler *handler, int period, int repeat = 0); // 0 uninfine
            int unrigist_timer(int timer_id);

        protected:
            CPoller      *m_poller = nullptr;
            CTimerMgr    *m_timer_mgr = nullptr;

            bool        m_del_poller = false;
            bool        m_del_tm_mgr = false;
    };
}

#endif

