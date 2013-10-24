#ifndef X_WINDOW_HPP
#define X_WINDOW_HPP

#include "drawable.hpp"

namespace x {

class window : public drawable<xcb_window_t> {
  public:
    window(connection & c, xcb_window_t window)
      : drawable(c, window)
    {}

    void
    map(void)
    {
      m_c.map(m_window);
    }

    void
    unmap(void)
    {
      m_c.unmap(m_window);
    }

    void
    change_attributes(const uint32_t & mask,
                      const std::vector<uint32_t> & values)
    {
      m_c.change_attributes(m_window, mask, values);
    }

    request::get_window_attributes
    get_attributes(void)
    {
      return request::get_window_attributes(*m_c, m_window);
    }

    request::get_geometry
    get_geometry(void)
    {
      return request::get_geometry(*m_c, m_window);
    }

    void
    configure(const uint32_t & mask,
              const std::vector<uint32_t> & values)
    {
      m_c.configure(m_window, mask, values);
    }

    void
    warp_pointer(int16_t dst_x, int16_t dst_y)
    {
      m_c.warp_pointer(XCB_NONE, m_window, 0, 0, 0, 0, dst_x, dst_y);
    }

    request::grab_pointer
    grab_pointer(bool owner_events, uint16_t event_mask, uint8_t pointer_mode,
                 uint8_t keyboard_mode, xcb_window_t confine_to,
                 xcb_cursor_t cursor,
                 xcb_timestamp_t time = XCB_TIME_CURRENT_TIME)
    {
      return m_c.grab_pointer(owner_events, m_window, event_mask,
                              pointer_mode, keyboard_mode, confine_to,
                              cursor, time);
    }

    void
    grab_button(bool owner_events, uint16_t event_mask, uint8_t pointer_mode,
                uint8_t keyboard_mode, xcb_window_t confine_to,
                xcb_cursor_t cursor, uint8_t button, uint16_t modifiers)
    {
      m_c.grab_button(owner_events, m_window, event_mask, pointer_mode,
                      keyboard_mode, confine_to, cursor, button, modifiers);
    }

    void
    ungrab_button(uint8_t button, uint16_t modifiers)
    {
      m_c.ungrab_button(button, m_window, modifiers);
    }

    void
    grab_key(bool owner_events, uint16_t modifiers,
             xcb_keycode_t key, uint8_t pointer_mode, uint8_t keyboard_mode)
    {
      m_c.grab_key(owner_events, m_window, modifiers,
                   key, pointer_mode, keyboard_mode);
    }

    void
    ungrab_key(xcb_keycode_t key, uint16_t modifiers)
    {
      m_c.ungrab_key(key, m_window, modifiers);
    }

    void
    focus(xcb_input_focus_t revert_to,
          xcb_timestamp_t time = XCB_TIME_CURRENT_TIME)
    {
      m_c.focus(revert_to, m_window, time);
    }

  protected:
    xcb_window_t & m_window = m_drawable;
}; // class window

}; // namespace x

#endif // X_WINDOW_HPP
