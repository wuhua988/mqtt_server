#define GETSETVAR(type, name) \
    private: \
    type name; \
public: \
    const type & get_##name() const { return name; } \
    void set_##name(const type& newval) { name = newval; }


class MyClass
{
    GETSETVAR(int,a)
    GETSETVAR(double,b)
};


int main()
{
    MyClass a;
    return 0;
}
