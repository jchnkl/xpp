#ifndef XPP_CORE_HPP
#define XPP_CORE_HPP

#include <string>
#include <memory>
#include <xcb/xcb.h>
#include "generic/type.hpp"

namespace xpp {

class core
  : public xpp::xcb::type<xcb_connection_t * const>
{
  public:
    explicit
    core(xcb_connection_t * c)
      : m_c(std::shared_ptr<xcb_connection_t>(c, [](...) {}))
    {}

    template<typename ... ConnectionParameter>
    explicit
    core(xcb_connection_t * (*Connect)(ConnectionParameter ...),
               ConnectionParameter ... connection_parameter)
      : m_c(std::shared_ptr<xcb_connection_t>(
          Connect(connection_parameter ...),
          [&](void *) { disconnect(); }))
    {}

    // xcb_connect (const char *displayname, int *screenp)
    explicit
    core(const std::string & displayname = "")
      : core(xcb_connect, displayname.c_str(), &m_screen)
    {}

    // xcb_connect_to_fd (int fd, xcb_auth_info_t *auth_info)
    explicit
    core(int fd, xcb_auth_info_t * auth_info)
      : core(xcb_connect_to_fd, fd, auth_info)
    {}

    // xcb_connect_to_display_with_auth_info (
    //     const char *display, xcb_auth_info_t *auth, int *screen)
    explicit
    core(const std::string & display, xcb_auth_info_t * auth)
      : core(xcb_connect_to_display_with_auth_info,
                   display.c_str(), auth, &m_screen)
    {}

    virtual
    ~core(void)
    {}

    virtual
    xcb_connection_t * const
    operator*(void) const
    {
      return m_c.get();
    }

    virtual
    operator xcb_connection_t * const(void) const
    {
      return m_c.get();
    }

    virtual
    int
    default_screen(void) const
    {
      return m_screen;
    }

    virtual
    int
    flush(void) const
    {
      return xcb_flush(m_c.get());
    }

    virtual
    uint32_t
    get_maximum_request_length(void) const
    {
      return xcb_get_maximum_request_length(m_c.get());
    }

    virtual
    void
    prefetch_maximum_request_length(void) const
    {
      xcb_prefetch_maximum_request_length(m_c.get());
    }

    virtual
    std::shared_ptr<xcb_generic_event_t>
    wait_for_event(void) const
    {
      xcb_generic_event_t * event = xcb_wait_for_event(m_c.get());
      if (event) {
        return std::shared_ptr<xcb_generic_event_t>(event, std::free);
      } else {
        throw std::runtime_error("xcb_wait_for_event failed");
      }
    }

    virtual
    std::shared_ptr<xcb_generic_event_t>
    poll_for_event(void) const
    {
      xcb_generic_event_t * event = xcb_poll_for_event(m_c.get());
      if (event) {
        return std::shared_ptr<xcb_generic_event_t>(event, std::free);
      } else {
        throw std::runtime_error("xcb_poll_for_event failed");
      }
    }

    virtual
    std::shared_ptr<xcb_generic_event_t>
    poll_for_queued_event(void) const
    {
      xcb_generic_event_t * event = xcb_poll_for_queued_event(m_c.get());
      if (event) {
        return std::shared_ptr<xcb_generic_event_t>(event, std::free);
      } else {
        throw std::runtime_error("xcb_poll_for_queued_event failed");
      }
    }

    virtual
    std::shared_ptr<xcb_generic_event_t>
    poll_for_special_event(xcb_special_event_t * se) const
    {
      xcb_generic_event_t * event = xcb_poll_for_special_event(m_c.get(), se);
      if (event) {
        return std::shared_ptr<xcb_generic_event_t>(event, std::free);
      } else {
        throw std::runtime_error("xcb_poll_for_special_event failed");
      }
    }

    virtual
    std::shared_ptr<xcb_generic_event_t>
    wait_for_special_event(xcb_special_event_t * se) const
    {
      xcb_generic_event_t * event = xcb_wait_for_special_event(m_c.get(), se);
      if (event) {
        return std::shared_ptr<xcb_generic_event_t>(event, std::free);
      } else {
        throw std::runtime_error("xcb_wait_for_event failed");
      }
    }

    virtual
    xcb_special_event_t *
    register_for_special_xge(xcb_extension_t * ext,
                               uint32_t eid,
                               uint32_t * stamp) const
    {
      return xcb_register_for_special_xge(m_c.get(), ext, eid, stamp);
    }

    virtual
    void
    unregister_for_special_event(xcb_special_event_t * se) const
    {
      xcb_unregister_for_special_event(m_c.get(), se);
    }

    virtual
    xcb_generic_error_t *
    request_check(xcb_void_cookie_t cookie) const
    {
      return xcb_request_check(m_c.get(), cookie);
    }

    virtual
    void
    discard_reply(unsigned int sequence) const
    {
      xcb_discard_reply(m_c.get(), sequence);
    }

    // The result must not be freed.
    // This storage is managed by the cache itself.
    virtual
    const xcb_query_extension_reply_t * const
    get_extension_data(xcb_extension_t * ext) const
    {
      return xcb_get_extension_data(m_c.get(), ext);
    }

    virtual
    void
    prefetch_extension_data(xcb_extension_t * ext) const
    {
      xcb_prefetch_extension_data(m_c.get(), ext);
    }

    virtual
    const xcb_setup_t * const
    get_setup(void) const
    {
      return xcb_get_setup(m_c.get());
    }

    virtual
    int
    get_file_descriptor(void) const
    {
      return xcb_get_file_descriptor(m_c.get());
    }

    virtual
    int
    connection_has_error(void) const
    {
      return xcb_connection_has_error(m_c.get());
    }

    virtual
    void
    disconnect(void) const
    {
      xcb_disconnect(m_c.get());
    }

    // hostname, display, screen
    virtual
    std::tuple<std::string, int, int>
    parse_display(const std::string & name) const
    {
      int screen = 0;
      int display = 0;
      char * host = NULL;
      std::string hostname;

      xcb_parse_display(name.c_str(), &host, &display, &screen);
      if (host != NULL) {
        hostname = std::string(host);
      }

      return std::make_tuple(hostname, display, screen);
    }

    virtual
    uint32_t
    generate_id(void) const
    {
      return xcb_generate_id(m_c.get());
    }

    xcb_screen_t * const
    screen_of_display(int screen)
    {
      xcb_screen_iterator_t iter;

      iter = xcb_setup_roots_iterator(xcb_get_setup(m_c.get()));
      for (; iter.rem; --screen, xcb_screen_next(&iter))
        if (screen == 0)
          return iter.data;

      return NULL;
    }

  private:
    int m_screen = 0;
    // reference counting for xcb_connection_t
    std::shared_ptr<xcb_connection_t> m_c;

}; // class core

}; // namespace xpp

#endif // XPP_CORE_HPP
