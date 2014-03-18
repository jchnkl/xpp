#ifndef XPP_CONNECTION_HPP
#define XPP_CONNECTION_HPP

#include "core.hpp"
#include "generic/factory.hpp"
#include "generic/connection.hpp"

#include "proto/x.hpp"

namespace xpp {

template<typename ... Extensions>
class connection
  : public xpp::core
  , public xpp::x::extension
  , public xpp::x::extension::error_dispatcher
  , public xpp::x::extension::protocol<connection<Extensions ...> &>
  , public Extensions ...
  , public Extensions::error_dispatcher ...
  , public Extensions::template protocol<connection<Extensions ...> &> ...
  , public xpp::generic::error_dispatcher
{
  protected:
    typedef connection<Extensions ...> self;

    operator const self &(void) const
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
      init_extensions<Extensions ...>();
      m_root_window = screen_of_display(default_screen())->root;
    }

    virtual
    ~connection(void)
    {}

    virtual
    operator xcb_connection_t *(void) const
    {
      return *(static_cast<const core &>(*this));
    }

    void
    operator()(const std::shared_ptr<xcb_generic_error_t> & error) const
    {
      check<xpp::x::extension, Extensions ...>(error);
    }

    // TODO
    // virtual operator Display * const(void) const
    // {
    // }

    template<typename Window = xcb_window_t>
    Window
    root(void) const
    {
      using make = xpp::generic::factory::make<self, xcb_window_t, Window>;
      return make()(*this, m_root_window);
    }

    virtual
    shared_generic_event_ptr
    wait_for_event(void) const
    {
      try {
        return core::wait_for_event();
      } catch (const std::shared_ptr<xcb_generic_error_t> & error) {
        check<xpp::x::extension, Extensions ...>(error);
      }
      // re-throw any exception caused by wait_for_event
      throw;
    }

    virtual
    shared_generic_event_ptr
    wait_for_special_event(xcb_special_event_t * se) const
    {
      try {
        return core::wait_for_special_event(se);
      } catch (const std::shared_ptr<xcb_generic_error_t> & error) {
        check<xpp::x::extension, Extensions ...>(error);
      }
      // re-throw any exception caused by wait_for_special_event
      throw;
    }

  private:
    xcb_window_t m_root_window;

    template<typename Extension, typename Next, typename ... Rest>
    void
    prefetch_data(void)
    {
      prefetch_data<Extension>();
      prefetch_data<Next, Rest ...>();
    }

    template<typename Extension>
    void
    prefetch_data(void)
    {
      static_cast<Extension *>(this)->prefetch_data();
      std::cerr << "prefetch_data" << std::endl;
    }

    template<typename Extension, typename Next, typename ... Rest>
    void
    init_extensions(void)
    {
      init_extensions<Extension>();
      init_extensions<Next, Rest ...>();
    }

    template<typename Extension>
    void
    init_extensions(void)
    {
      using error_dispatcher = typename Extension::error_dispatcher &;
      Extension & extension = static_cast<Extension &>(*this);
      extension.init();
      static_cast<error_dispatcher>(*this).first_error(extension->first_error);
    }

    template<typename Extension, typename Next, typename ... Rest>
    void
    check(const std::shared_ptr<xcb_generic_error_t> & error) const
    {
      check<Extension>(error);
      check<Next, Rest ...>(error);
    }

    template<typename Extension>
    void
    check(const std::shared_ptr<xcb_generic_error_t> & error) const
    {
      using error_dispatcher = typename Extension::error_dispatcher;
      auto & dispatcher = static_cast<const error_dispatcher &>(*this);
      dispatcher(error);
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
