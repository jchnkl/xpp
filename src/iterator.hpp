#ifndef X_REQUEST_ITERATOR_HPP
#define X_REQUEST_ITERATOR_HPP

#include <cstdlib> // size_t
#include <memory>
#include <stack>
#include <xcb/xcb.h> // xcb_str_*

namespace xpp {

namespace generic {

namespace variable_size {

template<typename Data,
         typename Return,
         typename Reply,
         typename Iterator,
         void (*Next)(Iterator *),
         int (*SizeOf)(const void *),
         Iterator (*GetIterator)(const Reply *)>
class iterator {
  public:
    iterator(void) {}

    bool operator==(const iterator & other)
    {
      return m_iterator.rem == other.m_iterator.rem;
    }

    bool operator!=(const iterator & other)
    {
      return ! (*this == other);
    }

    const Return & operator*(void)
    {
      return *(static_cast<Return *>(m_iterator.data));
    }

    // prefix
    iterator & operator++(void)
    {
      m_lengths.push(SizeOf(m_iterator.data));
      Next(&m_iterator);
      return *this;
    }

    // postfix
    iterator operator++(int)
    {
      auto copy = *this;
      ++(*this);
      return copy;
    }

    // prefix
    iterator & operator--(void)
    {
      if (m_lengths.empty()) {
        Data * data = m_iterator.data;
        Data * prev = data - m_lengths.top();
        m_lengths.pop();

        m_iterator.index = (char *)m_iterator.data - (char *)prev;
        m_iterator.data = prev;
        ++m_iterator.rem;
      }

      return *this;
    }

    // postfix
    iterator operator--(int)
    {
      auto copy = *this;
      --(*this);
      return copy;
    }

    static
    iterator
    begin(const std::shared_ptr<Reply> & reply)
    {
      return iterator(reply);
    }

    static
    iterator
    end(const std::shared_ptr<Reply> & reply)
    {
      auto it = iterator(reply);
      it.m_iterator.rem = 0;
      return it;
    }

  private:
    std::shared_ptr<Reply> m_reply;
    std::stack<std::size_t> m_lengths;
    Iterator m_iterator;

    iterator(const std::shared_ptr<Reply> & reply)
      : m_reply(reply), m_iterator(GetIterator(reply.get()))
    {}
}; // class iterator

template<typename Reply, xcb_str_iterator_t (*GetIterator)(const Reply *)>
class iterator<xcb_str_t,
               xcb_str_t,
               Reply,
               xcb_str_iterator_t,
               &xcb_str_next,
               &xcb_str_sizeof,
               GetIterator>
{
  public:
    iterator(void) {}

    bool operator==(const iterator & other)
    {
      return m_iterator.rem == other.m_iterator.rem;
    }

    bool operator!=(const iterator & other)
    {
      return ! (*this == other);
    }

    const std::string & operator*(void)
    {
      if (m_string.empty()) {
        m_string = std::string((char *)xcb_str_name(m_iterator.data),
                               xcb_str_name_length(m_iterator.data));
      }
      return m_string;
    }

    const std::string * operator->(void)
    {
      if (m_string.empty()) {
        m_string = std::string((char *)xcb_str_name(m_iterator.data),
                               xcb_str_name_length(m_iterator.data));
      }
      return &m_string;
    }

    // prefix
    iterator & operator++(void)
    {
      m_lengths.push(xcb_str_sizeof(m_iterator.data));
      xcb_str_next(&m_iterator);
      m_string.clear();
      return *this;
    }

    // postfix
    iterator operator++(int)
    {
      auto copy = *this;
      ++(*this);
      return copy;
    }

    // prefix
    iterator & operator--(void)
    {
      if (m_lengths.empty()) {
        xcb_str_t * data = m_iterator.data;
        xcb_str_t * prev = data - m_lengths.top();
        m_lengths.pop();

        m_iterator.index = (char *)m_iterator.data - (char *)prev;
        m_iterator.data = prev;
        ++m_iterator.rem;

        m_string.clear();
      }

      return *this;
    }

    // postfix
    iterator operator--(int)
    {
      auto copy = *this;
      --(*this);
      return copy;
    }

    static
    iterator
    begin(const std::shared_ptr<Reply> & reply)
    {
      return iterator(reply);
    }

    static
    iterator
    end(const std::shared_ptr<Reply> & reply)
    {
      auto it = iterator(reply);
      it.m_iterator.rem = 0;
      return it;
    }

  private:
    std::shared_ptr<Reply> m_reply;
    std::stack<std::size_t> m_lengths;
    xcb_str_iterator_t m_iterator;
    std::string m_string;

    iterator(const std::shared_ptr<Reply> & reply)
      : m_reply(reply), m_iterator(GetIterator(reply.get()))
    {}
}; // class iterator

}; // namespace variable_size

namespace fixed_size {

template<typename IteratorType,
         typename Data,
         typename Return,
         typename Reply,
         Data * (*Accessor)(const Reply *),
         int (*Length)(const Reply *)>
class iterator_base {
public:
  virtual
  bool operator==(const iterator_base & other)
  {
    return m_index == other.m_index;
  }

  virtual
  bool operator!=(const iterator_base & other)
  {
    return ! (*this == other);
  }

  virtual
  const Return & operator*(void) = 0;

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
  begin(const std::shared_ptr<Reply> & reply);

  static
  IteratorType
  end(const std::shared_ptr<Reply> & reply);

protected:
  std::size_t m_index = 0;
  std::shared_ptr<Reply> m_reply;

  iterator_base(void) {}

  iterator_base(const std::shared_ptr<Reply> & reply, std::size_t index)
    : m_index(index), m_reply(reply)
  {}
}; // class iterator

template<typename Data,
         typename Return,
         typename Reply,
         Data * (*Accessor)(const Reply *),
         int (*Length)(const Reply *)>
class iterator
  : public iterator_base<iterator<Data, Return, Reply, Accessor, Length>,
                         Data, Return, Reply, Accessor, Length>
{
public:
    using iterator_base<iterator<Data, Return, Reply, Accessor, Length>,
                        Data, Return, Reply, Accessor, Length>::iterator_base;

    virtual
    const Return & operator*(void)
    {
      return static_cast<Return *>(
          Accessor(this->m_reply.get()))[this->m_index];
    }

    static
    iterator
    begin(const std::shared_ptr<Reply> & reply)
    {
      return iterator(reply, 0);
    }

    static
    iterator
    end(const std::shared_ptr<Reply> & reply)
    {
      return iterator(reply, Length(reply.get()));
    }
  }; // class iterator

template<typename Return,
         typename Reply,
         void * (*Accessor)(const Reply *),
         int (*Length)(const Reply *)>
class iterator<void, Return, Reply, Accessor, Length>
  : public iterator_base<iterator<void, Return, Reply, Accessor, Length>,
                         void, Return, Reply, Accessor, Length>
{
public:
    using iterator_base<iterator<void, Return, Reply, Accessor, Length>,
                        void, Return, Reply, Accessor, Length>::iterator_base;

    virtual
    const Return & operator*(void)
    {
      return static_cast<Return *>(
          Accessor(this->m_reply.get()))[this->m_index];
    }

    static
    iterator
    begin(const std::shared_ptr<Reply> & reply)
    {
      return iterator(reply, 0);
    }

    static
    iterator
    end(const std::shared_ptr<Reply> & reply)
    {
      return iterator(reply, Length(reply.get()) / sizeof(Return));
    }
}; // class iterator

}; // namespace fixed_size

template<typename Reply, typename Iterator>
class list {
  public:
    list(const std::shared_ptr<Reply> & reply)
      : m_reply(reply)
    {}

    Iterator begin(void)
    {
      return Iterator::begin(m_reply);
    }

    Iterator end(void)
    {
      return Iterator::end(m_reply);
    }

  private:
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
