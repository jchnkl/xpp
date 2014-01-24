#ifndef XPP_EXTENSION_HPP
#define XPP_EXTENSION_HPP

#include <iostream>
#include <xcb/xcb.h>
#include "type.hpp"

namespace xpp {

namespace extension {

template<xcb_extension_t * ID>
class generic
  : virtual public xpp::xcb::type<xcb_connection_t * const>
{
  public:
    virtual ~generic(void)
    {}

    virtual
    const xcb_query_extension_reply_t &
    operator*(void) const
    {
      return *m_extension;
    }

    virtual
    const xcb_query_extension_reply_t * const
    operator->(void) const
    {
      return m_extension;
    }

    virtual
    operator const xcb_query_extension_reply_t * const(void) const
    {
      return m_extension;
    }

    virtual
    void get_data(void)
    {
      m_extension = xcb_get_extension_data(*this, ID);
    }

    virtual
    void prefetch_data(void)
    {
      xcb_prefetch_extension_data(*this, ID);
    }

  private:
    // The result must not be freed.
    // This storage is managed by the cache itself.
    const xcb_query_extension_reply_t * m_extension = NULL;
}; // class generic

}; // namespace extension

}; // namespace xpp

#endif // XPP_EXTENSION_HPP
