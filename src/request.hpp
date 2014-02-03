#ifndef X_REQUEST_HPP
#define X_REQUEST_HPP

#include <memory>
#include <xcb/xcb.h>
#include "core/type.hpp"

namespace xpp {

namespace generic {

template<typename COOKIE,
         typename REPLY,
         REPLY * (*REPLY_FUN)(xcb_connection_t *, COOKIE, xcb_generic_error_t **)>
class request
  : virtual protected xpp::xcb::type<xcb_connection_t * const>
{
  public:
    template<typename ... CookieParameter>
    request(xcb_connection_t * const c,
            Cookie (*cookie_function)(xcb_connection_t *, CookieParameter ...),
            CookieParameter ... cookie_parameter)
      : m_c(c)
    {
      prepare(cookie_function, cookie_parameter ...);
    }

    virtual
    operator xcb_connection_t * const(void) const
    {
      return m_c;
    }

    const Reply & operator*(void)
    {
      return *this->get();
    }

    const Reply * const operator->(void)
    {
      return this->get().get();
    }

    std::shared_ptr<Reply>
    get(void)
    {
      if (! m_reply) {
        m_reply = std::shared_ptr<Reply>(ReplyFunction(m_c, m_cookie, nullptr));
      }
      return m_reply;
    }

    void reset(void)
    {
      m_reply.reset();
    }

  protected:
    xcb_connection_t * m_c;
    Cookie m_cookie;
    std::shared_ptr<Reply> m_reply;

    request(xcb_connection_t * const c)
      : m_c(c)
    {}

    template<typename ... CookieParameter>
    void
    prepare(Cookie (*cookie_function)(xcb_connection_t *, CookieParameter ...),
            CookieParameter ... cookie_parameter)
    {
      m_cookie = cookie_function(m_c, cookie_parameter ...);
    }

    xcb_connection_t * const
    connection(void)
    {
      return m_c;
    }
};

}; // namespace generic

}; // namespace xpp

#endif // X_REQUEST_HPP
