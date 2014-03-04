#ifndef XPP_CONNECTION_HPP
#define XPP_CONNECTION_HPP

#include "core.hpp"
// #include "type.hpp"
#include "window.hpp"
#include "extension.hpp"
// #include "core/connection.hpp"
#include "../factory.hpp"
#include "generic/connection.hpp"

// #include "../gen/xproto-stub.hpp"
#include "../gen/xproto.hpp"
// #include "../gen/protos.hpp"

namespace xpp {

template<typename ... Extensions>
class connection
  : virtual public xpp::core
  , virtual public xpp::x::extension
  , virtual public xpp::x::extension::protocol<connection<Extensions ...> &>
  , virtual public Extensions ...
  , virtual public Extensions::template protocol<connection<Extensions ...> &> ...
  , virtual protected xpp::generic::connection<connection<Extensions ...> &>
{
  protected:
    typedef connection<Extensions ...> self;

    // virtual const self & get(void) const
    // {
    //   // return const_cast<self &>(*this);
    //   return *this;
    // }

    virtual self & get(void)
    {
      // return const_cast<self &>(*this);
      return *this;
    }

    virtual const self & get(void) const
    {
      // return const_cast<self &>(*this);
      return *this;
    }

  public:
    template<typename ... Parameters>
    explicit
    connection(Parameters && ... parameters)
      : xpp::core::core(std::forward<Parameters>(parameters) ...)
      // , m_root_window(static_cast<const core &>(*this))
      // , m_root_window(*this)
    {
      prefetch_data<Extensions ...>();
      get_data<Extensions ...>();
      m_root_window = screen_of_display(default_screen())->root;
    }

    // virtual
    // ~connection(void)
    // {}

    // virtual
    // operator core &(void)
    // {
    //   return static_cast<core &>(*this);
    // }

    // virtual
    // operator const core &(void) const
    // {
    //   return static_cast<const core &>(*this);
    // }

    virtual
    operator xcb_connection_t * const(void) const
    {
      return *(static_cast<const core &>(*this));
    }

    template<typename Extension>
    operator const connection<Extension> &(void) const
    {
      return reinterpret_cast<const connection<Extension> &>(*this);
      // return static_cast<connection<> &>(*this);
      // return *this;
    }

    // operator const connection<> &(void) const
    // {
    //   return reinterpret_cast<const connection<> &>(*this);
    //   // return static_cast<connection<> &>(*this);
    //   // return *this;
    // }

    // TODO
    // virtual operator Display * const(void) const
    // {
    // }

    template<typename Window = xcb_window_t>
    Window
    root(void)
    {
      using make = xpp::generic::factory::make<self &, xcb_window_t, Window>;
      return make()(m_root_window, *this);
    }

  private:
    // xpp::window<self &> m_root_window;
    xcb_window_t m_root_window;

    template<typename Extension, typename Next, typename ... Rest>
    void
    prefetch_data(void)
    {
      static_cast<Extension *>(this)->prefetch_data();
      prefetch_data<Next, Rest ...>();
    }

    template<typename Extension>
    void
    prefetch_data(void)
    {
      static_cast<Extension *>(this)->prefetch_data();
    }

    template<typename Extension, typename Next, typename ... Rest>
    void
    get_data(void)
    {
      static_cast<Extension *>(this)->get_data();
      get_data<Next, Rest ...>();
    }

    template<typename Extension>
    void
    get_data(void)
    {
      static_cast<Extension *>(this)->get_data();
    }

}; // class connection

template<>
template<typename ... Parameters>
connection<>::connection(Parameters && ... parameters)
  : xpp::core::core(std::forward<Parameters>(parameters) ...)
  // , m_root_window(static_cast<const core &>(*this))
  // , m_root_window(*this)
{
  m_root_window = screen_of_display(static_cast<core &>(*this).default_screen())->root;
}

// template<typename ... Extensions>
// template<>
// connection<Extensions ...>::operator const connection<> &(void) const
// {
//   return reinterpret_cast<const connection<> &>(*this);
// }

}; // namespace xpp

#endif // XPP_CONNECTION_HPP
