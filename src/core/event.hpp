#ifndef XPP_EVENT_HPP
#define XPP_EVENT_HPP

#include <memory> // shared_ptr
#include <xcb/xcb.h> // xcb_generic_event_t

namespace xpp {

namespace event {

template<int OpCode, typename Event>
class generic {
  public:
    generic(Event * event)
      : m_event(event)
    {}

    generic(xcb_generic_event_t * event)
      // : m_event(static_cast<Event *>(event))
      : m_event((Event *)(event))
    {}

    virtual
    ~generic(void) {}

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
    std::shared_ptr<Event> m_event;
}; // class generic

}; // namespace event

}; // namespace xpp

// }; }; // xpp::event

#endif // XPP_EVENT_HPP
