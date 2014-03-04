from utils import _n, _ext, _n_item, get_namespace

_templates = {}

_templates['cookie_reply_using'] = \
'''\
/*
namespace checked {
using %s =
  xpp::generic::cookie<xcb_%s_cookie_t,
    xpp::generic::checked::cookie<SIGNATURE(xcb_%s)>>;
}; // namespace checked

namespace unchecked {
using %s =
  xpp::generic::cookie<xcb_%s_cookie_t,
    xpp::generic::unchecked::cookie<SIGNATURE(xcb_%s_unchecked)>>;
}; // namespace unchecked
*/
'''

def _cookie_reply_using(name):
    return _templates['cookie_reply_using'] % \
        ( name
        , name
        , name
        , name
        , name
        , name)

_templates['cookie_void_using'] = \
'''\
/*
namespace checked {
using %s =
  xpp::generic::cookie<xcb_void_cookie_t,
    xpp::generic::checked::cookie<SIGNATURE(xcb_%s_checked)>>;
}; // namespace checked

namespace unchecked {
using %s =
  xpp::generic::cookie<xcb_void_cookie_t,
    xpp::generic::unchecked::cookie<SIGNATURE(xcb_%s)>>;
}; // namespace checked
*/
'''

def _cookie_void_using(name):
    return _templates['cookie_void_using'] % \
        ( name
        , name
        , name
        , name
        )

_templates['reply_cookie_class_derived'] = \
'''\
template<typename CookieMethod>
class %s
  : public xpp::generic::cookie<typename CookieMethod::xcb_cookie_t,
                                %s<CookieMethod>>
{
  public:
    typedef xpp::generic::cookie<typename CookieMethod::xcb_cookie_t,
                                 %s<CookieMethod>>
                                   base;
    using base::base;
%s\
};
'''

def _reply_cookie_class_derived(name, ctors):
    return _templates['reply_cookie_class_derived'] % \
            ( name
            , name
            , name
            , ctors
            )

             # xpp::generic::checked::cookie<SIGNATURE(xcb_%s)>>
_templates['void_cookie_class_derived'] = \
'''\
template<typename CookieMethod>
class %s
  : public xpp::generic::cookie<typename CookieMethod::xcb_cookie_t,
                                %s<CookieMethod>>
{
  public:
    typedef xpp::generic::cookie<typename CookieMethod::xcb_cookie_t,
                                 %s<CookieMethod>>
                                   base;
    using base::base;
%s\
};
'''

def _void_cookie_class_derived(name, ctors):
    return _templates['void_cookie_class_derived'] % \
            ( name
            , name
            , name
            , ctors
            )

_templates['cookie_checked_unchecked_derived'] = \
'''\
namespace checked { namespace cookie {
using %s = generic::cookie::%s<
  xpp::generic::checked::cookie<SIGNATURE(%s)>>;
}; }; // namespace checked::cookie

namespace unchecked { namespace cookie {
using %s = generic::cookie::%s<
  xpp::generic::unchecked::cookie<SIGNATURE(%s)>>;
}; }; // namespace unchecked::cookie
'''

def _cookie_checked_unchecked_derived(name, is_void):
    if is_void:
        checked_c_name = "xcb_" + name + "_checked"
        unchecked_c_name = "xcb_" + name
    else:
        checked_c_name = "xcb_" + name
        unchecked_c_name = "xcb_" + name + "_unchecked"

    return _templates['cookie_checked_unchecked_derived'] % \
            ( name
            , name
            , checked_c_name
            , name
            , name
            , unchecked_c_name
            )


_templates['cookie_checked_unchecked'] = \
'''\
namespace checked { namespace cookie {
using %s = xpp::generic::cookie<%s,
  xpp::generic::checked::cookie<SIGNATURE(%s)>>;
}; }; // namespace checked::cookie

namespace unchecked { namespace cookie {
using %s = xpp::generic::cookie<%s,
  xpp::generic::unchecked::cookie<SIGNATURE(%s)>>;
}; }; // namespace unchecked::cookie
'''

def _cookie_checked_unchecked(name, is_void):
    if is_void:
        cookie = "xcb_void_cookie_t"
        checked_c_name = "xcb_" + name + "_checked"
        unchecked_c_name = "xcb_" + name
    else:
        cookie = "xcb_%s_cookie_t" % name
        checked_c_name = "xcb_" + name
        unchecked_c_name = "xcb_" + name + "_unchecked"

    return _templates['cookie_checked_unchecked'] % \
            ( name
            , cookie
            , checked_c_name
            , name
            , cookie
            , unchecked_c_name
            )

_templates['cookie_class_static_ctor'] = \
'''
%s\
    static
    %s
    get(xcb_connection_t * const c%s)
    {%s\
      return CookieMethod::get(c%s);
    }
'''

def _cookie_class_static_ctor(template, return_value, protos, calls, initializer):
    return _templates['cookie_class_static_ctor'] % \
            ( template
            , return_value
            , protos
            , initializer
            , calls
            )

# %s_checked(xcb_connection_t * const c%s)
_templates['void_cookie_function'] = \
'''\
%s\
void
%s_checked(Connection && c%s)
{%s\
  xpp::generic::check(std::forward<Connection>(c),
                      %s_checked(std::forward<Connection>(c)%s));
}

%s\
void
%s(Connection && c%s)
{%s\
  %s(std::forward<Connection>(c)%s);
}
'''


def _void_cookie_function(name, c_name, template, return_value, protos, calls, initializer):
    if len(template) == 0: template = "template<typename Connection>\n"
    return _templates['void_cookie_function'] % \
            ( template
            # , return_value
            , name
            , protos
            , initializer
            , c_name
            , calls
            # checked
            , template
            # , return_value
            , name
            , protos
            , initializer
            , c_name
            , calls
            )


_templates['cookie_static_getter'] = \
'''\
%s\
    static
    %s
    cookie(xcb_connection_t * const c%s)
    {%s\
      return base::cookie(c%s);
    }
'''

def _cookie_static_getter(template, return_value, protos, calls, initializer):
    return _templates['cookie_static_getter'] % \
            ( template
            , return_value
            , protos
            , initializer
            , calls
            )

class CppCookie(object):
    def __init__(self, namespace, is_void, name, reply, parameter_list):
        self.namespace = namespace
        self.is_void = is_void
        self.name = name
        self.reply = reply
        self.parameter_list = parameter_list
        self.request_name = _ext(_n_item(self.name[-1]))
        self.c_name = "xcb" \
            + (("_" + get_namespace(namespace)) if namespace.is_ext else "") \
            + "_" + self.request_name

    def comma(self):
        return self.parameter_list.comma()

    def calls(self, sort):
        return self.parameter_list.calls(sort)

    def protos(self, sort, defaults):
        return self.parameter_list.protos(sort, defaults)

    def template(self, indent="    ", tail="\n"):
        return indent + "template<typename " \
                + ", typename ".join(self.parameter_list.templates) \
                + ">" + tail \
                if len(self.parameter_list.templates) > 0 \
                else ""

    def iterator_template(self, indent="    ", tail="\n"):
        prefix = "template<typename " + ("Connection, typename " if self.is_void else "")
        return indent + prefix \
                + ", typename ".join(self.parameter_list.iterator_templates \
                                   + self.parameter_list.templates) \
                + ">" + tail \
                if len(self.parameter_list.iterator_templates) > 0 \
                else ""


    def iterator_calls(self, sort):
        return self.parameter_list.iterator_calls(sort)

    def iterator_protos(self, sort, defaults):
        return self.parameter_list.iterator_protos(sort, defaults)

    def iterator_initializers(self):
        return self.parameter_list.iterator_initializers()

    def methods(self, protos, calls, template="", initializer=[]):
        inits = "" if len(initializer) > 0 else "\n"
        for i in initializer:
            inits += "\n"
            for line in i.split('\n'):
                inits += "      " + line + "\n"

        if self.is_void: return_value = "xcb_void_cookie_t"
        else: return_value = "%s_cookie_t" % self.c_name

        return _cookie_class_static_ctor(template,
                                         return_value,
                                         self.comma() + protos,
                                         self.comma() + calls,
                                         inits)

    def void_functions(self, protos, calls, template="", initializer=[]):
        inits = "" if len(initializer) > 0 else "\n"
        for i in initializer:
            inits += "\n"
            for line in i.split('\n'):
                inits += "      " + line + "\n"

        return_value = "xcb_void_cookie_t"

        return _void_cookie_function(self.request_name,
                                     self.c_name,
                                     template,
                                     return_value,
                                     self.comma() + protos,
                                     self.comma() + calls,
                                     inits)


    def static_reply_methods(self, protos, calls, template="", initializer=[]):
        inits = "" if len(initializer) > 0 else "\n"
        for i in initializer:
            inits += "\n"
            for line in i.split('\n'):
                inits += "      " + line + "\n"

        if self.is_void: return_value = "xcb_void_cookie_t"
        else: return_value = self.c_name + "_cookie_t"

        return _cookie_static_getter(template,
                                     return_value,
                                     self.comma() + protos,
                                     self.comma() + calls,
                                     inits)


    def make_static_getter(self):
        default = self.static_reply_methods(self.protos(False, False), self.calls(False))

        if self.parameter_list.has_defaults:
            default = self.static_reply_methods(self.protos(True, True), self.calls(False))

        wrapped = ""
        if self.parameter_list.want_wrap:
            wrapped = "\n" + \
                self.static_reply_methods(self.iterator_protos(True, True),
                        self.iterator_calls(False), self.iterator_template(),
                        self.iterator_initializers())

        default_args = ""
        if self.parameter_list.is_reordered():
            default_args = "\n" + \
                self.static_reply_methods(self.protos(True, True), self.calls(False))


        result = ""
        if len(default_args + wrapped) > 0:
            result += "\n" + default + default_args + wrapped

        return result

    def make_void_functions(self):
        default = self.void_functions(self.protos(False, False), self.calls(False))

        if self.parameter_list.has_defaults:
            default = self.void_functions(self.protos(True, True), self.calls(False))

        wrapped = ""
        if self.parameter_list.want_wrap:
            wrapped = "\n" + \
                self.void_functions(self.iterator_protos(True, True),
                        self.iterator_calls(False), self.iterator_template(),
                        self.iterator_initializers())

        default_args = ""
        if self.parameter_list.is_reordered():
            default_args = "\n" + \
                self.void_functions(self.protos(True, True), self.calls(False))

        result = ""
        if len(default_args + wrapped) > 0:
            result += "\n" + default + default_args + wrapped

        return result

    def make(self):

        default = self.methods(self.protos(False, False), self.calls(False))

        if self.parameter_list.has_defaults:
            default = self.methods(self.protos(True, True), self.calls(False))

        wrapped = ""
        if self.parameter_list.want_wrap:
            wrapped = "\n" + \
                self.methods(self.iterator_protos(True, True),
                        self.iterator_calls(False), self.iterator_template(),
                        self.iterator_initializers())

        default_args = ""
        if self.parameter_list.is_reordered():
            default_args = "\n" + \
                self.methods(self.protos(True, True), self.calls(False))


        result = ""
        if len(default_args + wrapped) > 0:
            result += "namespace generic {\n"
            result += "namespace cookie {\n"


            if self.is_void:
                result += _void_cookie_class_derived(self.request_name, default + default_args + wrapped)
            else:
                result += _reply_cookie_class_derived(self.request_name, default + default_args + wrapped)

            result += "}; // cookie\n"
            result += "}; // generic\n"

            result += _cookie_checked_unchecked_derived(self.request_name, self.is_void)

        # elif self.is_void:
        #     result += _cookie_void_using(self.request_name)

        # else:
        #     result += _cookie_reply_using(self.request_name)

        else:
            result += _cookie_checked_unchecked(self.request_name, self.is_void)

        return result
