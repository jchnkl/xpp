#ifndef XPP_ITERABLE_HPP
#define XPP_ITERABLE_HPP

#include <cstdlib> // size_t

namespace xpp {

// interface for object types to be used with an itertor

template<typename Type>
class iterable {
  public:
    virtual ~iterable(void) {}
    virtual void operator=(Type) = 0;
}; // class iterable

template<>
class iterable<void> {
  public:
    virtual ~iterable(void) {}
    static std::size_t size_of(void);
    virtual void operator=(void * const) = 0;
};

};

#endif // XPP_ITERABLE_HPP
