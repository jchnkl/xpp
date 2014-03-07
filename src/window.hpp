#ifndef XPP_WINDOW_HPP
#define XPP_WINDOW_HPP

#include <map>
#include <vector>

#include "proto/xproto.hpp"

namespace xpp {

template<typename Connection>
class window : virtual public xpp::x::window<Connection>
             , virtual public xpp::x::drawable<Connection>
             , virtual public xpp::iterable<void>
             , virtual public xpp::iterable<xcb_window_t>
             , virtual protected xpp::generic::connection<Connection>
{
  protected:

    virtual Connection get(void)
    {
      return m_c;
    }

  public:

    static std::size_t size_of(void)
    {
      return sizeof(xcb_window_t);
    }

    window(const Connection & c)
      : m_c(c)
    {}

    window(const Connection & c, const xcb_window_t & window)
      : m_c(c)
      , m_window(std::make_shared<xcb_window_t>(window))
    {}

    window(const xcb_window_t & window, const Connection & c)
      : xpp::window<Connection>(c, window)
    {}

    window(const Connection & c,
           uint8_t depth, xcb_window_t parent, int16_t x, int16_t y,
           uint16_t width, uint16_t height, uint16_t border_width,
           uint16_t _class, xcb_visualid_t visual,
           uint32_t value_mask, const uint32_t * value_list)
      : m_c(c)
      , m_window(new xcb_window_t(xcb_generate_id(c)),
                                                [&](xcb_window_t * window)
                                                {
                                                  xpp::x::destroy_window(m_c, *window);
                                                  delete window;
                                                })
    {
      xpp::x::create_window(
          m_c, depth, *m_window, parent, x, y,
          width, height, border_width, _class, visual, value_mask, value_list);
    }

    // xpp::iterable<void * const>
    virtual
    void
    operator=(void * const data)
    {
      m_window =
        std::make_shared<xcb_window_t>(*static_cast<xcb_window_t *>(data));
    }

    virtual
    const xcb_window_t &
    operator*(void) const
    {
      return *m_window;
    }

    // xpp::iterable<const xcb_window_t &>
    virtual
    void
    operator=(xcb_window_t window)
    {
      m_window = std::make_shared<xcb_window_t>(window);
    }

    // xpp::xcb::type<const xcb_window_t &>
    virtual
    operator const xcb_window_t &(void) const
    {
      return *m_window;
    }

    // xpp::xcb::type<xcb_connection_t * const>
    virtual
    operator Connection(void) const
    {
      return m_c;
    }

    virtual
    window &
    x(unsigned int x)
    {
      m_mask |= XCB_CONFIG_WINDOW_X;
      m_values[XCB_CONFIG_WINDOW_X] = x;
      return *this;
    }

    virtual
    window &
    y(unsigned int y)
    {
      m_mask |= XCB_CONFIG_WINDOW_Y;
      m_values[XCB_CONFIG_WINDOW_Y] = y;
      return *this;
    }

    virtual
    window &
    width(unsigned int width)
    {
      m_mask |= XCB_CONFIG_WINDOW_WIDTH;
      m_values[XCB_CONFIG_WINDOW_WIDTH] = width;
      return *this;
    }

    virtual
    window &
    height(unsigned int height)
    {
      m_mask |= XCB_CONFIG_WINDOW_HEIGHT;
      m_values[XCB_CONFIG_WINDOW_HEIGHT] = height;
      return *this;
    }

    virtual
    window &
    border_width(unsigned int border_width)
    {
      m_mask |= XCB_CONFIG_WINDOW_BORDER_WIDTH;
      m_values[XCB_CONFIG_WINDOW_BORDER_WIDTH] = border_width;
      return *this;
    }

    virtual
    window &
    sibling(unsigned int sibling)
    {
      m_mask |= XCB_CONFIG_WINDOW_SIBLING;
      m_values[XCB_CONFIG_WINDOW_SIBLING] = sibling;
      return *this;
    }

    virtual
    window &
    stack_mode(unsigned int stack_mode)
    {
      m_mask |= XCB_CONFIG_WINDOW_STACK_MODE;
      m_values[XCB_CONFIG_WINDOW_STACK_MODE] = stack_mode;
      return *this;
    }

    virtual
    window &
    configure(void)
    {
      std::vector<unsigned int> values;
      for (auto & item : m_values) {
        values.push_back(item.second);
      }
      xpp::x::window<Connection>::configure(m_mask, values.data());
      return *this;
    }

  private:
    Connection m_c;
    // reference counting for xcb_window_t object
    std::shared_ptr<xcb_window_t> m_window;

    unsigned int m_mask = 0;
    std::map<unsigned int, unsigned int> m_values;

}; // class window

template<typename Connection>
std::ostream &
operator<<(std::ostream & os, const xpp::window<Connection> & window)
{
  return os << std::hex << "0x" << *window << std::dec;
}

}; // namespace xpp

#endif // XPP_WINDOW_HPP
