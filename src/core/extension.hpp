#ifndef XPP_EXTENSION_HPP
#define XPP_EXTENSION_HPP

#include "core.hpp"

namespace xpp {

template<xcb_extension_t * ID>
class extension {
  public:
    extension(const core & core)
      : m_core(core)
    {
      m_core.prefetch_extension_data(ID);
    }

    void get(void)
    {
      m_extension = m_core.get_extension_data(ID);
    }

    uint8_t major_opcode(void)
    {
      return m_extension->major_opcode;
    }

    uint8_t first_event(void)
    {
      return m_extension->first_event;
    }

    uint8_t first_error(void)
    {
      return m_extension->first_error;
    }

  private:
    const core & m_core;
    // The result must not be freed.
    // This storage is managed by the cache itself.
    const xcb_query_extension_reply_t * m_extension;
}; // class extension

}; // namespace xpp

#endif // XPP_EXTENSION_HPP
