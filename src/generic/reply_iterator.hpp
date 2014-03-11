#ifndef XPP_GENERIC_REPLY_ITERATOR_HPP
#define XPP_GENERIC_REPLY_ITERATOR_HPP

#include <cstdlib> // size_t
#include <memory>
#include <stack>
#include <xcb/xcb.h> // xcb_str_*
#include "type.hpp"
#include "factory.hpp"
#include "signature.hpp"
#include "iterator_traits.hpp"


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

// iterator for variable size data fields

template<typename ... Types>
class iterator;

template<typename Connection,
         typename ReturnData,
         typename Reply,
         typename XcbIterator,
         NEXT_TEMPLATE,
         SIZEOF_TEMPLATE,
         GETITERATOR_TEMPLATE>
class iterator<Connection,
               ReturnData,
               NEXT_SIGNATURE,
               SIZEOF_SIGNATURE,
               GETITERATOR_SIGNATURE>
{
  protected:
    using self = iterator<Connection,
                          ReturnData,
                          NEXT_SIGNATURE,
                          SIZEOF_SIGNATURE,
                          GETITERATOR_SIGNATURE>;

    // typename Dummy to allow specialization in class scope
    template<typename Data, typename = void>
    class get
    {
      public:
        Data
        operator()(Data * const data)
        {
          return *data;
        }
    };

    template<typename Dummy>
    class get<xcb_str_t, Dummy>
    {
      public:
        std::string
        operator()(xcb_str_t * const data)
        {
          return std::string(xcb_str_name(data),
                             xcb_str_name_length(data));
        }
    };

    Connection m_c;
    std::shared_ptr<Reply> m_reply;
    std::stack<std::size_t> m_lengths;
    XcbIterator m_iterator;

  public:
    iterator(void) {}

    template<typename C>
    iterator(C && c, const std::shared_ptr<Reply> & reply)
      : m_c(std::forward<C>(c))
      , m_reply(reply)
      , m_iterator(GetIterator(reply.get()))
    {}

    bool
    operator==(const iterator & other)
    {
      return m_iterator.rem == other.m_iterator.rem;
    }

    bool
    operator!=(const iterator & other)
    {
      return ! (*this == other);
    }

    auto
    operator*(void) -> decltype(get<ReturnData>()(this->m_iterator.data))
    {
      return get<ReturnData>()(m_iterator.data);
    }

    // prefix
    self &
    operator++(void)
    {
      m_lengths.push(SizeOf(m_iterator.data));
      Next(&m_iterator);
      return *this;
    }

    // postfix
    self
    operator++(int)
    {
      auto copy = *this;
      ++(*this);
      return copy;
    }

    // prefix
    self &
    operator--(void)
    {
      typedef typename std::remove_pointer<decltype(m_iterator.data)>::type data_t;
      if (m_lengths.empty()) {
        data_t * data = m_iterator.data;
        data_t * prev = data - m_lengths.top();
        m_lengths.pop();
        m_iterator.index = (char *)m_iterator.data - (char *)prev;
        m_iterator.data = prev;
        ++m_iterator.rem;
      }
      return *this;
    }

    // postfix
    self
    operator--(int)
    {
      auto copy = *this;
      --(*this);
      return copy;
    }

    template<typename C>
    static
    self
    begin(C && c, const std::shared_ptr<Reply> & reply)
    {
      return self { std::forward<C>(c), reply };
    }

    template<typename C>
    static
    self
    end(C && c, const std::shared_ptr<Reply> & reply)
    {
      auto it = self { std::forward<C>(c), reply };
      it.m_iterator.rem = 0;
      return it;
    }
}; // class iterator

// iterator for fixed size data fields

template<typename Connection,
         typename ReturnData,
         typename Data,
         typename Reply,
         ACCESSOR_TEMPLATE,
         LENGTH_TEMPLATE>
class iterator<Connection,
               ReturnData,
               ACCESSOR_SIGNATURE,
               LENGTH_SIGNATURE>
{
  protected:
    using data_t = typename xpp::generic::conversion_type<ReturnData>::type;
    using make = xpp::generic::factory::make<Connection, data_t, ReturnData>;

    Connection m_c;
    std::size_t m_index = 0;
    std::shared_ptr<Reply> m_reply;

  public:
    typedef iterator<Connection,
                     ReturnData,
                     ACCESSOR_SIGNATURE,
                     LENGTH_SIGNATURE>
                       self;

    iterator(void) {}

    template<typename C>
    iterator(C && c,
             const std::shared_ptr<Reply> & reply,
             std::size_t index)
      : m_c(c)
      , m_index(index)
      , m_reply(reply)
    {
      if (std::is_void<Data>::value) {
        m_index /= sizeof(data_t);
      }
    }

    bool operator==(const iterator & other)
    {
      return m_index == other.m_index;
    }

    bool operator!=(const iterator & other)
    {
      return ! (*this == other);
    }

    ReturnData operator*(void)
    {
      return make()(m_c, static_cast<data_t *>(Accessor(m_reply.get()))[m_index]);
    }

    // prefix
    self & operator++(void)
    {
      ++m_index;
      return *this;
    }

    // postfix
    self operator++(int)
    {
      auto copy = *this;
      ++(*this);
      return copy;
    }

    // prefix
    self & operator--(void)
    {
      --m_index;
      return *this;
    }

    // postfix
    self operator--(int)
    {
      auto copy = *this;
      --(*this);
      return copy;
    }

    template<typename C>
    static
    self
    begin(C && c, const std::shared_ptr<Reply> & reply)
    {
      return self { std::forward<C>(c), reply, 0 };
    }

    template<typename C>
    static
    self
    end(C && c, const std::shared_ptr<Reply> & reply)
    {
      return self { std::forward<C>(c),
                    reply,
                    static_cast<std::size_t>(Length(reply.get())) };
    }
}; // class iterator

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

}; // namespace generic

}; // namespace xpp

#endif // XPP_GENERIC_REPLY_ITERATOR_HPP
