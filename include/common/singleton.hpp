#ifndef _SINGLETON_H__
#define _SINGLETON_H__

// can work in signal thread and after c++0x11/c++0x14
// more info can read from: http://www.cnblogs.com/liyuan989/p/4264889.html

template<typename T>
class CSingleton
{
public:
    static T * instance()
    {
        static T value;
        return &value;
    }
    
//private:
//    CSingleton();
//    ~CSingleton();
};

#endif

