#ifndef X_REQUEST_ITERATOR_HPP
#define X_REQUEST_ITERATOR_HPP

#include <cstdlib> // size_t
#include <memory>
#include <stack>
#include <xcb/xcb.h> // xcb_str_*

namespace xpp {

namespace request {

namespace container {

template<typename Type,
         typename Reply,
         typename Iterator,
         void (*Next)(Iterator *),
         int (*SizeOf)(const void *),
         Iterator (*GetIterator)(const Reply *)>
class variable {
  public:
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

        const Type & operator*(void)
        {
          return *m_iterator.data;
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
            Type * data = m_iterator.data;
            Type * prev = data - m_lengths.top();
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
    };

    variable(const std::shared_ptr<Reply> & reply)
      : m_reply(reply)
    {}

    iterator begin(void)
    {
      return iterator::begin(m_reply);
    }

    iterator end(void)
    {
      return iterator::end(m_reply);
    }

  private:
    std::shared_ptr<Reply> m_reply;
}; // class variable

template<typename Data,
         typename Reply,
         Data * (*Accessor)(const Reply *),
         int (*Length)(const Reply *)>
class fixed {
  public:
    class iterator {
      public:
        iterator(void) {}

        bool operator==(const iterator & other)
        {
          return m_index == other.m_index;
        }

        bool operator!=(const iterator & other)
        {
          return ! (*this == other);
        }

        const Data & operator*(void)
        {
          return Accessor(m_reply.get())[m_index];
        }

        // prefix
        iterator & operator++(void)
        {
          ++m_index;
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
          --m_index;
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
          return iterator(reply, 0);
        }

        static
        iterator
        end(const std::shared_ptr<Reply> & reply)
        {
          return iterator(reply, Length(reply.get()));
        }

      private:
        std::size_t m_index = 0;
        std::shared_ptr<Reply> m_reply;

        iterator(const std::shared_ptr<Reply> & reply, std::size_t index)
          : m_index(index), m_reply(reply)
        {}
    }; // class iterator

    fixed(const std::shared_ptr<Reply> & reply)
      : m_reply(reply)
    {}

    iterator begin(void)
    {
      return iterator::begin(m_reply);
    }

    iterator end(void)
    {
      return iterator::end(m_reply);
    }

  private:
    std::shared_ptr<Reply> m_reply;
}; // class fixed

}; // namespace container

}; // namespace request

}; // namespace xpp

#endif // X_REQUEST_ITERATOR_HPP
