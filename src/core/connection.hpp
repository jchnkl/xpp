#ifndef XPP_CONNECTION_HPP
#define XPP_CONNECTION_HPP

#include "core.hpp"
#include "window.hpp"
#include "extension.hpp"
#include "type.hpp"

#include "../gen/protos.hpp"

namespace xpp {

template<typename ... Extensions>
class connection
  : virtual public xpp::core
  , virtual public xpp::extension::x
  , virtual public xpp::extension::x::protocol
  , virtual public Extensions ...
  , virtual public Extensions::protocol ...
{
  public:
    template<typename ... Parameters>
    explicit
    connection(Parameters ... parameters)
      : xpp::core::core(parameters ...)
      , m_root_window(static_cast<const core &>(*this))
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

    virtual
    const xpp::window &
    root(void)
    {
      return m_root_window;
    }

  private:
    xpp::window m_root_window;

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

    xcb_screen_t * const
    screen_of_display(int screen)
    {
      xcb_screen_iterator_t iter;

      iter = xcb_setup_roots_iterator(xcb_get_setup(*this));
      for (; iter.rem; --screen, xcb_screen_next(&iter))
        if (screen == 0)
          return iter.data;

      return NULL;
    }

}; // class connection

template<>
template<typename ... Parameters>
connection<>::connection(Parameters ... parameters)
  : xpp::core::core(parameters ...)
  , m_root_window(static_cast<const core &>(*this))
{
  m_root_window = screen_of_display(default_screen())->root;
}

}; // namespace xpp

#endif // XPP_CONNECTION_HPP
