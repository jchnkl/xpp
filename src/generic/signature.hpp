#ifndef XPP_GENERIC_SIGNATURE_HPP
#define XPP_GENERIC_SIGNATURE_HPP

namespace xpp { namespace generic {

template<typename Signature, Signature & S>
class signature;

#define SIGNATURE(NAME) \
  xpp::generic::signature<decltype(NAME), NAME>

}; };

#endif // XPP_GENERIC_SIGNATURE_HPP
