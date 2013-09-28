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
    change_window_attributes(xcb_window_t window,
                             const uint32_t & mask,
                             const std::vector<uint32_t> & values)
    {
      xcb_change_window_attributes(*m_c, m_window, mask, values.data());
    }

  protected:
    xcb_window_t & m_window = m_drawable;
}; // class window

}; // namespace x

#endif // X_WINDOW_HPP
