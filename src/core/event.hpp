#ifndef XPP_EVENT_HPP
#define XPP_EVENT_HPP

#include <memory> // shared_ptr
#include <xcb/xcb.h> // xcb_generic_event_t
#include "type.hpp"

namespace xpp {

namespace event {

template<int OpCode, typename Event>
class generic
  : virtual protected xpp::xcb::type<xcb_connection_t * const>
{
  public:
    generic(xcb_connection_t * const c, xcb_generic_event_t * event)
      : m_c(c)
      , m_event(reinterpret_cast<Event *>(event))
    {}

    virtual
    ~generic(void) {}

    virtual
    operator xcb_connection_t * const(void) const
    {
      return m_c;
    }

    virtual
    operator Event &(void) const
    {
      return *m_event;
    }

    virtual
    const Event &
    operator*(void) const
    {
      return *m_event;
    }

    virtual
    Event * const
    operator->(void) const
    {
      return m_event.get();
    }

    static const int opcode = OpCode;

  protected:
    xcb_connection_t * m_c;
    std::shared_ptr<Event> m_event;
}; // class generic

}; // namespace event

}; // namespace xpp

// }; }; // xpp::event

#endif // XPP_EVENT_HPP
