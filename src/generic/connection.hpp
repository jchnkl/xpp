#ifndef XPP_GENERIC_CONNECTION_HPP
#define XPP_GENERIC_CONNECTION_HPP

namespace xpp { namespace generic {

template<typename Connection>
class connection {
  public:
    virtual
    operator Connection(void) const = 0;

    virtual
    operator Connection(void)
    {
      return const_cast<const connection<Connection> &>(*this);
    }
};

}; }; // namespace xpp::generic

#endif // XPP_GENERIC_CONNECTION_HPP
