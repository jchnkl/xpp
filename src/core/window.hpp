#ifndef XPP_WINDOW_HPP
#define XPP_WINDOW_HPP

#include <map>
// #include <iostream>

#include "../iterator.hpp"
#include "../gen/protos.hpp"

namespace xpp {

// // xproto
// class drawable {};
// class pixmap {};
// // class window {};
// class atom {};
// class cursor {};
// class font {};
// class gcontext {};
// class fontable {};

// // randr
// class mode {};
// class crtc {};
// class output {};
// class provider {};

class window : virtual public xpp::resource::window::x
             , virtual public xpp::resource::drawable::x
             , virtual public xpp::iterable<void>
             , virtual public xpp::iterable<xcb_window_t>
{
  public:

    // window(void)
    // {}
    static std::size_t size_of(void)
    {
      return sizeof(xcb_window_t);
    }

    window(xcb_connection_t * c)
      : m_c(c)
    {}

    window(xcb_connection_t * c, const xcb_window_t & window)
      : m_c(c)
      , m_window(std::make_shared<xcb_window_t>(window))
    {}

    window(xcb_connection_t * c,
           uint8_t depth, xcb_window_t parent, int16_t x, int16_t y,
           uint16_t width, uint16_t height, uint16_t border_width,
           uint16_t _class, xcb_visualid_t visual,
           uint32_t value_mask, const uint32_t * value_list)
      : m_c(c)
      , m_window(new xcb_window_t(xcb_generate_id(c)),
                                                [&](xcb_window_t * window)
                                                {
                                                  destroy();
                                                  delete window;
                                                })
    {
      xpp::request::x::create_window()(
          m_c, depth, *m_window, parent, x, y,
          width, height, border_width, _class, visual, value_mask, value_list);
    }

    // // xpp::iterable<void * const>
    // virtual
    // std::size_t size_of(void)
    // {
    //   return sizeof(xcb_window_t);
    // }

    // xpp::iterable<void * const>
    virtual
    void
    operator=(void * const data)
    {
      // xcb_window_t * windows = static_cast<xcb_window_t *>(data);
      m_window =
        std::make_shared<xcb_window_t>(*static_cast<xcb_window_t *>(data));
        // std::make_shared<xcb_window_t>(*static_cast<xcb_window_t *>(ptr));
      // return sizeof(xcb_window_t);
      // return *m_window;
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

    virtual
    void
    operator=(xcb_connection_t * const c)
    {
      m_c = c;
    }

    // xpp::xcb::type<const xcb_window_t &>
    virtual
    operator const xcb_window_t &(void) const
    {
      return *m_window;
    }

    // xpp::xcb::type<xcb_connection_t * const>
    virtual
    operator xcb_connection_t * const(void) const
    {
      return m_c;
    }

    static std::size_t size(void) { return sizeof(xcb_window_t); }

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
      xpp::resource::window::x::configure(m_mask, values.data());
      return *this;
    }

  private:
    xcb_connection_t * m_c;
    // reference counting for xcb_window_t object
    std::shared_ptr<xcb_window_t> m_window;

    unsigned int m_mask = 0;
    std::map<unsigned int, unsigned int> m_values;

}; // class window

std::ostream &
operator<<(std::ostream & os, const xpp::window & window)
{
  return os << std::hex << "0x" << *window << std::dec;
}

// template<typename Reply,
//          typename Accessor,
//          typename Length>
// class iterator<xcb_window_t, xpp::window, Reply, Accessor, Length>
//   : public iterator<iterator<xcb_window_t, xpp::window, Reply, Accessor, Length>,
//                     xcb_window_t, xpp::window, Reply, Accessor, Length>
// {
//   public:
//     typedef iterator<iterator<xcb_window_t, xpp::window, Reply, Accessor, Length>,
//                      xcb_window_t, xpp::window, Reply, Accessor, Length>
//                        base;
// 
//     iterator(xcb_connection_t * const c,
//              const std::shared_ptr<Reply> & reply,
//              std::size_t index)
//       : base(c, reply, index), m_window(c)
//     {}
// 
//     virtual
//     const xpp::window & operator*(void)
//     {
//       m_window = (static_cast<xcb_window_t *>(
//             Accessor()(this->m_reply.get()))[this->m_index]);
//       return m_window;
//     }
// 
//   private:
//     xpp::window m_window;
// 
// }; // class iterator

// template<typename Reply,
//          typename Accessor,
//          typename Length>
// class iterator<void, xpp::window, Reply, Accessor, Length>
//   : public iterator<xcb_window_t, xpp::window, Reply, Accessor, Length>
// {
//   public:
//     using iterator<xcb_window_t, xpp::window, Reply, Accessor, Length>::iterator;
// 
//     static
//     iterator
//     begin(xcb_connection_t * const c, const std::shared_ptr<Reply> & reply)
//     {
//       return iterator(c, reply, 0);
//     }
// 
//     static
//     iterator
//     end(xcb_connection_t * const c, const std::shared_ptr<Reply> & reply)
//     {
//       return iterator(c, reply, Length()(reply.get()) / sizeof(xcb_window_t));
//     }
// 
// }; // class iterator

}; // namespace xpp

#endif // XPP_WINDOW_HPP
