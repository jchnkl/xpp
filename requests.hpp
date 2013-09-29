#ifndef X_REQUESTS_HPP
#define X_REQUESTS_HPP

#include "request.hpp"

namespace x {

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

}; // namespace request

}; // namespace x

#endif // X_REQUESTS_HPP
