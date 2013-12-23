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
    template<typename Handler, typename Event>
      void dispatch(Handler *, Event *);
}; // class dispatcher

template<typename Event>
class sink {
  public:
    virtual ~sink(void) {}
    virtual void handle(Event * e) = 0;
}; // class sink

class source {
  public:
    virtual ~source(void) {}
    virtual void run(void) = 0;
    virtual void attach(const priorities &, dispatcher *) = 0;
    virtual void detach(const priorities &, dispatcher *) = 0;
}; // class source

class container {
  public:
    virtual ~container(void) {}
    virtual dispatcher * const at(const unsigned int &) const = 0;
}; // class container

// O(1) event dispatcher
// container[window]->dispatch(e) ..
template<typename Event,
         int Type, int Priority,
         unsigned int (* Window)(Event * const)>
class adapter : public dispatcher
              , public sink<Event>
{
  public:
    adapter(source & source, const container & container)
      : m_source(source), m_container(container)
    {
      m_source.attach({ { Priority, Type } }, this);
    }

    ~adapter(void)
    {
      m_source.detach({ { Priority, Type } }, this);
    }

    void handle(Event * e)
    {
      auto * d = m_container.at(Window(e));
      d->dispatch(d, e);
    }

  private:
    source & m_source;
    const container & m_container;
}; // class adapter

// TODO: multi - adapter:
// template<typename ... ETC>
// class mult : public adapter<ETC> ...
// question: how to get multiple variadic template parameters?

// with event object
// Object object = container(window)
// for (handler : handlers) handler->handle(object, event)
namespace object {

template <typename Object>
class container {
  public:
    virtual ~container(void) {}
    virtual Object * const at(const unsigned int &) = 0;
};

template<typename Object, typename Event,
         int Type, int Priority,
         unsigned int (* Window)(Event * const)>
class adapter : public dispatcher
              , public sink<Event>
{
  public:
    adapter(source & source, container<Object> & container)
      : m_source(source)
      , m_container(container)
    {
      m_source.attach({ { Priority, Type } }, this);
    }

    ~adapter(void)
    {
      m_source.detach({ { Priority, Type } }, this);
    }

    void handle(Event * const e)
    {
      handle(m_container.at(Window(e)), e);
    }

    virtual void handle(Object * const, Event * const) = 0;

  private:
    source & m_source;
    container<Object> & m_container;
}; // class adapter

}; // namespace object

template<typename Handler, typename Event>
void dispatcher::dispatch(Handler * h, Event * e)
{
  try {
    dynamic_cast<sink<Event> &>(*h).handle(e);
  } catch (...) {}
}

}; // namespace event

}; // namespace interface

}; // namespace x

#endif // X_INTERFACE_HPP
