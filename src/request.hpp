#ifndef X_REQUEST_HPP_NG
#define X_REQUEST_HPP_NG

#include <iostream> // std::ostream
#include <array>
#include <memory>
#include <cstdlib>
#include <xcb/xcb.h>
#include "signature.hpp"
#include "core/generic/connection.hpp"
// #include "core/type.hpp"
// #include "gen/xproto-stub.hpp"

/*
#define COOKIE_TEMPLATE \
  typename Cookie, \
  typename ... CookieParameter, \
  Cookie(&CookieFunction)(CookieParameter ...)

#define COOKIE_SIGNATURE \
  xpp::generic::signature<Cookie(CookieParameter ...), CookieFunction>

#define VOID_COOKIE_TEMPLATE \
  typename ... CookieParameter, \
  xcb_void_cookie_t(&CookieFunction)(CookieParameter ...)

#define VOID_COOKIE_SIGNATURE \
  xpp::generic::signature<xcb_void_cookie_t(CookieParameter ...), \
                          CookieFunction>
  */

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

// template<typename T>
// struct is_callable {
// private:
//     typedef char(&yes)[1];
//     typedef char(&no)[2];
// 
//     struct Dummy {};
//     struct Fallback { void operator()(); };
//     struct Derived : std::conditional<! std::is_fundamental<T>::value,
//                                       T,
//                                       Dummy>::type,
//                      Fallback { };
// 
//     template<typename U, U> struct Check;
// 
//     template<typename>
//     static yes test(...);
// 
//     template<typename C>
//     static no test(Check<void (Fallback::*)(), &C::operator()>*);
// 
// public:
//     static const bool value = sizeof(test<Derived>(0)) == sizeof(yes);
// };

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
      : m_dispatcher(static_cast<const Dispatcher &>(std::forward<Connection>(c)))
    {}

    void
    handle(const std::shared_ptr<xcb_generic_error_t> & error)
    {
      std::cerr << "ERROR_HANDLER handling error" << std::endl;
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
    handle(const std::shared_ptr<xcb_generic_error_t> & error)
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

// class error_handler {
//   public:
//     virtual void handle(std::shared_ptr<xcb_generic_error_t> & error) = 0;
// };

struct checked_tag {};
struct unchecked_tag {};

template<typename ... Types>
class reply;

template<typename Connection, REPLY_TEMPLATE, REPLY_COOKIE_TEMPLATE, typename Derived, typename Check>
class reply<Connection, REPLY_SIGNATURE, REPLY_COOKIE_SIGNATURE, Derived, Check>
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

// template<REPLY_TEMPLATE, REPLY_COOKIE_TEMPLATE, typename Check>
// class reply<REPLY_SIGNATURE, REPLY_COOKIE_SIGNATURE, Check>
//   : public reply<REPLY_SIGNATURE, REPLY_COOKIE_SIGNATURE,
//                  reply<REPLY_SIGNATURE, REPLY_COOKIE_SIGNATURE, Check>,
//                  Check>
// {
//   public:
//     typedef reply<REPLY_SIGNATURE, REPLY_COOKIE_SIGNATURE, Check> self;
//     typedef reply<REPLY_SIGNATURE, REPLY_COOKIE_SIGNATURE, self, Check> base;
//     using base::base;
// };

}; }; // namespace xpp::generic

/*
namespace test {

namespace reply {

template<typename Connection, typename Check>
class get_window_attributes
  : public xpp::generic::reply<SIGNATURE(xcb_get_window_attributes_reply),
                               SIGNATURE(xcb_get_window_attributes),
                               get_window_attributes<Connection, Check>,
                               Check>
  , public xpp::generic::error_handler<Connection, xpp::x::error::dispatcher>
{
  public:
    typedef xpp::generic::reply<SIGNATURE(xcb_get_window_attributes_reply),
                                SIGNATURE(xcb_get_window_attributes),
                                get_window_attributes<Connection, Check>,
                                Check>
                                  base;

    typedef xpp::generic::error_handler<Connection, xpp::x::error::dispatcher>
      error_handler;

    template<typename ... Parameter>
    get_window_attributes(Connection && c, Parameter && ... parameter)
      : base(std::forward<Connection>(c), std::forward<Parameter>(parameter) ...)
      , error_handler(std::forward<Connection>(c))
    {}

    template<typename ... Parameter>
    static
    xcb_get_window_attributes_cookie_t
    cookie(Parameter && ... parameter)
    {
      std::cerr << "get_window_attributes_2 cookie" << std::endl;
      return base::cookie(std::forward<Parameter>(parameter) ...);
    }

    void
    handle(const std::shared_ptr<xcb_generic_error_t> & error)
    {
      error_handler::handle(error);
    }
};

}; // namespace reply

template<typename ... Parameter>
void
map_window_checked(xcb_connection_t * const c, Parameter && ... parameter)
{
  xpp::generic::check(
      c, xcb_map_window_checked(c, std::forward<Parameter>(parameter) ...));
}

template<typename ... Parameter>
void
map_window(Parameter ... parameter)
{
  xcb_map_window(parameter ...);
}

template<typename Connection, typename ... Parameter>
reply::get_window_attributes<Connection, xpp::generic::checked_tag>
get_window_attributes(Connection && c, Parameter && ... parameter)
{
  return reply::get_window_attributes<Connection, xpp::generic::checked_tag>(
      std::forward<Connection>(c), std::forward<Parameter>(parameter) ...);
}

template<typename Connection, typename ... Parameter>
reply::get_window_attributes<Connection, xpp::generic::unchecked_tag>
get_window_attributes_unchecked(Connection && c, Parameter && ... parameter)
{
  return reply::get_window_attributes<Connection, xpp::generic::unchecked_tag>(
      std::forward<Connection>(c), std::forward<Parameter>(parameter) ...);
}

// template<typename ... Parameter>
// reply::get_window_attributes<xpp::generic::unchecked_tag>
// get_window_attributes_unchecked(Parameter && ... parameter)
// {
//   return reply::get_window_attributes<xpp::generic::unchecked_tag>(
//       std::forward<Parameter>(parameter) ...);
// }

}; // namespace test
*/

#endif // X_REQUEST_HPP
