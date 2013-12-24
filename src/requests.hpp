#ifndef X_REQUESTS_HPP
#define X_REQUESTS_HPP

#include "macros.hpp"
#include "request.hpp"

#define REQUEST_PROT(NAMESPACE, NAME, ...)                                    \
namespace NAMESPACE {                                                         \
class NAME : public generic::request<xcb_ ## NAME ## _cookie_t,               \
                            xcb_ ## NAME ## _reply_t,                         \
                            &xcb_ ## NAME ## _reply>                          \
{                                                                             \
  public:                                                                     \
    NAME(xcb_connection_t * c, MACRO_DISPATCHER(TYPE_ARG_CC, __VA_ARGS__));   \
};                                                                            \
};

#define REQUEST_BODY(NAMESPACE, NAME, ...)                                    \
NAMESPACE::NAME::NAME(xcb_connection_t * c,                                   \
                      MACRO_DISPATCHER(TYPE_ARG_CC, __VA_ARGS__))             \
  : request(c, &xcb_ ## NAME,

#define SIMPLE_REQUEST(NAMESPACE, NAME, ...)                                  \
  REQUEST_PROT(NAMESPACE, NAME, __VA_ARGS__)                                  \
  REQUEST_BODY(NAMESPACE, NAME, __VA_ARGS__)                                  \
  MACRO_DISPATCHER(ARGN_PASTER, __VA_ARGS__))                                 \
{}

namespace xpp {

namespace request {

class get_window_attributes
  : public generic::request<xcb_get_window_attributes_cookie_t,
                            xcb_get_window_attributes_reply_t,
                            &xcb_get_window_attributes_reply>
{
  public:
    get_window_attributes(xcb_connection_t * c, xcb_window_t window)
      : request(c, &xcb_get_window_attributes, window)
    {}
};

class get_geometry
  : public generic::request<xcb_get_geometry_cookie_t,
                            xcb_get_geometry_reply_t,
                            &xcb_get_geometry_reply>
{
  public:
    get_geometry(xcb_connection_t * c, xcb_window_t window)
      : request(c, &xcb_get_geometry, window)
    {}
};

class query_tree
  : public generic::request<xcb_query_tree_cookie_t,
                            xcb_query_tree_reply_t,
                            &xcb_query_tree_reply>
{
  public:
    query_tree(xcb_connection_t * c, xcb_window_t window)
      : request(c, &xcb_query_tree, window)
    {}
};

class intern_atom
  : public generic::request<xcb_intern_atom_cookie_t,
                            xcb_intern_atom_reply_t,
                            &xcb_intern_atom_reply>
{
  public:
    intern_atom(xcb_connection_t * c, bool only_if_exists, const std::string & name)
      : request(c, &xcb_intern_atom,
                static_cast<uint8_t>(only_if_exists),
                static_cast<uint16_t>(name.length()), name.c_str())
    {}
};

class get_atom_name
  : public generic::request<xcb_get_atom_name_cookie_t,
                            xcb_get_atom_name_reply_t,
                            &xcb_get_atom_name_reply>
{
  public:
    get_atom_name(xcb_connection_t * c, xcb_atom_t atom)
      : request(c, &xcb_get_atom_name, atom)
    {}
};

class get_property
  : public generic::request<xcb_get_property_cookie_t,
                            xcb_get_property_reply_t,
                            &xcb_get_property_reply>
{
  public:
    get_property(xcb_connection_t *c, bool _delete, xcb_window_t window,
                 xcb_atom_t property, xcb_atom_t type, uint32_t long_offset,
                 uint32_t long_length)
      : request(c, &xcb_get_property, static_cast<uint8_t>(_delete), window,
                property, type, long_offset, long_length)
    {}
};

class grab_pointer
  : public generic::request<xcb_grab_pointer_cookie_t,
                            xcb_grab_pointer_reply_t,
                            &xcb_grab_pointer_reply>
{
  public:
    grab_pointer(xcb_connection_t * c, uint8_t owner_events,
                 xcb_window_t grab_window, uint16_t event_mask,
                 uint8_t pointer_mode, uint8_t keyboard_mode,
                 xcb_window_t confine_to, xcb_cursor_t cursor,
                 xcb_timestamp_t time)
      : request(c, &xcb_grab_pointer, owner_events, grab_window, event_mask,
                pointer_mode, keyboard_mode, confine_to, cursor, time)
  {}
};

}; // namespace request

}; // namespace xpp

#endif // X_REQUESTS_HPP
