#ifndef XPP_ERROR_HPP
#define XPP_ERROR_HPP

#include <memory> // shared_ptr
#include <xcb/xcb.h> // xcb_generic_error_t

namespace xpp {

  // namespace error { namespace x { class window ..

namespace error {

template<int OpCode, typename Error>
class generic : public std::runtime_error {
  public:
    generic(xcb_generic_error_t * error)
      : runtime_error(get_error_description(error))
      , m_error(reinterpret_cast<Error *>(error))
    {}

    virtual
    ~generic(void)
    {}

    virtual
    operator Error &(void) const
    {
      return *m_error;
    }

    virtual
    const Error &
    operator*(void) const
    {
      return *m_error;
    }

    virtual
    Error * const
    operator->(void) const
    {
      return m_error.get();
    }

  protected:
    virtual
    std::string
    get_error_description(xcb_generic_error_t * error) const
    {
      return "Error code " + std::to_string(error->error_code);
    }

    static const int opcode = OpCode;

    std::shared_ptr<Error> m_error;
}; // class generic

}; }; // xpp::error

#endif // XPP_ERROR_HPP
