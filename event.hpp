#ifndef X_EVENT_HPP
#define X_EVENT_HPP

#include <map>
#include <unordered_map>

#include <xcb/xcb.h>

#include "interface.hpp"
#include "connection.hpp"

#define EVENT_HANDLE(IF, TYPE, POINTER) \
  IF ((e->response_type & ~0x80) == TYPE) { \
    try { \
      for (auto & item : m_dispatcher.at(TYPE)) { \
        item.second->dispatch(item.second, reinterpret_cast<POINTER *>(e)); \
      } \
    } catch(...) {} \
  }

namespace x
{

namespace event
{

namespace interface = x::interface::event;

class source : public interface::source
{
  public:
    source(connection & c) : m_c(c) {}

    void run(void)
    {
      xcb_generic_event_t * e;

      while (true)
      {
        m_c.flush();
        e = xcb_wait_for_event(*m_c);

        if (e == NULL) {
          continue;
        }

        EVENT_HANDLE(else if, XCB_KEY_PRESS,         xcb_key_press_event_t)
        EVENT_HANDLE(else if, XCB_KEY_RELEASE,       xcb_key_release_event_t)
        EVENT_HANDLE(else if, XCB_BUTTON_PRESS,      xcb_button_press_event_t)
        EVENT_HANDLE(else if, XCB_BUTTON_RELEASE,    xcb_button_release_event_t)
        EVENT_HANDLE(else if, XCB_MOTION_NOTIFY,     xcb_motion_notify_event_t)
        EVENT_HANDLE(else if, XCB_ENTER_NOTIFY,      xcb_enter_notify_event_t)
        EVENT_HANDLE(else if, XCB_LEAVE_NOTIFY,      xcb_leave_notify_event_t)
        EVENT_HANDLE(else if, XCB_FOCUS_IN,          xcb_focus_in_event_t)
        EVENT_HANDLE(else if, XCB_FOCUS_OUT,         xcb_focus_out_event_t)
        EVENT_HANDLE(else if, XCB_KEYMAP_NOTIFY,     xcb_keymap_notify_event_t)
        EVENT_HANDLE(else if, XCB_EXPOSE,            xcb_expose_event_t)
        EVENT_HANDLE(else if, XCB_GRAPHICS_EXPOSURE, xcb_graphics_exposure_event_t)
        EVENT_HANDLE(else if, XCB_NO_EXPOSURE,       xcb_no_exposure_event_t)
        EVENT_HANDLE(else if, XCB_VISIBILITY_NOTIFY, xcb_visibility_notify_event_t)
        EVENT_HANDLE(else if, XCB_CREATE_NOTIFY,     xcb_create_notify_event_t)
        EVENT_HANDLE(else if, XCB_DESTROY_NOTIFY,    xcb_destroy_notify_event_t)
        EVENT_HANDLE(else if, XCB_UNMAP_NOTIFY,      xcb_unmap_notify_event_t)
        EVENT_HANDLE(else if, XCB_MAP_NOTIFY,        xcb_map_notify_event_t)
        EVENT_HANDLE(else if, XCB_MAP_REQUEST,       xcb_map_request_event_t)
        EVENT_HANDLE(else if, XCB_REPARENT_NOTIFY,   xcb_reparent_notify_event_t)
        EVENT_HANDLE(else if, XCB_CONFIGURE_NOTIFY,  xcb_configure_notify_event_t)
        EVENT_HANDLE(else if, XCB_CONFIGURE_REQUEST, xcb_configure_request_event_t)
        EVENT_HANDLE(else if, XCB_GRAVITY_NOTIFY,    xcb_gravity_notify_event_t)
        EVENT_HANDLE(else if, XCB_RESIZE_REQUEST,    xcb_resize_request_event_t)
        EVENT_HANDLE(else if, XCB_CIRCULATE_NOTIFY,  xcb_circulate_notify_event_t)
        EVENT_HANDLE(else if, XCB_CIRCULATE_REQUEST, xcb_circulate_request_event_t)
        EVENT_HANDLE(else if, XCB_PROPERTY_NOTIFY,   xcb_property_notify_event_t)
        EVENT_HANDLE(else if, XCB_SELECTION_CLEAR,   xcb_selection_clear_event_t)
        EVENT_HANDLE(else if, XCB_SELECTION_REQUEST, xcb_selection_request_event_t)
        EVENT_HANDLE(else if, XCB_SELECTION_NOTIFY,  xcb_selection_notify_event_t)
        EVENT_HANDLE(else if, XCB_COLORMAP_NOTIFY,   xcb_colormap_notify_event_t)
        EVENT_HANDLE(else if, XCB_CLIENT_MESSAGE,    xcb_client_message_event_t)
        EVENT_HANDLE(else if, XCB_MAPPING_NOTIFY,    xcb_mapping_notify_event_t)

        delete e;
      }
    }

    void insert(interface::dispatcher * h)
    {
      insert(h->masks(), h);
    }

    void remove(interface::dispatcher * h)
    {
      remove(h->masks(), h);
    }

    void
    insert(const interface::dispatcher::priority_masks & masks,
           interface::dispatcher * h)
    {
      for (auto & item : masks) {
        m_dispatcher[item.second].insert({ item.first, h });
      }
    }

    void
    remove(const interface::dispatcher::priority_masks & masks,
           interface::dispatcher * h)
    {
      for (auto & item : masks) {
        try {
          auto & pmap = m_dispatcher.at(item.second);
          for (auto it = pmap.begin(); it != pmap.end(); ) {
            if (h == it->second) {
              it = pmap.erase(it);
            } else {
              ++it;
            }
          }
        } catch (...) {}
      }
    }

  private:
    connection & m_c;

    typedef std::multimap<unsigned int, interface::dispatcher *> priorities;
    std::unordered_map<interface::type, priorities> m_dispatcher;
}; // class source

}; // namespace event

}; // namespace x

#endif // X_EVENT_HPP
