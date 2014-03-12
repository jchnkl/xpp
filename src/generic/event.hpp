#ifndef XPP_GENERIC_EVENT_HPP
#define XPP_GENERIC_EVENT_HPP

#include <memory> // shared_ptr
#include <xcb/xcb.h> // xcb_generic_event_t

namespace xpp { namespace generic {

template<typename Connection, typename Event, int OpCode>
class event
{

  public:
    template<typename C>
    event(C && c, const std::shared_ptr<xcb_generic_event_t> & event)
      : m_c(std::forward<C>(c))
      , m_event(event)
    {}

    virtual
    ~event(void) {}


    virtual
    operator const Event &(void) const
    {
      return reinterpret_cast<const Event &>(*m_event);
    }

    virtual
    const Event &
    operator*(void) const
    {
      return reinterpret_cast<const Event &>(*m_event);
    }

    virtual
    Event * const
    operator->(void) const
    {
      return reinterpret_cast<Event * const>(m_event.get());
    }

    static const int opcode = OpCode;

  protected:
    Connection m_c;
    std::shared_ptr<xcb_generic_event_t> m_event;
}; // class event

}; }; // namespace xpp::generic

#endif // XPP_GENERIC_EVENT_HPP
