#ifndef X_REQUEST_HPP
#define X_REQUEST_HPP

#include <memory>
#include <xcb/xcb.h>
#include "core/type.hpp"
#include "core/extension.hpp"

namespace xpp { namespace extension {
  class x;
};};

namespace xpp { namespace generic {

template<typename Extension>
typename std::enable_if<! std::is_same<Extension, xpp::extension::x>::value, void>::type
// check(const Extension & ext, const std::shared_ptr<xcb_generic_error_t> & e)
check(const Extension & ext, xcb_generic_error_t * e)
{
  typedef typename Extension::error_dispatcher dispatcher;
  dispatcher(ext->first_error)(e);
}

template<typename Extension>
typename std::enable_if<std::is_same<Extension, xpp::extension::x>::value, void>::type
// check(const Extension & ext, const std::shared_ptr<xcb_generic_error_t> & e)
check(const Extension & ext, xcb_generic_error_t * e)
{
  typedef typename Extension::error_dispatcher dispatcher;
  dispatcher()(e);
}

  // namespace request { class generic { ...
  // namespace x { namespace request { class map_window
  // namespace x { class map_window

// template<typename Reply>
// class reply {
//   private:
//     std::shared_ptr<Reply> m_reply;
// };

namespace generic {

template<typename Connection,
         typename ReplySignature, ReplySignature & RS,
         typename CookieSignature, CookieSignature & CS>
class wrapper;

template<typename Connection,
         typename Reply,
         typename ... ReplyParameter,
         Reply(&ReplyFunction)(ReplyParameter ...),
         typename Cookie,
         typename ... CookieParameter,
         Cookie(&CookieFunction)(CookieParameter ...)>
class wrapper<Connection,
              Reply(ReplyParameter ...), ReplyFunction,
              Cookie(CookieParameter ...), CookieFunction>
{
  public:
    template<typename ... Parameter>
    wrapper(Connection c, Parameter ... parameter)
      : m_c(c)
    {
      prepare(parameter ...);
    }

    virtual
    const Reply &
    operator*(void)
    {
      return *this->get();
    }

    virtual
    const Reply * const
    operator->(void)
    {
      return this->get().get();
    }

    virtual
    std::shared_ptr<Reply>
    get(void)
    {
      if (! m_reply) {
        m_reply = std::shared_ptr<Reply>(ReplyFunction(m_c, m_cookie, nullptr));
      }
      return m_reply;
    }

    virtual
    void
    reset(void)
    {
      m_reply.reset();
    }

  protected:
    Connection m_c;
    Cookie m_cookie;
    std::shared_ptr<Reply> m_reply;

    wrapper(Connection c)
      : m_c(c)
    {}

    virtual
    operator Connection(void) const
    {
      return m_c;
    }

    template<typename ... Parameter>
    void
    prepare(Parameter ... parameter)
    {
      m_cookie = CookieFunction(m_c, parameter ...);
    }
};

};

template<typename Connection,
         typename ReplySignature, ReplySignature & RS,
         typename CookieSignature, CookieSignature & CS>
class wrapper;

template<typename Reply,
         typename ... ReplyParameter,
         Reply(&ReplyFunction)(ReplyParameter ...),
         typename Cookie,
         typename ... CookieParameter,
         Cookie(&CookieFunction)(CookieParameter ...)>
class wrapper<xcb_connection_t,
              Reply(ReplyParameter ...), ReplyFunction,
              Cookie(CookieParameter ...), CookieFunction>
  : public generic::wrapper<xcb_connection_t * const,
                            Reply(ReplyParameter ...), ReplyFunction,
                            Cookie(CookieParameter ...), CookieFunction>
{
  public:
    typedef generic::wrapper<xcb_connection_t * const,
                             Reply(ReplyParameter ...), ReplyFunction,
                             Cookie(CookieParameter ...), CookieFunction>
                               base;
    using base::base;
};

template<typename Connection,
         typename Reply,
         typename ... ReplyParameter,
         Reply(&ReplyFunction)(ReplyParameter ...),
         typename Cookie,
         typename ... CookieParameter,
         Cookie(&CookieFunction)(CookieParameter ...)>
class wrapper<Connection,
              Reply(ReplyParameter ...), ReplyFunction,
              Cookie(CookieParameter ...), CookieFunction>
  : public generic::wrapper<const Connection &,
                            Reply(ReplyParameter ...), ReplyFunction,
                            Cookie(CookieParameter ...), CookieFunction>
{
  public:
    typedef generic::wrapper<const Connection &,
                             Reply(ReplyParameter ...), ReplyFunction,
                             Cookie(CookieParameter ...), CookieFunction>
                               base;
    using base::base;

    virtual
    std::shared_ptr<Reply>
    get(void)
    {
      if (! base::m_reply) {
        // base::m_reply = std::shared_ptr<Reply>(
            // ReplyFunction(base::m_c, base::m_cookie, nullptr));
            Reply r = ReplyFunction(base::m_c, base::m_cookie, nullptr);
      }
      return base::m_reply;
    }

};

struct xpp_connection {
  typedef int extension;
  operator xcb_connection_t *(void) const { return nullptr; }
};

class query_tree
  : public wrapper<xcb_connection_t,
                   decltype(xcb_query_tree_reply), xcb_query_tree_reply,
                   decltype(xcb_query_tree), xcb_query_tree>
{
  public:
    using wrapper::wrapper;
};

template<typename Connection>
class query_tree_2
  : public wrapper<Connection,
                   decltype(xcb_query_tree_reply), xcb_query_tree_reply,
                   decltype(xcb_query_tree), xcb_query_tree>
{
  typedef wrapper<Connection,
                  decltype(xcb_query_tree_reply), xcb_query_tree_reply,
                  decltype(xcb_query_tree), xcb_query_tree>
                    base;
  public:
    using base::base;
};

void foo()
{
  xpp_connection c;
  query_tree_2<xpp_connection> q(c, 0);
}

template<typename Cookie,
         typename Reply,
         Reply * (*ReplyFunction)(xcb_connection_t *, Cookie, xcb_generic_error_t **)>
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
        // xcb_generic_error_t * error ; // = m_error.get();
        // m_reply = std::shared_ptr<Reply>(ReplyFunction(m_c, m_cookie, &error));
        // m_error.reset(error);
      }
      return m_reply;
    }

    void reset(void)
    {
      m_reply.reset();
    }

    template<typename Extension>
    void
    check(const Extension & extension)
    {
      if (m_error) {
        // xcb_generic_error_t * error = m_error.get();
        // m_error.reset(static_cast<xcb_generic_error_t *>(nullptr));
        // xpp::generic::check(extension, error);
        xpp::generic::check(extension, m_error.get());
        m_error.reset(static_cast<xcb_generic_error_t *>(nullptr));
        std::cerr << "after check" << std::endl;
        // m_error.reset(error);
      }
    }

  protected:
    xcb_connection_t * m_c;
    Cookie m_cookie;
    std::shared_ptr<Reply> m_reply;
    std::shared_ptr<xcb_generic_error_t> m_error;

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

// template<>
// template<typename Cookie,
//          typename Reply,
//          Reply * (*ReplyFunction)(xcb_connection_t *, Cookie, xcb_generic_error_t **)>
// void
// // check(const xpp::generic::extension<Id> & extension)
// request<Cookie, Reply, ReplyFunction>::check(const xpp::extension::x & extension)
// {
//   typedef typename Extension::error_dispatcher dispatcher;
//   dispatcher(/* e->first_error */)(
//       xcb_request_check(m_c, NULL);
//   // check(extension->first_error);
// }

}; // namespace generic

}; // namespace xpp

#endif // X_REQUEST_HPP
