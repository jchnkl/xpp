#ifndef X_CONNECTION_HPP
#define X_CONNECTION_HPP

#include <vector>

#include "requests.hpp"

namespace x {

class connection {
  public:
    connection(const std::string & displayname)
    {
      m_c = xcb_connect(displayname.c_str(), &m_default_screen_number);
      m_default_screen_of_display = screen_of_display(m_default_screen_number);
    }

    ~connection(void)
    {
      delete m_default_screen_of_display;
      xcb_disconnect(m_c);
    }

    xcb_connection_t * const operator*(void)
    {
      return m_c;
    }

    xcb_screen_t * const default_screen_of_display(void)
    {
      return m_default_screen_of_display;
    }

    xcb_window_t root(void)
    {
      return m_default_screen_of_display->root;
    }

    void
    change_window_attributes(xcb_window_t window,
                             const uint32_t & mask,
                             const std::vector<uint32_t> & values)
    {
      xcb_change_window_attributes(m_c, window, mask, values.data());
    }

    request::query_tree
    query_tree(xcb_window_t window)
    {
      return request::query_tree(m_c, window);
    }

  private:
    xcb_screen_t * screen_of_display(int screen)
    {
      xcb_screen_iterator_t iter;

      iter = xcb_setup_roots_iterator(xcb_get_setup(m_c));
      for (; iter.rem; --screen, xcb_screen_next (&iter))
        if (screen == 0)
          return iter.data;

      return NULL;
    }

    xcb_connection_t * m_c = NULL;
    int m_default_screen_number = 0;
    xcb_screen_t * m_default_screen_of_display = NULL;
};

};

#endif // X_CONNECTION_HPP
