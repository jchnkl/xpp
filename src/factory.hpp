#ifndef XPP_GENERIC_FACTORY_HPP
#define XPP_GENERIC_FACTORY_HPP

#include <utility> // std::forward

namespace xpp { namespace generic {

namespace factory {

template<typename ReturnType>
class make_object {
  public:
    template<typename Member, typename Connection, typename ... Parameter>
    ReturnType
    operator()(Member && member,
               Connection &&,
               Parameter && ... parameter)
    {
      return ReturnType { std::forward<Member>(member)
                        , std::forward<Parameter>(parameter) ... };
    }
};

template<typename ReturnType>
class make_object_with_connection {
  public:
    template<typename Member, typename Connection, typename ... Parameter>
    ReturnType
    operator()(Member && member,
               Connection && c,
               Parameter && ... parameter)
    {
      return ReturnType { std::forward<Member>(member)
                        , std::forward<Connection>(c)
                        , std::forward<Parameter>(parameter) ...
                        };
    }
};

template<typename ReturnType>
class make_fundamental {
  public:
    template<typename Member, typename Connection, typename ... Parameter>
    ReturnType
    operator()(Member && member, Connection &&)
    {
      return std::forward<Member>(member);
    }
};

template<typename Connection,
         typename MemberType,
         typename ReturnType,
         typename ... Parameter>
class make
  : public std::conditional<
               std::is_constructible<ReturnType, MemberType>::value,
               make_fundamental<ReturnType>,
               typename std::conditional<
                            std::is_constructible<ReturnType,
                                                  MemberType,
                                                  Connection,
                                                  Parameter ...>::value,
                            make_object_with_connection<ReturnType>,
                            make_object<ReturnType>
                        >::type
           >::type
{};

}; // namespace factory

}; }; // xpp::generic

#endif // XPP_GENERIC_FACTORY_HPP
