#ifndef X_CURSOR_HPP
#define X_CURSOR_HPP

#include <unordered_map>

#include <X11/cursorfont.h>

#include "connection.hpp"

namespace x {

class cursor {
  public:
    cursor(x::connection & c) : m_c(c)
    {
      m_font = m_c.generate_id();
      m_c.open_font(m_font, "cursor");
    }

    ~cursor(void)
    {
      m_c.close_font(m_font);
      for (auto & item : m_cursors) {
        m_c.free_cursor(item.second);
      }
    }

    xcb_cursor_t
    operator[](const uint16_t & source_char)
    {
      try {
        return m_cursors.at(source_char);
      } catch (...) {
        m_cursors[source_char] = m_c.generate_id();
        m_c.create_glyph_cursor(m_cursors[source_char], m_font, m_font,
                                source_char, source_char + 1,
                                0, 0, 0, 0xffff, 0xffff, 0xffff);
        return m_cursors[source_char];
      }
    }

  private:
    x::connection & m_c;
    xcb_font_t m_font;
    std::unordered_map<uint16_t, xcb_cursor_t> m_cursors;
}; // class cursors

}; // namespace x

#endif // X_CURSOR_HPP
