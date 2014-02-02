#ifndef X_EVENT_HPP
#define X_EVENT_HPP

#include <climits>
#include <map>
#include <vector>
#include <unordered_map>

#include <xcb/xcb.h>

#include "core/event.hpp"
#include "core/connection.hpp"
#include "gen/protos.hpp"

#define MAX_PRIORITY UINT32_MAX

namespace xpp {

namespace event {

namespace dispatcher {

class sink {
  public:
    virtual ~sink(void) {}
    template<typename Event> void dispatch(const Event & e);
};

}; // namespace dispatcher

template<typename ... Events>
class sink;

template<typename Event>
class sink<Event> : virtual public dispatcher::sink {
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
void dispatcher::sink::dispatch(const Event & e)
{
  dynamic_cast<xpp::event::sink<Event> *>(this)->handle(e);
}

template<typename ... Extensions>
class registry
  : virtual public xpp::dispatcher::event::x
  , virtual public Extensions::event_dispatcher ...
  , virtual protected xpp::xcb::type<xcb_connection_t * const>
{
  public:
    typedef unsigned int priority;

    explicit
    registry(const xpp::connection<Extensions ...> & c)
      : Extensions::event_dispatcher(c) ...
      , m_c(c)
    {}

    virtual
    operator xcb_connection_t * const(void) const
    {
      return m_c;
    }

    bool
    dispatch(xcb_generic_event_t * event) const
    {
      return dispatch<xpp::extension::x, Extensions ...>(event);
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
    typedef std::multimap<priority, dispatcher::sink *> priority_map;

    const xpp::connection<Extensions ...> & m_c;
    std::unordered_map<uint8_t, priority_map> m_dispatchers;

    template<typename Event, typename Extension>
    struct opcode_getter {
      uint8_t operator()(const xpp::connection<Extensions ...> & c)
      {
        return Event::opcode(static_cast<const Extension &>(c));
      }
    };

    template<typename Event>
    struct opcode_getter<Event, void> {
      uint8_t operator()(const xpp::connection<Extensions ...> & c)
      {
        return Event::opcode();
      }
    };

    template<typename Event>
    uint8_t opcode(void) const
    {
      return opcode_getter<Event, typename Event::extension>()(m_c);
    }

    template<typename Extension>
    bool
    dispatch(xcb_generic_event_t * event) const
    {
      typedef const typename Extension::event_dispatcher & dispatcher;
      return static_cast<dispatcher>(*this)(*this, event);
    }

    template<typename Extension, typename Next, typename ... Rest>
    bool
    dispatch(xcb_generic_event_t * event) const
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

    void attach(priority p, dispatcher::sink * d, uint8_t opcode)
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
    detach(priority p, dispatcher::sink * d, uint8_t opcode)
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

#endif // X_EVENT_HPP
