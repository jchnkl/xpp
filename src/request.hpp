#ifndef X_REQUEST_HPP_NG
#define X_REQUEST_HPP_NG

#include <array>
#include <memory>
#include <cstdlib>
#include <xcb/xcb.h>
#include "signature.hpp"
#include "core/generic/connection.hpp"

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

void
check(xcb_connection_t * const c, const xcb_void_cookie_t & cookie)
{
  xcb_generic_error_t * error = xcb_request_check(c, cookie);
  if (error) {
    throw std::shared_ptr<xcb_generic_error_t>(error, std::free);
  }
}

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
    error_handler(Connection && c)
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
    template<typename ... Parameter>
    reply(Connection && c, Parameter && ... parameter)
      : m_c(std::forward<Connection>(c))
      , m_cookie(Derived::cookie(std::forward<Connection>(c),
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

    virtual
    void
    handle(const std::shared_ptr<xcb_generic_error_t> & error)
    {
      throw error;
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
        handle(std::shared_ptr<xcb_generic_error_t>(error, std::free));
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

#endif // X_REQUEST_HPP
