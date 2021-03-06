#include "alfe/main.h"

#ifndef INCLUDED_REFERENCE_H
#define INCLUDED_REFERENCE_H

template<class T> class Reference : private Handle
{
public:
    Reference() { }
    template<class C, typename... Args> static Reference create(Args&&... args)
    {
        Reference r(Handle::create<Body<C>>(std::forward<Args>(args)...));
        return r;
    }
    T* operator->() { return body()->t(); }
    T& operator*() { return *body()->t(); }
private:
    class BaseBody : public Handle::Body
    {
    public:
        virtual T* t() = 0;
    };
    template<class C> class Body : public BaseBody
    {
    public:
        template<typename... Args> Body(Args&&... args)
          : _c(std::forward<Args>(args)...) { }
        T* t() { return &_c; }
        C _c;
    };
    BaseBody* body() { return as<BaseBody>(); }
    Reference(const Handle& other) : Handle(other) { }
};

#endif // INCLUDED_REFERENCE_H
