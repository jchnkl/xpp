#ifndef X_CONNECTION_HPP
#define X_CONNECTION_HPP

#include <vector>

#include <xcb/xcb_keysyms.h>

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
    flush(void)
    {
      xcb_flush(m_c);
    }

    void
    map(xcb_window_t window)
    {
      xcb_map_window(m_c, window);
    }

    void
    unmap(xcb_window_t window)
    {
      xcb_unmap_window(m_c, window);
    }

    void
    change_window_attributes(xcb_window_t window,
                             const uint32_t & mask,
                             const std::vector<uint32_t> & values)
    {
      xcb_change_window_attributes(m_c, window, mask, values.data());
    }

    void
    configure(xcb_window_t window, const uint32_t & mask,
              const std::vector<uint32_t> & values)
    {
      xcb_configure_window(m_c, window, mask, values.data());
    }

    std::vector<xcb_window_t>
    query_tree(xcb_window_t window)
    {
      auto reply = request::query_tree(m_c, window);
      std::vector<xcb_window_t>
        result(xcb_query_tree_children(*reply),
               xcb_query_tree_children(*reply)
               + xcb_query_tree_children_length(*reply));
      return result;
    }

    void
    grab_button(bool owner_events, xcb_window_t grab_window,
                uint16_t event_mask, uint8_t pointer_mode,
                uint8_t keyboard_mode, xcb_window_t confine_to,
                xcb_cursor_t cursor, uint8_t button, uint16_t modifiers)
    {
      xcb_grab_button(m_c, owner_events, grab_window, event_mask, pointer_mode,
                      keyboard_mode, confine_to, cursor, button, modifiers);
    }

    void
    ungrab_button(uint8_t button, xcb_window_t grab_window, uint16_t modifiers)
    {
      xcb_ungrab_button(m_c, button, grab_window, modifiers);
    }

    void
    grab_key(bool owner_events, xcb_window_t grab_window, uint16_t modifiers,
             xcb_keycode_t key, uint8_t pointer_mode, uint8_t keyboard_mode)
    {
      xcb_grab_key(m_c, owner_events, grab_window, modifiers, key,
                   pointer_mode, keyboard_mode);
    }

    void
    ungrab_key(xcb_keycode_t key, xcb_window_t grab_window, uint16_t modifiers)
    {
      xcb_ungrab_key(m_c, key, grab_window, modifiers);
    }

    xcb_keycode_t
    keysym_to_keycode(xcb_keysym_t keysym)
    {
      xcb_keycode_t keycode;
      xcb_key_symbols_t * keysyms;

      if (!(keysyms = xcb_key_symbols_alloc(m_c))) {
        keycode = XCB_NONE;
      } else {
        keycode = *xcb_key_symbols_get_keycode(keysyms, keysym);
        xcb_key_symbols_free(keysyms);
      }

      return keycode;
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
