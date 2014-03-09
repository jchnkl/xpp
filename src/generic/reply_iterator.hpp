#ifndef XPP_GENERIC_REPLY_ITERATOR_HPP
#define XPP_GENERIC_REPLY_ITERATOR_HPP

#include <cstdlib> // size_t
#include <memory>
#include <stack>
#include <xcb/xcb.h> // xcb_str_*
#include "type.hpp"
#include "iterable.hpp"
#include "signature.hpp"


#define NEXT_TEMPLATE \
  void (&Next)(XcbIterator *)

#define NEXT_SIGNATURE \
  xpp::generic::signature<void (XcbIterator *), Next>

#define SIZEOF_TEMPLATE \
  int (&SizeOf)(const void *)

#define SIZEOF_SIGNATURE \
  xpp::generic::signature<int (const void *), SizeOf>

#define GETITERATOR_TEMPLATE \
  XcbIterator (&GetIterator)(const Reply *)

#define GETITERATOR_SIGNATURE \
  xpp::generic::signature<XcbIterator (const Reply *), GetIterator>

#define ACCESSOR_TEMPLATE \
  Data * (&Accessor)(const Reply *)

#define ACCESSOR_SIGNATURE \
  xpp::generic::signature<Data * (const Reply *), Accessor>

#define LENGTH_TEMPLATE \
  int (&Length)(const Reply *)

#define LENGTH_SIGNATURE \
  xpp::generic::signature<int (const Reply *), Length>

namespace xpp {

namespace generic {


// http://bytes.com/topic/c/answers/692802-performance-static-member-function-versus-functor#post2754209
// functors are faster than static methods

}; // namespace generic

template<typename ... Arguments>
class iterator;

// abstract iterator for variable size data fields
// Derived: derived iterator class for CRTP

template<typename Derived,
         typename Connection,
         typename Data, // remove -> decltype
         typename ReturnData,
         typename Reply,
         typename XcbIterator,
         NEXT_TEMPLATE,
         SIZEOF_TEMPLATE,
         GETITERATOR_TEMPLATE>
class iterator<Derived,
               Connection,
               Data,
               ReturnData,
               NEXT_SIGNATURE,
               SIZEOF_SIGNATURE,
               GETITERATOR_SIGNATURE>
{
  public:
    iterator(void) {}

    template<typename C>
    iterator(C && c, const std::shared_ptr<Reply> & reply)
      : m_c(std::forward<C>(c))
      , m_reply(reply)
      , m_iterator(GetIterator(reply.get()))
    {}

    virtual
    bool
    operator==(const iterator & other)
    {
      return m_iterator.rem == other.m_iterator.rem;
    }

    virtual
    bool
    operator!=(const iterator & other)
    {
      return ! (*this == other);
    }

    virtual
    // const ReturnData &
    ReturnData
    operator*(void) = 0;

    // prefix
    virtual
    Derived &
    operator++(void)
    {
      m_lengths.push(SizeOf(m_iterator.data));
      Next(&m_iterator);
      return static_cast<Derived &>(*this);
    }

    // postfix
    virtual
    Derived
    operator++(int)
    {
      auto copy = static_cast<Derived &>(*this);
      ++(*this);
      return copy;
    }

    // prefix
    virtual
    Derived &
    operator--(void)
    {
      if (m_lengths.empty()) {
        Data * data = m_iterator.data;
        Data * prev = data - m_lengths.top();
        m_lengths.pop();

        m_iterator.index = (char *)m_iterator.data - (char *)prev;
        m_iterator.data = prev;
        ++m_iterator.rem;
      }

      return static_cast<Derived &>(*this);
    }

    // postfix
    virtual
    Derived
    operator--(int)
    {
      auto copy = static_cast<Derived &>(*this);
      --(*this);
      return copy;
    }

    template<typename C>
    static
    Derived
    begin(C && c, const std::shared_ptr<Reply> & reply)
    {
      return Derived(std::forward<C>(c), reply);
    }

    template<typename C>
    static
    Derived
    end(C && c, const std::shared_ptr<Reply> & reply)
    {
      auto it = Derived(std::forward<C>(c), reply);
      it.m_iterator.rem = 0;
      return it;
    }

  protected:
    Connection m_c;
    std::shared_ptr<Reply> m_reply;
    std::stack<std::size_t> m_lengths;
    XcbIterator m_iterator;
}; // class iterator

// general iterator for variable sized data fields

template<typename Connection,
         typename Data,
         typename ReturnData,
         typename Reply,
         typename XcbIterator,
         NEXT_TEMPLATE,
         SIZEOF_TEMPLATE,
         GETITERATOR_TEMPLATE>
class iterator<Connection,
               Data,
               ReturnData,
               NEXT_SIGNATURE,
               SIZEOF_SIGNATURE,
               GETITERATOR_SIGNATURE>
  : public xpp::iterator<
      // self
      iterator<Connection,
               Data,
               ReturnData,
               NEXT_SIGNATURE,
               SIZEOF_SIGNATURE,
               GETITERATOR_SIGNATURE>,
      // other args
      Connection,
      Data,
      ReturnData,
      NEXT_SIGNATURE,
      SIZEOF_SIGNATURE,
      GETITERATOR_SIGNATURE>
{
  public:
    typedef iterator<Connection,
                     Data,
                     ReturnData,
                     NEXT_SIGNATURE,
                     SIZEOF_SIGNATURE,
                     GETITERATOR_SIGNATURE>
                       self;

    typedef xpp::iterator<self,
                          Connection,
                          Data,
                          ReturnData,
                          NEXT_SIGNATURE,
                          SIZEOF_SIGNATURE,
                          GETITERATOR_SIGNATURE>
                            base;

    using base::base;

    virtual
    ReturnData
    operator*(void)
    {
      return *(static_cast<ReturnData *>(base::m_iterator.data));
    }
}; // class iterator

// specialized iterator for variable sized data fields which are strings

template<typename Connection,
         typename Reply,
         xcb_str_iterator_t (&GetIterator)(const Reply *)>
class iterator<Connection,
               xcb_str_t,
               xcb_str_t,
               SIGNATURE(xcb_str_next),
               SIGNATURE(xcb_str_sizeof),
               xpp::generic::signature<xcb_str_iterator_t (const Reply *), GetIterator>>
  : public iterator<
      // self
      iterator<Connection,
               xcb_str_t,
               xcb_str_t,
               SIGNATURE(xcb_str_next),
               SIGNATURE(xcb_str_sizeof),
               xpp::generic::signature<xcb_str_iterator_t (const Reply *), GetIterator>>,
      // other args
      Connection,
      xcb_str_t,
      std::string,
      SIGNATURE(xcb_str_next),
      SIGNATURE(xcb_str_sizeof),
      xpp::generic::signature<xcb_str_iterator_t (const Reply *), GetIterator>>
{
  public:
    typedef iterator<Connection,
                     xcb_str_t,
                     xcb_str_t,
                     SIGNATURE(xcb_str_next),
                     SIGNATURE(xcb_str_sizeof),
                     xpp::generic::signature<xcb_str_iterator_t (const Reply *), GetIterator>>
                       self;

    typedef iterator<self,
                     Connection,
                     xcb_str_t,
                     std::string,
                     SIGNATURE(xcb_str_next),
                     SIGNATURE(xcb_str_sizeof),
                     xpp::generic::signature<xcb_str_iterator_t (const Reply *), GetIterator>>
                       base;

    using base::base;

    std::string operator*(void)
    {
      return std::string((char *)xcb_str_name(base::m_iterator.data),
                         xcb_str_name_length(base::m_iterator.data));
    }

}; // class iterator

// abstract iterator for fixed size data fields
// Derived: derived iterator class for CRTP

template<typename Derived,
         typename Connection,
         typename ReturnData,
         typename Data,
         typename Reply,
         ACCESSOR_TEMPLATE,
         LENGTH_TEMPLATE>
class iterator<Derived,
               Connection,
               ReturnData,
               ACCESSOR_SIGNATURE,
               LENGTH_SIGNATURE>
{
public:
  iterator(void) {}

  template<typename C>
  iterator(C && c,
           const std::shared_ptr<Reply> & reply,
           std::size_t index)
    : m_c(std::forward<C>(c))
    , m_index(index)
    , m_reply(reply)
  {}

  virtual
  bool operator==(const iterator & other)
  {
    return m_index == other.m_index;
  }

  virtual
  bool operator!=(const iterator & other)
  {
    return ! (*this == other);
  }

  // const ReturnData & operator*(void) = 0;
  ReturnData operator*(void)
  {
    return *(static_cast<Derived &>(*this));
  }

  // prefix
  virtual
  Derived & operator++(void)
  {
    ++m_index;
    return static_cast<Derived &>(*this);
  }

  // postfix
  virtual
  Derived operator++(int)
  {
    auto copy = static_cast<Derived &>(*this);
    ++(*this);
    return copy;
  }

  // prefix
  virtual
  Derived & operator--(void)
  {
    --m_index;
    return static_cast<Derived &>(*this);
  }

  // postfix
  virtual
  Derived operator--(int)
  {
    auto copy = static_cast<Derived &>(*this);
    --(*this);
    return copy;
  }

  template<typename C>
  static
  Derived
  begin(C && c, const std::shared_ptr<Reply> & reply)
  {
    return Derived { std::forward<C>(c), reply, 0 };
  }

  template<typename C>
  static
  Derived
  end(C && c, const std::shared_ptr<Reply> & reply)
  {
    return Derived { std::forward<C>(c), reply, static_cast<std::size_t>(Length(reply.get())) };
  }

protected:
  Connection m_c;
  std::size_t m_index = 0;
  std::shared_ptr<Reply> m_reply;

}; // class iterator

namespace fixed {

// traits to set a data value on an object type
// requires the object to implement xpp::iterable<...>
// can not be in an if-else clause because arithmetic on void is not allowed and
// the compiler will generate an error, even though this code path is never
// taken

template<typename Data>
struct data_traits
{
  template<typename RealData = Data>
  static
  void
  set(RealData & iterable, Data * const data, std::size_t index)
  {
    iterable = static_cast<Data>(data[index]);
  }
};

template<>
struct data_traits<void>
{
  template<typename RealData>
  static
  void
  set(RealData & iterable, void * const data, std::size_t index)
  {
    char * ptr = (char *)data + index * RealData::size_of();
    iterable = (void * const)ptr;
  }
};

namespace iterator {

// handles all iterators for simple types (e.g. xcb_window_t)

template<typename ... Types>
class simple;

template<typename Connection,
         typename ReturnData,
         typename Data,
         typename Reply,
         ACCESSOR_TEMPLATE,
         LENGTH_TEMPLATE>
class simple<Connection,
             ReturnData,
             ACCESSOR_SIGNATURE,
             LENGTH_SIGNATURE>
  : public xpp::iterator<simple<Connection,
                                ReturnData,
                                ACCESSOR_SIGNATURE,
                                LENGTH_SIGNATURE>,
                         Connection,
                         ReturnData,
                         ACCESSOR_SIGNATURE,
                         LENGTH_SIGNATURE>
{
  public:
    typedef simple<Connection,
                   ReturnData,
                   ACCESSOR_SIGNATURE,
                   LENGTH_SIGNATURE>
                     self;

    typedef xpp::iterator<self,
                          Connection,
                          ReturnData,
                          ACCESSOR_SIGNATURE,
                          LENGTH_SIGNATURE>
                            base;

    template<typename C>
    simple(C && c,
           const std::shared_ptr<Reply> & reply,
           std::size_t index)
      : base(std::forward<C>(c), reply, index)
    {
      if (std::is_void<Data>::value) {
        this->m_index /= sizeof(ReturnData);
      }
    }

    virtual
    // const ReturnData &
    ReturnData
    operator*(void)
    {
      return static_cast<ReturnData *>(
          Accessor(this->m_reply.get()))[this->m_index];
    }
};

// handles all iterators for object types (e.g. xpp::window)

template<typename ... Types>
class object;

template<typename Connection,
         typename ReturnData,
         typename Data,
         typename Reply,
         ACCESSOR_TEMPLATE,
         LENGTH_TEMPLATE>
class object<Connection,
             ReturnData,
             ACCESSOR_SIGNATURE,
             LENGTH_SIGNATURE>
  : public xpp::iterator<object<Connection,
                                ReturnData,
                                ACCESSOR_SIGNATURE,
                                LENGTH_SIGNATURE>,
                         Connection,
                         ReturnData,
                         ACCESSOR_SIGNATURE,
                         LENGTH_SIGNATURE>
{
  public:
    typedef object<Connection,
                   ReturnData,
                   ACCESSOR_SIGNATURE,
                   LENGTH_SIGNATURE>
                     self;

    typedef xpp::iterator<self,
                          Connection,
                          ReturnData,
                          ACCESSOR_SIGNATURE,
                          LENGTH_SIGNATURE>
                            base;

    template<typename C>
    object(C & c,
           const std::shared_ptr<Reply> & reply,
           std::size_t index)
      : base(std::forward<C>(c), reply, index)
    {
      if (std::is_void<Data>::value) {
        this->m_index /= ReturnData::size_of();
      }
    }

    virtual
    ReturnData
    operator*(void)
    {
      ReturnData rd(base::m_c);
      data_traits<Data>::set(rd,
                             Accessor(this->m_reply.get()),
                             this->m_index);
      return rd;
    }
}; // class object

}; }; // fixed::iterator

// dispatcher to decide which implementation is necessary

template<typename Connection,
         typename ReturnData,
         ACCESSOR_TEMPLATE,
         LENGTH_TEMPLATE>
class iterator<Connection, ReturnData, ACCESSOR_SIGNATURE, LENGTH_SIGNATURE>
  : public std::conditional<
        ! std::is_base_of<xpp::iterable<Data>, ReturnData>::value,
          fixed::iterator::simple<Connection, ReturnData, ACCESSOR_SIGNATURE, LENGTH_SIGNATURE>,
          fixed::iterator::object<Connection, ReturnData, ACCESSOR_SIGNATURE, LENGTH_SIGNATURE>
      >::type
{
  public:
    typedef typename std::conditional<
        ! std::is_base_of<xpp::iterable<Data>, ReturnData>::value,
          fixed::iterator::simple<Connection, ReturnData, ACCESSOR_SIGNATURE, LENGTH_SIGNATURE>,
          fixed::iterator::object<Connection, ReturnData, ACCESSOR_SIGNATURE, LENGTH_SIGNATURE>
      >::type
        base;

    using base::base;
}; // class iterator


namespace generic {

template<typename Connection, typename Reply, typename Iterator>
class list {
  private:
    // before public part, to make decltype in begin() & end() work!
    Connection m_c;
    std::shared_ptr<Reply> m_reply;

  public:
    template<typename C>
    list(C && c, const std::shared_ptr<Reply> & reply)
      : m_c(std::forward<C>(c)), m_reply(reply)
    {}

    auto
    begin(void) -> decltype(Iterator::begin(this->m_c, this->m_reply))
    {
      return Iterator::begin(m_c, m_reply);
    }

    auto
    end(void) -> decltype(Iterator::end(this->m_c, this->m_reply))
    {
      return Iterator::end(m_c, m_reply);
    }

}; // class list

template<typename Reply,
         char * (*Accessor)(const Reply *),
         int (*Length)(const Reply *)>
class string {
  public:
    string(const std::shared_ptr<Reply> & reply)
      : m_string(std::string(Accessor(reply.get()), Length(reply.get())))
    {}

    const std::string & operator*(void) const
    {
      return m_string;
    }

    const std::string * operator->(void) const
    {
      return &m_string;
    }

  private:
    std::string m_string;
}; // class string

template<typename Reply,
         char * (*Accessor)(const Reply *),
         int (*Length)(const Reply *)>
std::ostream &
operator<<(std::ostream & os, const string<Reply, Accessor, Length> & string)
{
  return os << *string;
}

}; // namespace generic

}; // namespace xpp

#endif // XPP_GENERIC_REPLY_ITERATOR_HPP
