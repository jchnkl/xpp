#ifndef XPP_WINDOW_HPP
#define XPP_WINDOW_HPP

#include "generic/resource.hpp"

namespace xpp {

template<typename Connection, template<typename, typename> class ... Interfaces>
class window
  : public xpp::generic::resource<Connection, xcb_window_t, Interfaces ...>
{
  protected:
    using base = xpp::generic::resource<Connection, xcb_window_t, Interfaces ...>;

  public:
    using base::base;
    using base::operator=;

    template<typename C>
    window(C && c, uint8_t depth, xcb_window_t parent,
                int16_t x, int16_t y, uint16_t width, uint16_t height,
                uint16_t border_width,
                uint16_t _class, xcb_visualid_t visual,
                uint32_t value_mask, const uint32_t * value_list)

      : base(std::forward<C>(c),
             [&](const xcb_window_t & window)
             {
               xcb_create_window(c, depth, window, parent,
                                 x, y, width, height, border_width,
                                 _class, visual, value_mask, value_list);
             },
             [&](const xcb_window_t & window)
             {
               xcb_destroy_window(c, window);
             })
    {}
};

namespace generic {

template<typename Connection, template<typename, typename> class ... Interfaces>
struct traits<xpp::window<Connection, Interfaces ...>>
{
  typedef xcb_window_t type;
};

}; // namespace generic

}; // namespace xpp

#endif // XPP_WINDOW_HPP
