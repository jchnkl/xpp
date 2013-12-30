/*
 * vim macro for aligning backslashes:
 * f\:exe ":normal i" . repeat(' ', 79-col("."))j0
 * save to "a to repeat: @f@a
 */

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

#define NS_HEAD(NAME)                                                         \
namespace NAME {

#define NS_TAIL(NAME)                                                         \
}; /* NAME */

#define REPLY_REQUEST_CLASS_HEAD(NAME, C_NAME)                                \
class NAME : public generic::request<C_NAME ## _cookie_t,                     \
                                     C_NAME ## _reply_t,                      \
                                     &C_NAME ## _reply>                       \
{                                                                             \
  public:

#define REPLY_REQUEST_CLASS_BODY_CLASS(NAME, C_NAME, ...)                     \
  NAME(xcb_connection_t * c MACRO_DISPATCHER(TYPE_ARG_CC, __VA_ARGS__))

#define REPLY_REQUEST_CLASS_BODY_REQUEST(NAME, C_NAME, ...)                   \
    : request(c, &C_NAME MACRO_DISPATCHER(ARGN_PASTER, __VA_ARGS__))          \
{}



#define VOID_REQUEST_CLASS_HEAD(NAME, C_NAME)                                 \
class NAME {                                                                  \
  public:

#define VOID_REQUEST_CLASS_BODY_CLASS(NAME, C_NAME, ...)                      \
    NAME(xcb_connection_t * c MACRO_DISPATCHER(TYPE_ARG_CC, __VA_ARGS__))     \
    {                                                                         \
      C_NAME(c MACRO_DISPATCHER(ARGN_PASTER, __VA_ARGS__));                   \
    }

#define VOID_REQUEST_CLASS_BODY_REQUEST(NAME, C_NAME, ...)                    \
    void                                                                      \
    operator()(xcb_connection_t * c                                           \
               MACRO_DISPATCHER(TYPE_ARG_CC, __VA_ARGS__))                    \
    {                                                                         \
      C_NAME(c MACRO_DISPATCHER(ARGN_PASTER, __VA_ARGS__));                   \
    }



#define REQUEST_CLASS_TAIL(NAME)                                              \
}; /* class NAME */



#define REQUEST_FIXED_SIZE_LIST_ACCESSOR(MEMBER, TYPE, C_NAME)                \
  typedef xpp::generic::fixed_size::iterator<                                 \
              TYPE,                                                           \
              C_NAME ## _reply_t,                                             \
              C_NAME ## _ ## MEMBER,                                          \
              C_NAME ## _ ## MEMBER ## _length>                               \
            MEMBER ## _iterator;                                              \
                                                                              \
  xpp::generic::list<C_NAME ## _reply_t, MEMBER ## _iterator>                 \
  MEMBER(void)                                                                \
  {                                                                           \
    return xpp::generic::list<C_NAME ## _reply_t,                             \
                              MEMBER ## _iterator>(this->get());              \
  }



#define REQUEST_VARIABLE_SIZE_LIST_ACCESSOR(MEMBER, TYPE, ITER_NAME, C_NAME)  \
  typedef xpp::generic::variable_size::iterator<                              \
                TYPE,                                                         \
                C_NAME ## _reply_t,                                           \
                ITER_NAME ## _iterator_t,                                     \
                &ITER_NAME ## _next,                                          \
                &ITER_NAME ## _sizeof,                                        \
                &C_NAME ## _ ## MEMBER ## _iterator>                          \
            MEMBER ## _iterator;                                              \
                                                                              \
  xpp::generic::list<C_NAME ## _reply_t, MEMBER ## _iterator>                 \
  MEMBER(void)                                                                \
  {                                                                           \
    return xpp::generic::list<C_NAME ## _reply_t,                             \
                              MEMBER ## _iterator>(this->get());              \
  }



#define REQUEST_STRING_ACCESSOR(MEMBER, C_NAME)                               \
  xpp::generic::string<C_NAME ## _reply_t,                                    \
                       &C_NAME ## _ ## MEMBER,                                \
                       &C_NAME ## _ ## MEMBER ## _length>                     \
  MEMBER(void)                                                                \
  {                                                                           \
    return xpp::generic::string<C_NAME ## _reply_t,                           \
                                &C_NAME ## _ ## MEMBER,                       \
                                &C_NAME ## _ ## MEMBER ## _length>            \
                                  (this->get());                              \
  }

#define TYPE_CLASS(NAME)



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
