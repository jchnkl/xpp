#ifndef X_REQUEST_ITERATOR_HPP
#define X_REQUEST_ITERATOR_HPP

#include <cstdlib> // size_t
#include <memory>
#include <stack>
#include <xcb/xcb.h> // xcb_str_*
#include "core/type.hpp"
#include "iterable.hpp"

#define CALLABLE(FUNCTION) xpp::generic::callable<decltype(FUNCTION), FUNCTION>

namespace xpp {

namespace generic {

template<typename Signature, Signature & S>
struct callable;

// http://bytes.com/topic/c/answers/692802-performance-static-member-function-versus-functor#post2754209
// functors are faster than static methods
template<typename Return,
         typename ... Args, Return (&Function)(Args ...)>
struct callable<Return(Args ...), Function> {
  Return operator()(Args ... args)
  {
    return Function(args ...);
  }
}; // struct callable

}; // namespace generic

template<typename ... Arguments>
class iterator;

// abstract iterator for variable size data fields
// IteratorType: derived iterator class for CRTP

template<typename IteratorType,
         typename Connection,
         typename Data,
         typename ReturnData,
         typename Reply,
         typename XCBIterator,
         typename Next,
         typename SizeOf,
         typename GetIterator>
class iterator<IteratorType,
               Connection,
               Data,
               ReturnData,
               Reply,
               XCBIterator,
               Next,
               SizeOf,
               GetIterator>
{
  public:
    iterator(void) {}

    iterator(Connection && c, const std::shared_ptr<Reply> & reply)
      : m_c(std::forward<Connection>(c))
      , m_reply(reply)
      , m_iterator(GetIterator()(reply.get()))
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
    IteratorType &
    operator++(void)
    {
      m_lengths.push(SizeOf()(m_iterator.data));
      Next()(&m_iterator);
      return static_cast<IteratorType &>(*this);
    }

    // postfix
    virtual
    IteratorType
    operator++(int)
    {
      auto copy = static_cast<IteratorType &>(*this);
      ++(*this);
      return copy;
    }

    // prefix
    virtual
    IteratorType &
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

      return static_cast<IteratorType &>(*this);
    }

    // postfix
    virtual
    IteratorType
    operator--(int)
    {
      auto copy = static_cast<IteratorType &>(*this);
      --(*this);
      return copy;
    }

    static
    IteratorType
    begin(xcb_connection_t * const c, const std::shared_ptr<Reply> & reply)
    {
      return IteratorType(c, reply);
    }

    static
    IteratorType
    end(xcb_connection_t * const c, const std::shared_ptr<Reply> & reply)
    {
      auto it = IteratorType(c, reply);
      it.m_iterator.rem = 0;
      return it;
    }

  protected:
    Connection m_c;
    std::shared_ptr<Reply> m_reply;
    std::stack<std::size_t> m_lengths;
    XCBIterator m_iterator;
}; // class iterator

// general iterator for variable sized data fields

template<typename Connection,
         typename Data,
         typename ReturnData,
         typename Reply,
         typename XCBIterator,
         typename Next,
         typename SizeOf,
         typename GetIterator>
class iterator<Connection, Data, ReturnData, Reply, XCBIterator, Next, SizeOf, GetIterator>
  : public iterator<
      // self
      iterator<Connection, Data, ReturnData, Reply, XCBIterator, Next, SizeOf, GetIterator>,
      // other args
      Connection, Data, ReturnData, Reply, XCBIterator, Next, SizeOf, GetIterator>
{
  public:
    typedef iterator<Connection,
                     Data,
                     ReturnData,
                     Reply,
                     XCBIterator,
                     Next,
                     SizeOf,
                     GetIterator> self;

    typedef iterator<self,
                     Connection,
                     Data,
                     ReturnData,
                     Reply,
                     XCBIterator,
                     Next,
                     SizeOf,
                     GetIterator> base;

    using base::base;

    virtual
    ReturnData
    operator*(void)
    {
      return *(static_cast<ReturnData *>(base::m_iterator.data));
    }
}; // class iterator

// specialized iterator for variable sized data fields which are strings

template<typename Reply, typename GetIterator>
class iterator<xcb_str_t, xcb_str_t, Reply, xcb_str_iterator_t,
               CALLABLE(xcb_str_next), CALLABLE(xcb_str_sizeof), GetIterator>
  : public iterator<
      // self
      iterator<xcb_str_t, xcb_str_t, Reply, xcb_str_iterator_t,
               CALLABLE(xcb_str_next), CALLABLE(xcb_str_sizeof), GetIterator>,
      // other args
      xcb_str_t, std::string, Reply, xcb_str_iterator_t,
      CALLABLE(xcb_str_next),
      CALLABLE(xcb_str_sizeof),
      GetIterator>
{
  public:
    typedef iterator<xcb_str_t, xcb_str_t, Reply, xcb_str_iterator_t,
                     CALLABLE(xcb_str_next),
                     CALLABLE(xcb_str_sizeof),
                     GetIterator>
                       self;

    typedef iterator<self,
                     xcb_str_t, std::string, Reply, xcb_str_iterator_t,
                     CALLABLE(xcb_str_next),
                     CALLABLE(xcb_str_sizeof),
                     GetIterator>
                       base;

    using base::base;

    const std::string & operator*(void)
    {
      if (m_string.empty()) {
        m_string = std::string((char *)xcb_str_name(base::m_iterator.data),
                               xcb_str_name_length(base::m_iterator.data));
      }
      return m_string;
    }

    const std::string * operator->(void)
    {
      if (m_string.empty()) {
        m_string = std::string((char *)xcb_str_name(base::m_iterator.data),
                               xcb_str_name_length(base::m_iterator.data));
      }
      return &m_string;
    }

    // prefix
    iterator & operator++(void)
    {
      base::operator++();
      m_string.clear();
      return *this;
    }

    // prefix
    iterator & operator--(void)
    {
      base::operator--();
      if (base::m_lengths.empty()) {
        m_string.clear();
      }

      return *this;
    }

  protected:
    std::string m_string;
}; // class iterator

// abstract iterator for fixed size data fields
// IteratorType: derived iterator class for CRTP

template<typename IteratorType,
         typename Connection,
         typename Data,
         typename ReturnData,
         typename Reply,
         typename Accessor,
         typename Length>
class iterator<IteratorType, Connection, Data, ReturnData, Reply, Accessor, Length>
{
public:
  iterator(void) {}

  iterator(Connection && c,
           const std::shared_ptr<Reply> & reply,
           std::size_t index)
    : m_c(std::forward<Connection>(c))
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

  virtual
  // const ReturnData & operator*(void) = 0;
  ReturnData operator*(void) = 0;

  // prefix
  virtual
  IteratorType & operator++(void)
  {
    ++m_index;
    return static_cast<IteratorType &>(*this);
  }

  // postfix
  virtual
  IteratorType operator++(int)
  {
    auto copy = static_cast<IteratorType &>(*this);
    ++(*this);
    return copy;
  }

  // prefix
  virtual
  IteratorType & operator--(void)
  {
    --m_index;
    return static_cast<IteratorType &>(*this);
  }

  // postfix
  virtual
  IteratorType operator--(int)
  {
    auto copy = static_cast<IteratorType &>(*this);
    --(*this);
    return copy;
  }

  static
  IteratorType
  begin(Connection && c, const std::shared_ptr<Reply> & reply)
  {
    return IteratorType(std::forward<Connection>(c), reply, 0);
  }

  static
  IteratorType
  end(Connection && c, const std::shared_ptr<Reply> & reply)
  {
    return IteratorType(std::forward<Connection>(c), reply, Length()(reply.get()));
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

template<typename Connection,
         typename Data,
         typename ReturnData,
         typename Reply,
         typename Accessor,
         typename Length>
class simple
  : public xpp::iterator<
                    xpp::iterator<Connection, Data, ReturnData, Reply, Accessor, Length>,
                    Connection, Data, ReturnData, Reply, Accessor, Length>
{
  public:
    typedef xpp::iterator<Connection, Data, ReturnData, Reply, Accessor, Length> self;
    typedef xpp::iterator<self, Connection, Data, ReturnData, Reply, Accessor, Length> base;

    simple(Connection && c,
           const std::shared_ptr<Reply> & reply,
           std::size_t index)
      : base(std::forward<Connection>(c), reply, index)
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
          Accessor()(this->m_reply.get()))[this->m_index];
    }
};

// handles all iterators for object types (e.g. xpp::window)

template<typename Connection,
         typename Data,
         typename ReturnData,
         typename Reply,
         typename Accessor,
         typename Length>
class object
  : public xpp::iterator<
                    xpp::iterator<Connection, Data, ReturnData, Reply, Accessor, Length>,
                    Connection, Data, ReturnData, Reply, Accessor, Length>
{
  public:
    typedef xpp::iterator<Connection, Data, ReturnData, Reply, Accessor, Length> self;
    typedef xpp::iterator<self, Connection, Data, ReturnData, Reply, Accessor, Length> base;

    object(Connection && c,
           const std::shared_ptr<Reply> & reply,
           std::size_t index)
      : base(std::forward<Connection>(c), reply, index)
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
                             Accessor()(this->m_reply.get()),
                             this->m_index);
      return rd;
    }
}; // class object

}; }; // fixed::iterator

// dispatcher to decide which implementation is necessary

template<typename Connection,
         typename Data,
         typename ReturnData,
         typename Reply,
         typename Accessor,
         typename Length>
class iterator<Connection, Data, ReturnData, Reply, Accessor, Length>
  : public std::conditional<
        ! std::is_base_of<xpp::iterable<Data>, ReturnData>::value,
          fixed::iterator::simple<Connection, Data, ReturnData, Reply, Accessor, Length>,
          fixed::iterator::object<Connection, Data, ReturnData, Reply, Accessor, Length>
      >::type
{
  public:
    typedef typename std::conditional<
        ! std::is_base_of<xpp::iterable<Data>, ReturnData>::value,
          fixed::iterator::simple<Connection, Data, ReturnData, Reply, Accessor, Length>,
          fixed::iterator::object<Connection, Data, ReturnData, Reply, Accessor, Length>
      >::type
        base;

    using base::base;
}; // class iterator


namespace generic {

template<typename Connection, typename Reply, typename Iterator>
class list {
  public:
    list(Connection && c, const std::shared_ptr<Reply> & reply)
      : m_c(std::forward<Connection>(c)), m_reply(reply)
    {}

    Iterator begin(void)
    {
      return Iterator::begin(m_c, m_reply);
    }

    Iterator end(void)
    {
      return Iterator::end(m_c, m_reply);
    }

  private:
    Connection m_c;
    std::shared_ptr<Reply> m_reply;
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

#endif // X_REQUEST_ITERATOR_HPP
