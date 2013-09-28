#ifndef X_REQUEST_HPP
#define X_REQUEST_HPP

#include <memory>
#include <xcb/xcb.h>

namespace x {

namespace generic {

template<typename COOKIE,
         typename REPLY,
         REPLY * (*REPLY_FUN)(xcb_connection_t *, COOKIE, xcb_generic_error_t **)>
class request {
  public:
    template<typename ... COOKIE_ARGS>
    request(xcb_connection_t * const c,
            COOKIE (*cookie_fun)(xcb_connection_t *, COOKIE_ARGS ...),
            COOKIE_ARGS ... cookie_args)
      : m_c(c)
    {
      m_cookie = cookie_fun(c, cookie_args ...);
    }

    const REPLY * const operator*(void)
    {
      if (! m_reply) {
        m_reply = std::shared_ptr<REPLY>(REPLY_FUN(m_c, m_cookie, NULL));
      }
      return m_reply.get();
    }

    const REPLY * const operator->(void)
    {
      return &*(*this);
    }

  private:
    xcb_connection_t * m_c;
    COOKIE m_cookie;
    std::shared_ptr<REPLY> m_reply;
};

}; // namespace generic

}; // namespace x

#endif // X_REQUEST_HPP
