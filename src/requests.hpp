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

SIMPLE_REQUEST(core, get_window_attributes, xcb_window_t, window)

SIMPLE_REQUEST(core, get_geometry, xcb_window_t, window)

namespace core {

class query_tree : public generic::request<xcb_query_tree_cookie_t,
  xcb_query_tree_reply_t,
  &xcb_query_tree_reply>
{
  public:
    class iterator {
      public:
        iterator(xcb_query_tree_reply_t * const reply, bool begin)
          : m_reply(reply)
        {
          m_elements = xcb_query_tree_children(m_reply);
          if (! begin) m_position = m_reply->children_len;
        }

        bool operator==(const iterator & other)
        {
          return m_position == other.m_position;
        }

        bool operator!=(const iterator & other)
        {
          return ! (*this == other);
        }

        const xcb_window_t & operator*(void)
        {
          return m_elements[m_position];
        }

        // prefix
        iterator & operator++(void)
        {
          ++m_position;
          return *this;
        }

        // postfix
        iterator operator++(int)
        {
          auto copy = *this;
          ++(*this);
          return copy;
        }

        // prefix
        iterator & operator--(void)
        {
          --m_position;
          return *this;
        }

        // postfix
        iterator operator--(int)
        {
          auto copy = *this;
          --(*this);
          return copy;
        }

      private:
        std::size_t m_position = 0;
        xcb_window_t * m_elements = NULL;
        xcb_query_tree_reply_t * m_reply;
    };

    query_tree(xcb_connection_t * c, xcb_window_t window)
      : request(c, &xcb_query_tree, window)
    {}

    iterator begin(void)
    {
      return iterator(this->get().get(), true);
    }

    iterator end(void)
    {
      return iterator(this->get().get(), false);
    }
};

};

REQUEST_PROT(core, intern_atom, bool, only_if_exists, const std::string &, name)
REQUEST_BODY(core, intern_atom, bool, only_if_exists, const std::string &, name)
      static_cast<uint8_t>(only_if_exists),
      static_cast<uint16_t>(name.length()), name.c_str())
{}

SIMPLE_REQUEST(core, get_atom_name, xcb_atom_t, atom)

SIMPLE_REQUEST(core, get_property, uint8_t, _delete, xcb_window_t, window,
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