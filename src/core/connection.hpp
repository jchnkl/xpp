#ifndef XPP_CONNECTION_HPP
#define XPP_CONNECTION_HPP

#include "core.hpp"

namespace xpp {

template<typename ... Protocols>
class connection
  : virtual public core
  , virtual public Protocols ...
{
  public:
    template<typename ... Parameters>
    connection(Parameters ... parameters)
      : core::core(parameters ...)
      , Protocols(static_cast<const core &>(*this)) ...
    {}
}; // class connection

}; // namespace xpp

#endif // XPP_CONNECTION_HPP
