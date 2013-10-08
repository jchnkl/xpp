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

    uint32_t
    generate_id(void)
    {
      return xcb_generate_id(m_c);
    }

    void
    open_font(xcb_font_t fid, const std::string & name)
    {
      xcb_open_font(m_c, fid, name.length(), name.c_str());
    }

    void
    close_font(xcb_font_t font)
    {
      xcb_close_font(m_c, font);
    }

    void
    create_glyph_cursor(xcb_cursor_t cid, xcb_font_t source_font,
                        xcb_font_t mask_font, uint16_t source_char,
                        uint16_t mask_char, uint16_t fore_red,
                        uint16_t fore_green, uint16_t fore_blue,
                        uint16_t back_red, uint16_t back_green,
                        uint16_t back_blue)
    {
      xcb_create_glyph_cursor(m_c, cid, source_font, mask_font, source_char,
                              mask_char, fore_red, fore_green, fore_blue,
                              back_red, back_green, back_blue);
    }

    void
    free_cursor(xcb_cursor_t cursor)
    {
      xcb_free_cursor(m_c, cursor);
    }

    void
    create_gc(xcb_gcontext_t cid, xcb_drawable_t drawable,
              const uint32_t & mask,
              const std::vector<uint32_t> & values)
    {
      xcb_create_gc(m_c, cid, drawable, mask, values.data());
    }

    void
    free_gc(xcb_gcontext_t gc)
    {
      xcb_free_gc(m_c, gc);
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
    change_attributes(xcb_window_t window,
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

    void
    warp_pointer(xcb_window_t src_window, xcb_window_t dst_window,
                 int16_t src_x, int16_t src_y,
                 uint16_t src_width, uint16_t src_height,
                 int16_t dst_x, int16_t dst_y)
    {
      xcb_warp_pointer(m_c, src_window, dst_window,
                       src_x, src_y, src_width, src_height, dst_x, dst_y);
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

    request::grab_pointer
    grab_pointer(bool owner_events, xcb_window_t grab_window,
                 uint16_t event_mask, uint8_t pointer_mode,
                 uint8_t keyboard_mode, xcb_window_t confine_to,
                 xcb_cursor_t cursor,
                 xcb_timestamp_t time = XCB_TIME_CURRENT_TIME)
    {
      return request::grab_pointer(m_c, owner_events, grab_window, event_mask,
                                   pointer_mode, keyboard_mode, confine_to,
                                   cursor, time);
    }

    void
    ungrab_pointer(xcb_timestamp_t time = XCB_TIME_CURRENT_TIME)
    {
      xcb_ungrab_pointer(m_c, time);
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
    xcb_screen_t * const screen_of_display(int screen)
    {
      xcb_screen_iterator_t iter;

      iter = xcb_setup_roots_iterator(xcb_get_setup(m_c));
      for (; iter.rem; --screen, xcb_screen_next(&iter))
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
