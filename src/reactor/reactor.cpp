#include "reactor/reactor.hpp"
#include "reactor/poller_epoll.hpp"
#include "reactor/timer_wheel_mgr.hpp"

namespace reactor
{
    CReactor::CReactor()
    {
        this->m_poller = new CPollerEpoll();
        this->m_timer_mgr = new CTimerWheelMgr();

        this->m_del_poller = true;
        this->m_del_tm_mgr = true;

        this->open();
    }

    int CReactor::open()
    {
        return this->m_poller->open(1024);
    }

    CReactor::CReactor(CPoller *poller, CTimerMgr *timer_mgr)
    {
        this->m_poller = poller;
        this->m_timer_mgr = timer_mgr;

        this->open();
    }

    CReactor::~CReactor()
    {
        if (this->m_del_poller && this->m_poller)
        {
            delete this->m_poller;
            this->m_poller = nullptr;
        }

        if (this->m_del_tm_mgr && this->m_timer_mgr)
        {
            delete this->m_timer_mgr;
            this->m_timer_mgr = nullptr;
        }
    }

    CPoller * CReactor::poller()
    {
        return this->m_poller;
    }

    // poller interface
    bool CReactor::run_event_loop()
    {
        uint32_t tm = 10; // 10ms
        while(this->m_poller->run(tm))
        {
            this->m_timer_mgr->update_timer();
        }

        return true;
    }

    bool CReactor::run_event_loop_once(int tm)
    {
        if (this->m_poller->run(tm))
        {
            this->m_timer_mgr->update_timer();
            return true;
        }

        return false;
    }

    void CReactor::end_event_loop()
    {
        this->m_poller->stop();
    }

    
    int CReactor::add_event(CEventHandler *event_handler, uint32_t event_mask)
    {
        return this->m_poller->add_event(event_handler, event_mask);
    }

    int CReactor::del_event(CEventHandler *event_handler, uint32_t event_mask)
    {
        return this->m_poller->del_event(event_handler, event_mask);
    }

    int CReactor::mod_event(CEventHandler *event_handler, uint32_t event_mask)
    {
        return this->m_poller->mod_event(event_handler, event_mask);
    }

    //
    void CReactor::regist_notify(CEventHandler *notify)
    {
        return this->m_poller->regist_notify(notify);
    }

    void CReactor::unregist_notify()
    {
        this->m_poller->unregist_notify();
    }

    int CReactor::notify(CMbuf_ptr &mbuf)
    {
        return this->m_poller->notify(mbuf);
    }
    
    int  CReactor::pop_front(CMbuf_ptr &msg)
    {
        return this->m_poller->pop_front(msg);
    }
    
    int  CReactor::push_back(CMbuf_ptr &msg)
    {
        return this->m_poller->push_back(msg);
    }                                                                                                                       
    
    int CReactor::regist_timer(CEventHandler *handler, int period, int repeat) // repeat = 0 uninfine
    {
        return this->m_timer_mgr->add_timer(handler, period, repeat);
    }
    
    int CReactor::unrigist_timer(int timer_id)
    {
        return this->m_timer_mgr->del_timer(timer_id);
    }
}
