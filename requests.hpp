#ifndef X_REQUESTS_HPP
#define X_REQUESTS_HPP

#include "request.hpp"

namespace x {

namespace request {

class query_tree
  : generic::request<xcb_query_tree_cookie_t,
                     xcb_query_tree_reply_t,
                     &xcb_query_tree_reply>
{
  public:
    query_tree(xcb_connection_t * c, xcb_window_t window)
      : request(c, &xcb_query_tree, window)
    {}
};

}; // namespace request

}; // namespace x

#endif // X_REQUESTS_HPP
