#ifndef XPP_XCB_TYPE_HPP
#define XPP_XCB_TYPE_HPP

#include <cstddef> // size_t

namespace xpp {

namespace xcb {

template<typename Type>
class type {
  public:
    virtual ~type(void) {}
    virtual operator Type(void) const = 0;
}; // class type

}; // namespace xcb

}; // namespace xpp

#endif // XPP_XCB_TYPE_HPP
