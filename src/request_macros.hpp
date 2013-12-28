#ifndef X_REQUEST_MACROS_HPP
#define X_REQUEST_MACROS_HPP

#define REQUEST_PROT(NAMESPACE, NAME, ...)                                    \
namespace NAMESPACE {                                                         \
class NAME : public generic::request<xcb_ ## NAME ## _cookie_t,               \
                            xcb_ ## NAME ## _reply_t,                         \
                            &xcb_ ## NAME ## _reply>                          \
{                                                                             \
  public:                                                                     \
    NAME(xcb_connection_t * c MACRO_DISPATCHER(TYPE_ARG_CC, __VA_ARGS__));    \
};                                                                            \
};

#define REQUEST_BODY(NAMESPACE, NAME, ...)                                    \
NAMESPACE::NAME::NAME(xcb_connection_t * c                                    \
                      MACRO_DISPATCHER(TYPE_ARG_CC, __VA_ARGS__))             \
  : request(c, &xcb_ ## NAME

#define SIMPLE_REQUEST(NAMESPACE, NAME, ...)                                  \
  REQUEST_PROT(NAMESPACE, NAME, __VA_ARGS__)                                  \
  REQUEST_BODY(NAMESPACE, NAME, __VA_ARGS__)                                  \
  MACRO_DISPATCHER(ARGN_PASTER, __VA_ARGS__))                                 \
{}

#define REQUEST_CLASS_HEAD(NAME, C_NAME)                                      \
class NAME : public generic::request<C_NAME ## _cookie_t,                     \
                                     C_NAME ## _reply_t,                      \
                                     &C_NAME ## _reply>                       \
{                                                                             \
  public:

#define REQUEST_CLASS_BODY(NAME, C_NAME, ...)                                 \
  NAME(xcb_connection_t * c MACRO_DISPATCHER(TYPE_ARG_CC, __VA_ARGS__))       \
    : request(c, &C_NAME MACRO_DISPATCHER(ARGN_PASTER, __VA_ARGS__))          \
{}

#define REQUEST_CLASS_TAIL(NAME)                                              \
}; /* class NAME */

#define ITERATOR_REQUEST_PROTO_COMMON(NAME, MEMBER, TYPE, ...)\
class NAME : public generic::request<xcb_ ## NAME ## _cookie_t,               \
                                     xcb_ ## NAME ## _reply_t,                \
                                     &xcb_ ## NAME ## _reply>                 \
{                                                                             \
  public:                                                                     \
    ITERATOR(NAME, MEMBER)                                           \
    NAME(xcb_connection_t * c MACRO_DISPATCHER(TYPE_ARG_CC, __VA_ARGS__));    \
                                                                              \
    iterator<TYPE> begin(void)                                         \
    {                                                                         \
      return iterator<TYPE>(this->get().get(), true);                  \
    }                                                                         \
                                                                              \
    iterator<TYPE> end(void)                                           \
    {                                                                         \
      return iterator<TYPE>(this->get().get(), false);                 \
    }                                                                         \
}; /* class NAME */                                                           \
}; /* namespace NAMESPACE */

#define ITERATOR_TEMPLATE_REQUEST_PROTO(NAMESPACE, NAME, MEMBER, ...)         \
namespace NAMESPACE {                                                         \
template<typename IteratorMember>                                             \
  ITERATOR_REQUEST_PROTO_COMMON(NAME, MEMBER, IteratorMember, __VA_ARGS__)

#define ITERATOR_SPECIALIZED_REQUEST_PROTO(NAMESPACE, NAME, MEMBER, TYPE, ...)\
namespace NAMESPACE {                                                         \
  ITERATOR_REQUEST_PROTO_COMMON(NAME, MEMBER, TYPE, __VA_ARGS__)

#define ITERATOR_SPECIALIZED_REQUEST(NAMESPACE, NAME, MEMBER, TYPE, ...)      \
ITERATOR_SPECIALIZED_REQUEST_PROTO(NAMESPACE, NAME, MEMBER, TYPE, __VA_ARGS__)\
NAMESPACE::NAME::NAME(                                                        \
    xcb_connection_t * c MACRO_DISPATCHER(TYPE_ARG_CC, __VA_ARGS__))          \
      : request(c, &xcb_ ## NAME MACRO_DISPATCHER(ARGN_PASTER, __VA_ARGS__))  \
{}

#define ITERATOR_TEMPLATE_REQUEST(NAMESPACE, NAME, MEMBER, ...)               \
  ITERATOR_TEMPLATE_REQUEST_PROTO(NAMESPACE, NAME, MEMBER, __VA_ARGS__)       \
template<typename IteratorMember>                                             \
NAMESPACE::NAME<IteratorMember>::NAME(                                        \
    xcb_connection_t * c MACRO_DISPATCHER(TYPE_ARG_CC, __VA_ARGS__))          \
  : request(c, &xcb_ ## NAME MACRO_DISPATCHER(ARGN_PASTER, __VA_ARGS__))      \
{}

#endif // X_REQUEST_MACROS_HPP
