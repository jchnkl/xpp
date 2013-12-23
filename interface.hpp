#ifndef X_INTERFACE_HPP
#define X_INTERFACE_HPP

#include <climits>
#include <vector>

#define MAX_PRIORITY UINT32_MAX

namespace x {

namespace interface {

namespace event {

typedef std::vector<std::pair<unsigned int, int>> priorities;

class dispatcher {
  public:
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
    virtual void attach(const priorities &, dispatcher *) = 0;
    virtual void detach(const priorities &, dispatcher *) = 0;
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
