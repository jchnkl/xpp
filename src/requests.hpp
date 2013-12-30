#ifndef X_REQUESTS_HPP
#define X_REQUESTS_HPP

#include <vector>
#include <xcb/randr.h>

#include "macros.hpp"
#include "request.hpp"
#include "request_macros.hpp"
#include "request_iterator.hpp"

namespace xpp {

namespace request {

SIMPLE_REQUEST(core, get_window_attributes, xcb_window_t, window)

SIMPLE_REQUEST(core, get_geometry, xcb_window_t, window)

// ITERATOR_SPECIALIZED_REQUEST(core, query_tree, children, xcb_window_t,
//                              xcb_window_t, window)

REQUEST_PROT(core, intern_atom, bool, only_if_exists, const std::string &, name)
REQUEST_BODY(core, intern_atom, bool, only_if_exists, const std::string &, name),
      static_cast<uint8_t>(only_if_exists),
      static_cast<uint16_t>(name.length()), name.c_str())
{}

SIMPLE_REQUEST(core, get_atom_name, xcb_atom_t, atom)

// ITERATOR_TEMPLATE_REQUEST(core, get_property, value,
//                           uint8_t, _delete, xcb_window_t, window,
//                           xcb_atom_t, property, xcb_atom_t, type,
//                           uint32_t, long_offset, uint32_t, long_length)

SIMPLE_REQUEST(core, get_property,
               uint8_t, _delete, xcb_window_t, window,
               xcb_atom_t, property, xcb_atom_t, type,
               uint32_t, long_offset, uint32_t, long_length)

SIMPLE_REQUEST(core, grab_pointer,
               uint8_t, owner_events, xcb_window_t, grab_window,
               uint16_t, event_mask, uint8_t, pointer_mode,
               uint8_t, keyboard_mode, xcb_window_t, confine_to,
               xcb_cursor_t, cursor, xcb_timestamp_t, time)

// SIMPLE_REQUEST(core, get_input_focus)

REQUEST_NS_HEAD(core)
REQUEST_CLASS_HEAD(get_input_focus)
REQUEST_CLASS_BODY(get_input_focus)
REQUEST_CLASS_TAIL(get_input_focus)
REQUEST_NS_TAIL(core)

// SIMPLE_REQUEST(core, list_fonts_with_info, uint16_t, max_names,
//                uint16_t, pattern_len, const char*, pattern)

// REQUEST_PROT(core, list_fonts_with_info, uint16_t, max_names, const std::string &, pattern)
// REQUEST_BODY(core, list_fonts_with_info, uint16_t, max_names, const std::string &, pattern),
//       max_names, static_cast<uint16_t>(pattern.length()), pattern.c_str())
// {}

namespace randr {

class get_output_primary : public generic::request<
                        xcb_randr_get_output_primary_cookie_t,
                        xcb_randr_get_output_primary_reply_t,
                        &xcb_randr_get_output_primary_reply>
{
  public:
    get_output_primary(xcb_connection_t * c, xcb_window_t window);

    // template<typename T>
    // iterator<T> begin(void)
    // {
    //   throw std::logic_error("No template specialization for " + __PRETTY_FUNCTION__);
    // }

    // template<typename T>
    // iterator<T> end(void)
    // {
    //   throw std::logic_error("No template specialization for " + __PRETTY_FUNCTION__);
    // }

}; // class get_output_primary

get_output_primary::get_output_primary(xcb_connection_t * c, xcb_window_t window)
  : request(c, &xcb_randr_get_output_primary, window)
{}

// template<typename Data>
         // typename Reply,
         // Data * (*Accessor)(const Reply *),
         // int (*Length)(const Reply *)>
// class container {
// 
//   protected:
//     const std::vector<Data> &
//     // data(const Reply * reply)
//     data(const Data * data, int length)
//     {
//       if (m_data.empty()) {
//         // m_data = std::vector<Data>(Accessor(reply), reply + Length(reply));
//         m_data = std::vector<Data>(data, data + length);
//       }
//       return m_data;
//     }
// 
//   private:
//     std::vector<Data> m_data;
// };

class get_output_info
  : public generic::request<
    xcb_randr_get_output_info_cookie_t,
    xcb_randr_get_output_info_reply_t,
    &xcb_randr_get_output_info_reply>

{
  public:
    get_output_info(xcb_connection_t * c, xcb_randr_output_t output, xcb_timestamp_t config_timestamp = XCB_TIME_CURRENT_TIME);

    container<
      xcb_randr_crtc_t, xcb_randr_get_output_info_reply_t,
      &xcb_randr_get_output_info_crtcs,
      &xcb_randr_get_output_info_crtcs_length>
    crtcs(void)
    {
      return
        container<
        xcb_randr_crtc_t, xcb_randr_get_output_info_reply_t,
        &xcb_randr_get_output_info_crtcs,
        &xcb_randr_get_output_info_crtcs_length>(*(*this));
    }

    container<
      xcb_randr_mode_t, xcb_randr_get_output_info_reply_t,
      &xcb_randr_get_output_info_modes,
      &xcb_randr_get_output_info_modes_length>
    modes(void)
    {
      return
        container<
        xcb_randr_mode_t, xcb_randr_get_output_info_reply_t,
        &xcb_randr_get_output_info_modes,
        &xcb_randr_get_output_info_modes_length>(*(*this));
    }

    container<
      xcb_randr_output_t, xcb_randr_get_output_info_reply_t,
      &xcb_randr_get_output_info_clones,
      &xcb_randr_get_output_info_clones_length>
    clones(void)
    {
      return
        container<
        xcb_randr_output_t, xcb_randr_get_output_info_reply_t,
        &xcb_randr_get_output_info_clones,
        &xcb_randr_get_output_info_clones_length>(*(*this));
    }

}; // class get_output_info

get_output_info::get_output_info(xcb_connection_t * c, xcb_randr_output_t output, xcb_timestamp_t config_timestamp)
  : request(c, &xcb_randr_get_output_info, output, config_timestamp)
{}

}; // namespace randr

REQUEST_NS_HEAD(core)
REQUEST_CLASS_HEAD(query_tree)
REQUEST_CLASS_BODY(query_tree, xcb_window_t, window)
REQUEST_CLASS_LIST_ACCESSOR(query_tree, children, xcb_window_t)
REQUEST_CLASS_TAIL(query_tree)
REQUEST_NS_TAIL(core)

namespace core {

// class query_tree : public generic::request<
//                              xcb_query_tree_cookie_t,
//                              xcb_query_tree_reply_t,
//                              &xcb_query_tree_reply>
// {
//   public:
//     query_tree(xcb_connection_t * c, xcb_window_t window)
//       : request(c, &xcb_query_tree, window)
//     {}
// 
//     container<xcb_window_t, xcb_query_tree_reply_t,
//               &xcb_query_tree_children,
//               &xcb_query_tree_children_length>
//     children(void)
//     {
//       return container<xcb_window_t, xcb_query_tree_reply_t,
//                        &xcb_query_tree_children,
//                        &xcb_query_tree_children_length>
//                         (*(*this));
//     }
// 
// }; // class query_tree

class list_fonts_with_info : public generic::request<
                             xcb_list_fonts_with_info_cookie_t,
                             xcb_list_fonts_with_info_reply_t,
                             &xcb_list_fonts_with_info_reply>
{
  public:
    list_fonts_with_info(xcb_connection_t * c,
        uint16_t max_names, const std::string & pattern);
};

};

core::list_fonts_with_info::list_fonts_with_info(
    xcb_connection_t * c, uint16_t max_names, const std::string & pattern)
  : request(c, &xcb_list_fonts_with_info,
      max_names, static_cast<uint16_t>(pattern.length()), pattern.c_str())
{}

#include "gen/xproto_requests.hpp"

}; // namespace request

}; // namespace xpp

#endif // X_REQUESTS_HPP
