#ifndef X_DRAWABLE_HPP
#define X_DRAWABLE_HPP

#include "connection.hpp"

namespace xpp {

template<typename DRAWABLE>
class drawable {
  public:
    drawable(connection & c, DRAWABLE drawable)
      : m_c(c), m_drawable(drawable)
    {}

    DRAWABLE
    id(void) const
    {
      return m_drawable;
    }

  protected:
    connection & m_c;
    DRAWABLE m_drawable;
}; // class drawable

}; // namespace xpp

#endif // X_DRAWABLE_HPP
