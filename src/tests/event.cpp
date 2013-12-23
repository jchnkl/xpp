#include <iostream>

#include "../event.hpp"
#include "../connection.hpp"

xcb_window_t
get_window(xcb_button_press_event_t * const e)
{
  return e->event;
}

xcb_window_t
get_window(xcb_motion_notify_event_t * const e)
{
  return e->event;
}

namespace test {

class handler : public xpp::event::dispatcher
              , public xpp::event::sink<xcb_key_press_event_t>
              , public xpp::event::sink<xcb_button_press_event_t>
              , public xpp::event::sink<xcb_motion_notify_event_t>
{
  public:
    void handle(xcb_key_press_event_t * const e)
    {
      std::cerr << __PRETTY_FUNCTION__ << std::endl;
    }

    void handle(xcb_button_press_event_t * const e)
    {
      if (XCB_BUTTON_PRESS == (e->response_type & ~0x80)) {
        std::cerr << __PRETTY_FUNCTION__ << " XCB_BUTTON_PRESS" << std::endl;
      } else {
        std::cerr << __PRETTY_FUNCTION__ << " XCB_BUTTON_RELEASE" << std::endl;
      }
    }

    void handle(xcb_motion_notify_event_t * const e)
    {
      std::cerr << __PRETTY_FUNCTION__ << std::endl;
    }
};

class container : public xpp::event::direct::container {
  public:
    xpp::event::dispatcher * const
      at(const unsigned int & window) const
    {
      return m_dispatcher.at(window);
    }

    std::unordered_map<unsigned int, xpp::event::dispatcher *> m_dispatcher;
};

struct foo {
  void bar(void) { std::cerr << __PRETTY_FUNCTION__ << std::endl; }
};

class foo_container : public xpp::event::any::container<foo> {
  public:
    foo * const at(const unsigned int & window)
    {
      return &m_foos.at(window);
    }

    std::unordered_map<unsigned int, foo> m_foos;
};

class foo_handler :
  public xpp::event::any::adapter<foo,
                                              xcb_button_press_event_t,
                                              XCB_BUTTON_PRESS,
                                              0,
                                              get_window>
{
  public:
    using adapter::adapter;

    void handle(foo * const f, xcb_button_press_event_t * const e)
    {
      std::cerr << __PRETTY_FUNCTION__ << " response_type: " << (int)(e->response_type & ~0x80) << std::endl;
      f->bar();
    }
};

}; // namespace test

int main(int argc, char ** argv)
{
  xpp::connection c("");
  xpp::event::source source(c);

  auto windows = c.query_tree(c.root());

  test::handler handler;
  test::container container;

  test::foo_container foo_container;
  test::foo_handler foo_handler(source, foo_container);

  for (auto & window : windows) {
    *(c.grab_pointer(false, window,
                     XCB_EVENT_MASK_BUTTON_PRESS
                     | XCB_EVENT_MASK_BUTTON_RELEASE
                     | XCB_EVENT_MASK_POINTER_MOTION,
                     XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC,
                     XCB_NONE, XCB_NONE, XCB_TIME_CURRENT_TIME));

    container.m_dispatcher[window] = &handler;
    foo_container.m_foos[window] = test::foo();
  }

  xpp::event::dispatcher * dispatcher[] =
    { new xpp::event::direct::adapter<xcb_button_press_event_t, XCB_BUTTON_PRESS, 0, get_window>(source, container)
    , new xpp::event::direct::adapter<xcb_button_release_event_t, XCB_BUTTON_RELEASE, 0, get_window>(source, container)
    , new xpp::event::direct::adapter<xcb_motion_notify_event_t, XCB_MOTION_NOTIFY, 0, get_window>(source, container)
    };

  source.run();

  for (auto * d : dispatcher) {
    delete d;
  }

  return EXIT_SUCCESS;
}
