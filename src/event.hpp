#ifndef XPP_EVENT_HPP
#define XPP_EVENT_HPP

#include <climits>
#include <map>
#include <vector>
#include <unordered_map>

#include "proto/x.hpp"

#define MAX_PRIORITY UINT32_MAX

namespace xpp {

namespace event {

class dispatcher {
  public:
    virtual ~dispatcher(void) {}
    template<typename Event> void dispatch(const Event & e);
}; // namespace dispatcher

template<typename ... Events>
class sink;

template<typename Event>
class sink<Event> : virtual public dispatcher
{
  public:
    virtual ~sink(void) {}
    virtual void handle(const Event &) = 0;
};

template<typename Event, typename ... Events>
class sink<Event, Events ...>
  : virtual public sink<Event>
  , virtual public sink<Events> ...
{};

template<typename Event>
void dispatcher::dispatch(const Event & e)
{
  dynamic_cast<xpp::event::sink<Event> *>(this)->handle(e);
}

template<typename Connection, typename ... Extensions>
class registry
  : virtual public xpp::x::event::dispatcher<Connection>
  , virtual public Extensions::template event_dispatcher<Connection> ...
  , virtual protected xpp::generic::connection<Connection>
{
  protected:
    virtual
    Connection
    get(void) const
    {
      return m_c;
    }

  public:
    typedef unsigned int priority;

    template<typename C>
    explicit
    registry(C && c)
      : Extensions::template event_dispatcher<Connection>(std::forward<C>(c)) ...
      , m_c(std::forward<C>(c))
    {}

    bool
    dispatch(const std::shared_ptr<xcb_generic_event_t> & event) const
    {
      return dispatch<xpp::x::extension, Extensions ...>(event);
    }

    template<typename Event>
    void
    operator()(const Event & event) const
    {
      try {
        for (auto & item : m_dispatchers.at(opcode<Event>())) {
          item.second->dispatch(event);
        }
      } catch (...) {}
    }

    template<typename Event, typename Next, typename ... Rest>
    void
    attach(priority p, sink<Event, Next, Rest ...> * s)
    {
      attach(p, s, opcode<Event>());
      attach<Next, Rest ...>(p, reinterpret_cast<sink<Next, Rest ...> *>(s));
    }

    template<typename Event, typename Next, typename ... Rest>
    void
    detach(priority p, sink<Event, Next, Rest ...> * s)
    {
      detach(p, s, opcode<Event>());
      detach<Next, Rest ...>(p, reinterpret_cast<sink<Next, Rest ...> *>(s));
    }

  private:
    typedef std::multimap<priority, xpp::event::dispatcher *> priority_map;

    Connection m_c;
    std::unordered_map<uint8_t, priority_map> m_dispatchers;

    template<typename C, typename Event, typename Extension>
    struct opcode_getter {
      uint8_t operator()(C && c)
      {
        return Event::opcode(static_cast<const Extension &>(std::forward<C>(c)));
      }
    };

    template<typename C, typename Event>
    struct opcode_getter<C, Event, xpp::x::extension> {
      uint8_t operator()(C &&)
      {
        return Event::opcode();
      }
    };

    template<typename Event>
    uint8_t opcode(void) const
    {
      return opcode_getter<Connection, Event, typename Event::extension>()(m_c);
    }

    template<typename Extension>
    bool
    dispatch(const std::shared_ptr<xcb_generic_event_t> & event) const
    {
      typedef const typename Extension::template event_dispatcher<Connection> & dispatcher;
      return static_cast<dispatcher>(*this)(*this, event);
    }

    template<typename Extension, typename Next, typename ... Rest>
    bool
    dispatch(const std::shared_ptr<xcb_generic_event_t> & event) const
    {
      dispatch<Extension>(event);
      return dispatch<Next, Rest ...>(event);
    }

    template<typename Event>
    void
    attach(priority p, sink<Event> * s)
    {
      attach(p, s, opcode<Event>());
    }

    void attach(priority p, xpp::event::dispatcher * d, uint8_t opcode)
    {
      m_dispatchers[opcode].emplace(p, d);
    }

    template<typename Event>
    void
    detach(priority p, sink<Event> * s)
    {
      detach(p, s, opcode<Event>());
    }

    void
    detach(priority p, xpp::event::dispatcher * d, uint8_t opcode)
    {
      try {
        auto & prio_map = m_dispatchers.at(opcode);
        const auto & prio_sink_pair = prio_map.equal_range(p);
        for (auto it = prio_sink_pair.first; it != prio_sink_pair.second; ) {
          if (d == it->second) {
            prio_map.erase(it);
            break;
          }
        }
      } catch (...) {}
    }

}; // xpp::event::source

}; // namespace event

}; // namespace xpp

#endif // XPP_EVENT_HPP
