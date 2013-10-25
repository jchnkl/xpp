#ifndef X_INTERFACE_HPP
#define X_INTERFACE_HPP

#include <vector>

namespace x {

namespace interface {

namespace event {

typedef int type;

class dispatcher {
  public:
    typedef unsigned int priority;
    typedef std::vector<std::pair<priority, type>> priority_masks;
    virtual ~dispatcher(void) {}
    template<typename H, typename E> void dispatch(H *, E *);
}; // class dispatcher

template<typename E>
class sink {
  public:
    virtual void handle(E * e) = 0;
}; // class sink

class source {
  public:
    virtual void run(void) = 0;
    virtual void insert(const dispatcher::priority_masks &, dispatcher *) = 0;
    virtual void remove(const dispatcher::priority_masks &, dispatcher *) = 0;
}; // class source

}; // namespace event

}; // namespace interface

}; // namespace x

template<typename H, typename E>
void x::interface::event::dispatcher::dispatch(H * h, E * e)
{
  try {
    dynamic_cast<x::interface::event::sink<E> &>(*h).handle(e);
  } catch (...) {}
}

#endif // X_INTERFACE_HPP
