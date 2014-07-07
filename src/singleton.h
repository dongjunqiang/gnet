#ifndef GNET_SINGLETON_H_
#define GNET_SINGLETON_H_

namespace gnet {

namespace wrapper {

template<class T>
class SingletonWrapper : public T
{
public:
    static bool destroyed_;
    ~SingletonWrapper()
    {
        destroyed_ = true;
    }
};

template<class T>
bool SingletonWrapper< T >::destroyed_ = false;

}

template <typename T>
class Singleton
{
protected:
    Singleton() {}

    static void use(T const &) {}

public:
    static T& GetInstance()
    {
        static wrapper::SingletonWrapper<T> instance;
        use(instance);
        return static_cast<T&>(instance);
    }

    static T* Instance()
    {
        return &GetInstance();
    }
};

}

#endif

