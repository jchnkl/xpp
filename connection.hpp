#ifndef X_CONNECTION_HPP
#define X_CONNECTION_HPP

#include "request.hpp"

namespace x {

class connection {
  public:
    connection(const std::string & displayname)
    {
      m_c = xcb_connect(displayname.c_str(), &m_default_screen_number);
    }

    ~connection(void)
    {
      xcb_disconnect(m_c);
    }

    xcb_connection_t * const operator*(void)
    {
      return m_c;
    }

  private:
    xcb_connection_t * m_c = NULL;
    int m_default_screen_number = 0;
};

};

#endif // X_CONNECTION_HPP
