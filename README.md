# xpp - A C++11 RAII wrapper for XCB

## Synopsis

XPP is a C++11
[RAII](https://en.wikipedia.org/wiki/Resource_Acquisition_Is_Initialization)
wrapper around [X protocol C-language Binding
(XCB)](http://xcb.freedesktop.org). Pointers to dynamically allocated memory,
such as events and errors are wrapped in std::shared_ptr.

Furthermore, interfaces for connection and resource types are provided to
facilitate the creation of custom classes. For convenience, a connection class
and several basic resource type classes are readily available.

XPP makes widespread use of the
[Curiously Recurring Template Pattern (CRTP)](https://en.wikibooks.org/wiki/More_C++_Idioms/Curiously_Recurring_Template_Pattern)
to avoid overhead through dynamic dispatch. Hence, most interfaces are
implicitly defined.

### Basic structure

`xpp::request::{x, randr, screensaver, ..}`:
---

Requests like those in the usual `xcb_{namespace_}{request_name}` format.

Example:

`xpp::request::x::query_tree(xcb_connection_t, xcb_window_t)`
`xpp::request::x::intern_atom(xcb_connection_t, bool, std::string)`
`xpp::request::randr::get_screen_info(xcb_connection_t, xcb_window_t)`


`xpp::protocol::{x, randr, screensaver, ..}`:
---

Facade like classes to bundle `xcb_connection_t` together with request methods.

Example:

`xpp::protocol::x::query_tree(xcb_window_t)`
`xpp::protocol::x::intern_atom(bool, std::string)`
`xpp::protocol::randr::get_screen_info(xcb_window_t)`


`xpp::x::drawable`, `xpp::x::window`, `xpp::x::pixmap`, etc.:
---

Facade like classes to bundle `xcb_connection_t` *and* `xcb_window_t`
together with request methods. Mainly for encapsulating X resources (like
windows)

Example:

`xpp::x::window::query_tree(xcb_window_t)`
`xpp::x::window::intern_atom(bool, std::string)`
