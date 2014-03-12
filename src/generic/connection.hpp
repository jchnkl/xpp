#ifndef XPP_GENERIC_CONNECTION_HPP
#define XPP_GENERIC_CONNECTION_HPP

namespace xpp { namespace generic {

template<typename Connection>
class connection {
  public:
    virtual Connection get(void) = 0;
};

}; }; // namespace xpp::generic

#endif // XPP_GENERIC_CONNECTION_HPP
