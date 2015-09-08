#include "reactor/sig_handler.hpp"
#include <sys/signalfd.h>
#include <signal.h>

namespace reactor
{
    
    int CSigHandler::open(std::set<int> &signal_set, std::set<int> &signal_ign_set)
    {
        m_signal_set = signal_set;
        m_signal_ign_set = signal_ign_set;
        
        sigset_t mask;
        sigemptyset(&mask);
        
        for (auto it = signal_set.begin(); it != signal_set.end(); it++)
        {
            sigaddset(&mask, *it);
        }
        
        for (auto it = signal_ign_set.begin(); it != signal_ign_set.end(); it++)
        {
            sigaddset(&mask, *it);
        }
        
        if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1)
        {
            LOG_ERROR("Add signal set failed. %d, %s", errno, strerror(errno));
            return -1;
        }
        
        m_sock_handle  = signalfd(-1, &mask, SFD_NONBLOCK|SFD_CLOEXEC);
        if (m_sock_handle == -1)
        {
            LOG_ERROR("Create siganl fd faield. %d, %s", errno, strerror(errno));
            return -1;
        }
        
        LOG_DEBUG("CSigHandler::open() signalfd is %d", m_sock_handle);
        
        
        return CEventHandler::open();
    }
    
    int CSigHandler::handle_input(socket_t)
    {
        struct signalfd_siginfo fdsi;
        ssize_t res = read(m_sock_handle, &fdsi, sizeof(fdsi));
        if (res > 0)
        {
            if (m_signal_set.find(fdsi.ssi_signo) != m_signal_set.end())
            {
                if (m_reactor_ptr != nullptr)
                {
                    LOG_DEBUG("Recv signale %d, ready to exit poll", fdsi.ssi_signo);
                    
                    m_reactor_ptr->end_event_loop();
                }
            }
        }
        
        return 0;
    }
    
}


