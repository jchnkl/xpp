#ifndef XPP_CONNECTION_HPP
#define XPP_CONNECTION_HPP

#include "core.hpp"
#include "../factory.hpp"
#include "generic/connection.hpp"

#include "../gen/xproto.hpp"

namespace xpp {

template<typename ... Extensions>
class connection
  : virtual public xpp::core
  , virtual public xpp::x::extension
  , virtual public xpp::x::extension::error_dispatcher
  , virtual public xpp::x::extension::protocol<connection<Extensions ...> &>
  , virtual public Extensions ...
  , virtual public Extensions::error_dispatcher ...
  , virtual public Extensions::template protocol<connection<Extensions ...> &> ...
  , virtual protected xpp::generic::connection<connection<Extensions ...> &>
{
  protected:
    typedef connection<Extensions ...> self;

    virtual self & get(void)
    {
      return *this;
    }

  public:
    template<typename ... Parameters>
    explicit
    connection(Parameters && ... parameters)
      : xpp::core::core(std::forward<Parameters>(parameters) ...)
    {
      prefetch_data<Extensions ...>();
      get_data<Extensions ...>();
      m_root_window = screen_of_display(default_screen())->root;
    }

    virtual
    ~connection(void)
    {}

    virtual
    operator xcb_connection_t * const(void) const
    {
      return *(static_cast<const core &>(*this));
    }

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
{
  m_root_window = screen_of_display(static_cast<core &>(*this).default_screen())->root;
}

}; // namespace xpp

#endif // XPP_CONNECTION_HPP
