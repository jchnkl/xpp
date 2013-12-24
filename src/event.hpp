#ifndef X_EVENT_HPP
#define X_EVENT_HPP

#include <climits>
#include <map>
#include <unordered_map>

#include <xcb/xcb.h>

#include "connection.hpp"

#define MAX_PRIORITY UINT32_MAX

#define EVENT_OBJECT(CLASS, TYPE, POINTER)                                    \
struct CLASS {                                                                \
  CLASS(POINTER * const event) : event(event) {}                              \
  POINTER * const operator*(void) const { return event; }                     \
  POINTER * const operator->(void) const { return event; }                    \
  static const int type = TYPE;                                               \
  POINTER * const event;                                                      \
};

#define NS_EVENT_OBJECT(NAMESPACE, CLASS, TYPE, STRUCT)                       \
namespace NAMESPACE {                                                         \
  EVENT_OBJECT(CLASS, TYPE, STRUCT)                                           \
};

#define EVENT_HANDLE(IF, CLASS, TYPE, POINTER)                                \
  IF ((e->response_type & ~0x80) == TYPE) {                                   \
    CLASS event(reinterpret_cast<POINTER *>(e));                              \
    for (auto & item : m_dispatcher.at(TYPE)) {                               \
      try {                                                                   \
        item.second->dispatch(item.second, event);                            \
      } catch(...) {}                                                         \
    }                                                                         \
  }

#define NS_EVENT_HANDLE(IF, NAMESPACE, CLASS, TYPE, POINTER)                  \
  EVENT_HANDLE(IF, NAMESPACE::CLASS, TYPE, POINTER)

namespace xpp {

namespace event {

typedef std::vector<std::pair<unsigned int, int>> priorities;

   EVENT_OBJECT(expose,               XCB_EXPOSE,            xcb_expose_event_t)
NS_EVENT_OBJECT(key,        press,    XCB_KEY_PRESS,         xcb_key_press_event_t)
NS_EVENT_OBJECT(key,        release,  XCB_KEY_RELEASE,       xcb_key_release_event_t)
NS_EVENT_OBJECT(button,     press,    XCB_BUTTON_PRESS,      xcb_button_press_event_t)
NS_EVENT_OBJECT(button,     release,  XCB_BUTTON_RELEASE,    xcb_button_release_event_t)
NS_EVENT_OBJECT(motion,     notify,   XCB_MOTION_NOTIFY,     xcb_motion_notify_event_t)
NS_EVENT_OBJECT(enter,      notify,   XCB_ENTER_NOTIFY,      xcb_enter_notify_event_t)
NS_EVENT_OBJECT(leave,      notify,   XCB_LEAVE_NOTIFY,      xcb_leave_notify_event_t)
NS_EVENT_OBJECT(focus,      in,       XCB_FOCUS_IN,          xcb_focus_in_event_t)
NS_EVENT_OBJECT(focus,      out,      XCB_FOCUS_OUT,         xcb_focus_out_event_t)
NS_EVENT_OBJECT(keymap,     notify,   XCB_KEYMAP_NOTIFY,     xcb_keymap_notify_event_t)
NS_EVENT_OBJECT(graphics,   exposure, XCB_GRAPHICS_EXPOSURE, xcb_graphics_exposure_event_t)
NS_EVENT_OBJECT(no,         exposure, XCB_NO_EXPOSURE,       xcb_no_exposure_event_t)
NS_EVENT_OBJECT(visibility, notify,   XCB_VISIBILITY_NOTIFY, xcb_visibility_notify_event_t)
NS_EVENT_OBJECT(create,     notify,   XCB_CREATE_NOTIFY,     xcb_create_notify_event_t)
NS_EVENT_OBJECT(destroy,    notify,   XCB_DESTROY_NOTIFY,    xcb_destroy_notify_event_t)
NS_EVENT_OBJECT(unmap,      notify,   XCB_UNMAP_NOTIFY,      xcb_unmap_notify_event_t)
NS_EVENT_OBJECT(map,        notify,   XCB_MAP_NOTIFY,        xcb_map_notify_event_t)
NS_EVENT_OBJECT(map,        request,  XCB_MAP_REQUEST,       xcb_map_request_event_t)
NS_EVENT_OBJECT(reparent,   notify,   XCB_REPARENT_NOTIFY,   xcb_reparent_notify_event_t)
NS_EVENT_OBJECT(configure,  notify,   XCB_CONFIGURE_NOTIFY,  xcb_configure_notify_event_t)
NS_EVENT_OBJECT(configure,  request,  XCB_CONFIGURE_REQUEST, xcb_configure_request_event_t)
NS_EVENT_OBJECT(gravity,    notify,   XCB_GRAVITY_NOTIFY,    xcb_gravity_notify_event_t)
NS_EVENT_OBJECT(resize,     request,  XCB_RESIZE_REQUEST,    xcb_resize_request_event_t)
NS_EVENT_OBJECT(circulate,  notify,   XCB_CIRCULATE_NOTIFY,  xcb_circulate_notify_event_t)
NS_EVENT_OBJECT(circulate,  request,  XCB_CIRCULATE_REQUEST, xcb_circulate_request_event_t)
NS_EVENT_OBJECT(property,   notify,   XCB_PROPERTY_NOTIFY,   xcb_property_notify_event_t)
NS_EVENT_OBJECT(selection,  clear,    XCB_SELECTION_CLEAR,   xcb_selection_clear_event_t)
NS_EVENT_OBJECT(selection,  request,  XCB_SELECTION_REQUEST, xcb_selection_request_event_t)
NS_EVENT_OBJECT(selection,  notify,   XCB_SELECTION_NOTIFY,  xcb_selection_notify_event_t)
NS_EVENT_OBJECT(colormap,   notify,   XCB_COLORMAP_NOTIFY,   xcb_colormap_notify_event_t)
NS_EVENT_OBJECT(client,     message,  XCB_CLIENT_MESSAGE,    xcb_client_message_event_t)
NS_EVENT_OBJECT(mapping,    notify,   XCB_MAPPING_NOTIFY,    xcb_mapping_notify_event_t)

class dispatcher {
  public:
    virtual ~dispatcher(void) {}
    template<typename Handler, typename Event>
      void dispatch(Handler *, const Event &);
}; // class dispatcher

template<typename Event>
class sink {
  public:
    virtual ~sink(void) {}
    virtual void handle(const Event &) = 0;
}; // class sink

class source {
  public:
    source(connection & c) : m_c(c) {}

    void run(void)
    {
      xcb_generic_event_t * e;

      while (true) {
        m_c.flush();
        e = xcb_wait_for_event(*m_c);

        if (e == NULL) {
          continue;
        }

           EVENT_HANDLE(else if, expose,                 XCB_EXPOSE,            xcb_expose_event_t)
        NS_EVENT_HANDLE(else if, key,        press,      XCB_KEY_PRESS,         xcb_key_press_event_t)
        NS_EVENT_HANDLE(else if, key,        release,    XCB_KEY_RELEASE,       xcb_key_release_event_t)
        NS_EVENT_HANDLE(else if, button,     press,      XCB_BUTTON_PRESS,      xcb_button_press_event_t)
        NS_EVENT_HANDLE(else if, button,     release,    XCB_BUTTON_RELEASE,    xcb_button_release_event_t)
        NS_EVENT_HANDLE(else if, motion,     notify,     XCB_MOTION_NOTIFY,     xcb_motion_notify_event_t)
        NS_EVENT_HANDLE(else if, enter,      notify,     XCB_ENTER_NOTIFY,      xcb_enter_notify_event_t)
        NS_EVENT_HANDLE(else if, leave,      notify,     XCB_LEAVE_NOTIFY,      xcb_leave_notify_event_t)
        NS_EVENT_HANDLE(else if, focus,      in,         XCB_FOCUS_IN,          xcb_focus_in_event_t)
        NS_EVENT_HANDLE(else if, focus,      out,        XCB_FOCUS_OUT,         xcb_focus_out_event_t)
        NS_EVENT_HANDLE(else if, keymap,     notify,     XCB_KEYMAP_NOTIFY,     xcb_keymap_notify_event_t)
        NS_EVENT_HANDLE(else if, graphics,   exposure,   XCB_GRAPHICS_EXPOSURE, xcb_graphics_exposure_event_t)
        NS_EVENT_HANDLE(else if, no,         exposure,   XCB_NO_EXPOSURE,       xcb_no_exposure_event_t)
        NS_EVENT_HANDLE(else if, visibility, notify,     XCB_VISIBILITY_NOTIFY, xcb_visibility_notify_event_t)
        NS_EVENT_HANDLE(else if, create,     notify,     XCB_CREATE_NOTIFY,     xcb_create_notify_event_t)
        NS_EVENT_HANDLE(else if, destroy,    notify,     XCB_DESTROY_NOTIFY,    xcb_destroy_notify_event_t)
        NS_EVENT_HANDLE(else if, unmap,      notify,     XCB_UNMAP_NOTIFY,      xcb_unmap_notify_event_t)
        NS_EVENT_HANDLE(else if, map,        notify,     XCB_MAP_NOTIFY,        xcb_map_notify_event_t)
        NS_EVENT_HANDLE(else if, map,        request,    XCB_MAP_REQUEST,       xcb_map_request_event_t)
        NS_EVENT_HANDLE(else if, reparent,   notify,     XCB_REPARENT_NOTIFY,   xcb_reparent_notify_event_t)
        NS_EVENT_HANDLE(else if, configure,  notify,     XCB_CONFIGURE_NOTIFY,  xcb_configure_notify_event_t)
        NS_EVENT_HANDLE(else if, configure,  request,    XCB_CONFIGURE_REQUEST, xcb_configure_request_event_t)
        NS_EVENT_HANDLE(else if, gravity,    notify,     XCB_GRAVITY_NOTIFY,    xcb_gravity_notify_event_t)
        NS_EVENT_HANDLE(else if, resize,     request,    XCB_RESIZE_REQUEST,    xcb_resize_request_event_t)
        NS_EVENT_HANDLE(else if, circulate,  notify,     XCB_CIRCULATE_NOTIFY,  xcb_circulate_notify_event_t)
        NS_EVENT_HANDLE(else if, circulate,  request,    XCB_CIRCULATE_REQUEST, xcb_circulate_request_event_t)
        NS_EVENT_HANDLE(else if, property,   notify,     XCB_PROPERTY_NOTIFY,   xcb_property_notify_event_t)
        NS_EVENT_HANDLE(else if, selection,  clear,      XCB_SELECTION_CLEAR,   xcb_selection_clear_event_t)
        NS_EVENT_HANDLE(else if, selection,  request,    XCB_SELECTION_REQUEST, xcb_selection_request_event_t)
        NS_EVENT_HANDLE(else if, selection,  notify,     XCB_SELECTION_NOTIFY,  xcb_selection_notify_event_t)
        NS_EVENT_HANDLE(else if, colormap,   notify,     XCB_COLORMAP_NOTIFY,   xcb_colormap_notify_event_t)
        NS_EVENT_HANDLE(else if, client,     message,    XCB_CLIENT_MESSAGE,    xcb_client_message_event_t)
        NS_EVENT_HANDLE(else if, mapping,    notify,     XCB_MAPPING_NOTIFY,    xcb_mapping_notify_event_t)

        delete e;
      }
    }

    void
    attach(const priorities & masks, dispatcher * h)
    {
      for (auto & item : masks) {
        m_dispatcher[item.second].insert({ item.first, h });
      }
    }

    void
    detach(const priorities & masks, dispatcher * h)
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

    typedef std::multimap<unsigned int, dispatcher *> priorities;
    std::unordered_map<int, priorities> m_dispatcher;
}; // class source

namespace direct {

class container {
  public:
    virtual ~container(void) {}
    virtual dispatcher * const at(const unsigned int &) const = 0;
}; // class container

// O(1) event dispatcher
// container[window]->dispatch(e) ..
template<typename Event, int Priority,
         unsigned int (* Window)(decltype(Event::event))>
class adapter : public dispatcher
              , public sink<Event>
{
  public:
    adapter(source & source, const container & container)
      : m_source(source), m_container(container)
    {
      m_source.attach({ { Priority, Event::type } }, this);
    }

    ~adapter(void)
    {
      m_source.detach({ { Priority, Event::type } }, this);
    }

    void handle(const Event & e)
    {
      auto * d = m_container.at(Window(*e));
      d->dispatch(d, e);
    }

  private:
    source & m_source;
    const container & m_container;
}; // class adapter

// TODO: multi - adapter:
// template<typename ... ETC>
// class mult : public adapter<ETC> ...
// question: how to get multiple variadic template parameters?

}; // namespace direct

// with any object
// Object object = container(window)
// for (handler : handlers) handler->handle(object, event)
namespace any {

template <typename Object>
class container {
  public:
    virtual ~container(void) {}
    virtual Object * const at(const unsigned int &) = 0;
};

template<typename Object, typename Event, int Priority,
         unsigned int (* Window)(decltype(Event::event))>
class adapter : public dispatcher
              , public sink<Event>
{
  public:
    adapter(source & source, container<Object> & container)
      : m_source(source)
      , m_container(container)
    {
      m_source.attach({ { Priority, Event::type } }, this);
    }

    ~adapter(void)
    {
      m_source.detach({ { Priority, Event::type } }, this);
    }

    void handle(const Event & e)
    {
      handle(m_container.at(Window(*e)), e);
    }

    virtual void handle(Object * const, const Event &) = 0;

  private:
    source & m_source;
    container<Object> & m_container;
}; // class adapter

}; // namespace any

template<typename Handler, typename Event>
void dispatcher::dispatch(Handler * h, const Event & e)
{
  try {
    dynamic_cast<sink<Event> &>(*h).handle(e);
  } catch (...) {}
}

}; // namespace event

}; // namespace xpp

#endif // X_EVENT_HPP
