#ifndef XPP_GENERIC_REQUEST_HPP
#define XPP_GENERIC_REQUEST_HPP

#include <array>
#include <memory>
#include <cstdlib>
#include <xcb/xcb.h>
#include "signature.hpp"

#define REPLY_TEMPLATE \
  typename Reply, \
  typename Cookie, \
  Reply *(&ReplyFunction)(xcb_connection_t *, Cookie, xcb_generic_error_t **)

#define REPLY_SIGNATURE \
  xpp::generic::signature<Reply *(xcb_connection_t *, \
                                  Cookie, \
                                  xcb_generic_error_t **), \
                          ReplyFunction>

#define REPLY_COOKIE_TEMPLATE \
  typename ... CookieParameter, \
  Cookie(&CookieFunction)(CookieParameter ...)

#define REPLY_COOKIE_SIGNATURE \
  xpp::generic::signature<Cookie(CookieParameter ...), CookieFunction>

namespace xpp { namespace generic {

template<typename ... Types>
struct error_handler;

template<typename Dispatcher>
class error_handler<Dispatcher>
{
  public:
    template<typename Connection>
    error_handler(Connection && c)
      : m_dispatcher(static_cast<Dispatcher &>(std::forward<Connection>(c)))
    {}

    void
    operator()(const std::shared_ptr<xcb_generic_error_t> & error) const
    {
      m_dispatcher(error);
    }

  protected:
    Dispatcher m_dispatcher;
};

template<>
class error_handler<void>
{
  public:
    template<typename Connection>
    error_handler(Connection &&)
    {}

    void
    operator()(const std::shared_ptr<xcb_generic_error_t> & error) const
    {
      throw error;
    }
};

template<typename Connection, typename Dispatcher>
class error_handler<Connection, Dispatcher>
  : public std::conditional<std::is_convertible<Connection, Dispatcher>::value,
                            error_handler<Dispatcher>,
                            error_handler<void>>::type
{
  public:
    typedef typename std::conditional<
                              std::is_convertible<Connection, Dispatcher>::value,
                              error_handler<Dispatcher>,
                              error_handler<void>>::type
                                base;
    using base::base;
};

template<typename Connection, typename Dispatcher>
void
check(Connection && c, const xcb_void_cookie_t & cookie)
{
  xcb_generic_error_t * error =
    xcb_request_check(std::forward<Connection>(c), cookie);
  if (error) {
    error_handler<Connection, Dispatcher>(std::forward<Connection>(c))(
        std::shared_ptr<xcb_generic_error_t>(error, std::free));
  }
}

struct checked_tag {};
struct unchecked_tag {};

template<typename ... Types>
class reply;

template<typename Derived,
         typename Connection,
         typename Check,
         REPLY_TEMPLATE,
         REPLY_COOKIE_TEMPLATE>
class reply<Derived,
            Connection,
            Check,
            REPLY_SIGNATURE,
            REPLY_COOKIE_SIGNATURE>
{
  public:
    template<typename C, typename ... Parameter>
    reply(C && c, Parameter && ... parameter)
      : m_c(std::forward<C>(c))
      , m_cookie(Derived::cookie(std::forward<C>(c),
                                 std::forward<Parameter>(parameter) ...))
    {}

    operator bool(void)
    {
      return m_reply.operator bool();
    }

    const std::shared_ptr<Reply> &
    get(void)
    {
      if (! m_reply) {
        m_reply = get(Check());
      }
      return m_reply;
    }

    template<typename ... Parameter>
    static
    Cookie
    cookie(Parameter && ... parameter)
    {
      return CookieFunction(std::forward<Parameter>(parameter) ...);
    }

  protected:
    Connection m_c;
    Cookie m_cookie;
    std::shared_ptr<Reply> m_reply;

    std::shared_ptr<Reply>
    get(checked_tag)
    {
      xcb_generic_error_t * error = nullptr;
      auto reply = std::shared_ptr<Reply>(ReplyFunction(m_c, m_cookie, &error),
                                          std::free);
      if (error) {
        static_cast<Derived &>(*this).handle(
            std::shared_ptr<xcb_generic_error_t>(error, std::free));
      }
      return reply;
    }

    std::shared_ptr<Reply>
    get(unchecked_tag)
    {
      return std::shared_ptr<Reply>(ReplyFunction(m_c, m_cookie, nullptr),
                                    std::free);
    }
};

}; }; // namespace xpp::generic

#endif // XPP_GENERIC_REQUEST_HPP
