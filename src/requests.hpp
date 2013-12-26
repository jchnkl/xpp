#ifndef X_REQUESTS_HPP
#define X_REQUESTS_HPP

#include "macros.hpp"
#include "request.hpp"
#include "request_macros.hpp"

namespace xpp {

namespace request {

SIMPLE_REQUEST(core, get_window_attributes, xcb_window_t, window)

SIMPLE_REQUEST(core, get_geometry, xcb_window_t, window)

ITERATOR_SPECIALIZED_REQUEST(core, query_tree, children, xcb_window_t,
                             xcb_window_t, window)

REQUEST_PROT(core, intern_atom, bool, only_if_exists, const std::string &, name)
REQUEST_BODY(core, intern_atom, bool, only_if_exists, const std::string &, name),
      static_cast<uint8_t>(only_if_exists),
      static_cast<uint16_t>(name.length()), name.c_str())
{}

SIMPLE_REQUEST(core, get_atom_name, xcb_atom_t, atom)

ITERATOR_TEMPLATE_REQUEST(core, get_property, value,
                          uint8_t, _delete, xcb_window_t, window,
                          xcb_atom_t, property, xcb_atom_t, type,
                          uint32_t, long_offset, uint32_t, long_length)

SIMPLE_REQUEST(core, grab_pointer,
               uint8_t, owner_events, xcb_window_t, grab_window,
               uint16_t, event_mask, uint8_t, pointer_mode,
               uint8_t, keyboard_mode, xcb_window_t, confine_to,
               xcb_cursor_t, cursor, xcb_timestamp_t, time)

}; // namespace request

}; // namespace xpp

#endif // X_REQUESTS_HPP
