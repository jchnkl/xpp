# vim: set ts=4 sws=4 sw=4:

# from utils import *
from utils import _n, _ext, _n_item, get_namespace
from parameter import *
from resource_classes import _resource_classes
from cppreply import CppReply
from cppcookie import CppCookie

_templates = {}

_templates['void_request_function'] = \
'''\
template<typename Connection, typename ... Parameter>
void
%s_checked(Connection && c, Parameter && ... parameter)
{
  xpp::generic::check(std::forward<Connection>(c),
                      xcb_%s_checked(std::forward<Connection>(c),
                                     std::forward<Parameter>(parameter) ...));
}

template<typename ... Parameter>
void
%s(Parameter && ... parameter)
{
  xcb_%s(std::forward<Parameter>(parameter) ...);
}
'''

def _void_request_function(name):
    return _templates['void_request_function'] % \
            ( name
            , name
            , name
            , name
            )

_templates['reply_request_function'] = \
'''\
template<typename Connection, typename ... Parameter>
reply::%s<Connection, xpp::generic::checked_tag>
%s(Connection && c, Parameter && ... parameter)
{
  return reply::%s<Connection, xpp::generic::checked_tag>(
      std::forward<Connection>(c), std::forward<Parameter>(parameter) ...);
}

template<typename Connection, typename ... Parameter>
reply::%s<Connection, xpp::generic::unchecked_tag>
%s_unchecked(Connection && c, Parameter && ... parameter)
{
  return reply::%s<Connection, xpp::generic::unchecked_tag>(
      std::forward<Connection>(c), std::forward<Parameter>(parameter) ...);
}
'''

def _reply_request_function(name):
    return _templates['reply_request_function'] % \
            ( name
            , name
            , name
            , name
            , name
            , name)

_templates['inline_reply_class'] = \
'''\
    template<typename ... Parameter>
    reply::%s<Connection, xpp::generic::checked_tag>
    %s(Parameter && ... parameter)
    {
      return xpp::%s::%s(
          static_cast<connection &>(*this).get(),
          %s\
          std::forward<Parameter>(parameter) ...);
    }

    template<typename ... Parameter>
    reply::%s<Connection, xpp::generic::unchecked_tag>
    %s_unchecked(Parameter && ... parameter)
    {
      return xpp::%s::%s_unchecked(
          static_cast<connection &>(*this).get(),
          %s\
          std::forward<Parameter>(parameter) ...);
    }
'''

def _inline_reply_class(request_name, method_name, member, ns):
    return _templates['inline_reply_class'] % \
            ( request_name
            , method_name
            , ns
            , request_name
            , member
            , request_name
            , method_name
            , ns
            , request_name
            , member
            )

_templates['inline_void_class'] = \
'''\
    template<typename ... Parameter>
    void
    %s_checked(Parameter && ... parameter)
    {
      xpp::%s::%s_checked(static_cast<connection &>(*this).get(),
                          %s\
                          std::forward<Parameter>(parameter) ...);
    }

    template<typename ... Parameter>
    void
    %s(Parameter && ... parameter)
    {
      xpp::%s::%s(static_cast<connection &>(*this).get(),
                  %s\
                  std::forward<Parameter>(parameter) ...);
    }
'''

def _inline_void_class(request_name, method_name, member, ns):
    return _templates['inline_void_class'] % \
            ( method_name
            , ns
            , request_name
            , member
            , method_name
            , ns
            , request_name
            , member
            )

_templates['void_constructor'] = \
"""\
    %s(xcb_connection_t * c%s)
    {%s
      request::operator()(c%s);
    }
"""

_templates['void_operator'] = \
"""\
    void
    operator()(xcb_connection_t * c%s) const
    {%s
      request::operator()(c%s);
    }
"""

_templates['void_request_head'] = \
"""\
namespace %s {%s namespace request {

class %s
  : public xpp::generic::%s::request<
        %s\
        FUNCTION_SIGNATURE(%s)>
{
  public:
    %s(void)
    {}
"""

_templates['void_request_tail'] = \
"""\
}; // class %s

}; };%s // request::%s%s
"""

_templates['reply_request'] = \
"""\
    %s(xcb_connection_t * c%s)
      // : request(c), m_c(c)
      : m_c(c)
    {%s
      request::prepare(c%s);
    }
"""

_templates['reply_request_head'] = \
"""\
namespace %s {%s namespace request {

class %s
  : public xpp::generic::%s::request<
        %s\
        FUNCTION_SIGNATURE(%s_reply),
        FUNCTION_SIGNATURE(%s)>
{
  public:
"""

_templates['reply_request_tail'] = \
"""\
%s\

  protected:
    operator xcb_connection_t * const(void) const
    {
      return m_c;
    }

  private:
    xcb_connection_t * m_c;
}; // class %s
%s\
}; };%s // request::%s%s
"""

_field_accessor_template = \
'''\
      template<typename %s = %s>
      %s
      %s(void) const
      {
        return %s(*this, %s);
      }\
'''

_field_accessor_template_specialization = \
'''\
template<>
%s
%s::%s<%s>(void) const
{
  return %s;
}\
'''

_replace_special_classes = \
        { "gcontext" : "gc" }

def replace_class(method, class_name):
    cn = _replace_special_classes.get(class_name, class_name)
    return method.replace("_" + cn, "")

class CppRequest(object):
    def __init__(self, request, name, is_void, namespace, reply):
        self.request = request
        self.name = name
        self.request_name = _ext(_n_item(self.request.name[-1]))
        self.is_void = is_void
        self.namespace = namespace
        self.reply = reply
        self.c_namespace = \
            "" if namespace.header.lower() == "xproto" \
            else get_namespace(namespace)
        self.accessors = []
        self.parameter_list = ParameterList()

    def make_wrapped(self):
        self.parameter_list.make_wrapped()

    def make_proto(self):
        return "  class " + self.name + ";"

    def make_class(self):
        cppcookie = CppCookie(self.namespace, self.is_void, self.request.name, self.reply, self.parameter_list)

        if self.is_void:
            void_functions = cppcookie.make_void_functions()
            if len(void_functions) > 0:
                return void_functions
            else:
                return _void_request_function(self.request_name)

        else:
            cppreply = CppReply(self.namespace, self.request.name, cppcookie, self.reply, self.accessors, self.parameter_list)
            return cppreply.make() + "\n\n" + _reply_request_function(self.request_name)

    def make_object_class_inline(self, is_connection, class_name=""):
        member = ""
        method_name = self.name
        if not is_connection:
            member = "*this,\n"
            method_name = replace_class(method_name, class_name)

        if self.is_void:
            return _inline_void_class(self.request_name, method_name, member, get_namespace(self.namespace))
        else:
            return _inline_reply_class(self.request_name, method_name, member, get_namespace(self.namespace))

    def add(self, param):
        self.parameter_list.add(param)

    def comma(self):
        return self.parameter_list.comma()

    def c_name(self, regular=True):
        ns = "" if self.c_namespace == "" else (self.c_namespace + "_")
        name = "xcb_" + ns + self.name

        # checked = void and not regular
        # unchecked = not void and not regular
        if not regular:
            if self.is_void:
                return name + "_checked"
            else:
                return name + "_unchecked"
        else:
            return name

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
        return indent + "template<typename " \
                + ", typename ".join(self.parameter_list.iterator_templates \
                                   + self.parameter_list.templates) \
                + ">" + tail \
                if len(self.parameter_list.iterator_templates) > 0 \
                else ""

    def wrapped_calls(self, sort):
        return self.parameter_list.wrapped_calls(sort)

    def wrapped_protos(self, sort, defaults):
        return self.parameter_list.wrapped_protos(sort, defaults)

    def iterator_calls(self, sort):
        return self.parameter_list.iterator_calls(sort)

    def iterator_2nd_lvl_calls(self, sort):
        return self.parameter_list.iterator_2nd_lvl_calls(sort)

    def iterator_protos(self, sort, defaults):
        return self.parameter_list.iterator_protos(sort, defaults)

    def iterator_initializers(self):
        return self.parameter_list.iterator_initializers()

    def make_accessors(self):
        return "\n".join(map(lambda a: "\n%s\n" % a, self.accessors))


    ########## VOID REQUEST  ##########

    def void_request(self, regular):
        ############ def methods(...) ############
        def methods(protos, calls, template="", initializer=[]):
            # if len(initializer) > 0:
            initializer = "\n      ".join([""] + initializer)

            ctor = _templates['void_constructor'] \
                % (self.name, self.comma() + protos,
                   initializer,
                   self.comma() + calls)

            operator = _templates['void_operator'] \
                % (self.comma() + protos,
                   initializer,
                   self.comma() + calls)

            return template + ctor + "\n" + template + operator

            # else:
            #     return ""

        ############ def methods(...) ############

        namespace = get_namespace(self.namespace)
        # checked = void and not regular
        checked_open = ""
        checked_close = ""
        checked_comment = ""
        extension = ""
        if not regular:
            checked_open = " namespace checked {"
            checked_close = " };"
            checked_comment = "checked::"
            extension = (namespace if self.namespace.is_ext else "void") + ",\n"

        else:
            checked_open = " namespace unchecked {"
            checked_close = " };"
            checked_comment = "unchecked::"

        head = _templates['void_request_head'] \
            % (namespace, checked_open,
               self.name,
               "checked" if not regular else "unchecked",
               extension,
               self.c_name(regular),
               self.name)

        tail = _templates['void_request_tail'] \
            % (self.name,
               checked_close,
               checked_comment,
               namespace)

        default = methods(self.protos(False, False), self.calls(False))

        if (self.parameter_list.has_defaults):
            default = methods(self.protos(True, True), self.calls(False))

        # if len(default) == 0:
        #     default = "    using request::request;\n"

        wrapped = ""
        if self.parameter_list.want_wrap:
            wrapped = "\n" + \
                methods(self.iterator_protos(True, True),
                        self.iterator_calls(False), self.iterator_template(),
                        self.iterator_initializers())

        default_args = ""
        if self.parameter_list.is_reordered():
            default_args = "\n" + \
                methods(self.protos(True, True), self.calls(False))

        if len(self.accessors) > 0:
            sys.stderr.write("FOOBAR!!!\n")

        return head \
             + default \
             + default_args \
             + wrapped \
             + tail
             # + self.make_accessors()

    ########## VOID REQUEST  ##########



    ########## REPLY_REQUEST ##########

    def reply_request(self, regular):
        ############ def methods(...) ############
        def methods(protos, calls, template="", initializer=[]):
            initializer = "\n      ".join([""] + initializer)

            return template + _templates['reply_request'] \
                    % (self.name,
                       self.comma() + protos,
                       initializer,
                       # self.c_name(regular), self.comma() + calls)
                       self.comma() + calls)

        ############ def methods(...) ############

        member_accessors = []
        member_accessors_special = []
        for field in self.reply.fields:
            if (field.field_type[-1] in _resource_classes
                and not field.type.is_list
                and not field.type.is_container):

                template_name = field.field_name.capitalize()
                c_name = field.c_field_type
                method_name = field.field_name.lower()
                if method_name == self.name: method_name += "_"
                member = "m_reply->" + field.c_field_name

                member_accessors.append(_field_accessor_template % \
                    ( template_name, c_name # template<typename %s = %s>
                    , template_name # return type
                    , method_name
                    , template_name, member # return %s(m_c, %s);
                    ))

                member_accessors_special.append(_field_accessor_template_specialization % \
                    ( c_name
                    , self.name, method_name, c_name
                    , member
                    ))

        if len(member_accessors) > 0:
            member_accessors = "\n" + "\n\n".join(member_accessors) + "\n\n"
            member_accessors_special = "\n" + "\n\n".join(member_accessors_special) + "\n\n"
        else:
            member_accessors = ""
            member_accessors_special = ""

        namespace = get_namespace(self.namespace)

        # unchecked = not void and not regular
        unchecked_open = ""
        unchecked_close = ""
        unchecked_comment = ""
        extension = ""
        if regular:
            unchecked_open = " namespace checked {"
            unchecked_close = " };"
            unchecked_comment = "checked::"
            extension = (namespace if self.namespace.is_ext else "void") + ",\n"

        else:
            unchecked_open = " namespace unchecked {"
            unchecked_close = " };"
            unchecked_comment = "unchecked::"

        # extension = ""
        # if regular:
        #     extension = if self.namespace.is_ext else "void",

        head = _templates['reply_request_head'] \
                % (namespace, unchecked_open,
                   self.name,
                   "checked" if regular else "unchecked",
                   extension,
                   self.c_name(),
                   self.c_name(regular))

        tail = _templates['reply_request_tail'] \
                % (member_accessors,
                   self.name,
                   member_accessors_special,
                   unchecked_close,
                   unchecked_comment,
                   namespace)

        default = methods(self.protos(False, False), self.calls(False))

        if self.parameter_list.has_defaults:
            default = methods(self.protos(True, True), self.calls(False))

        wrapped = ""
        if self.parameter_list.want_wrap:
            wrapped = "\n" + \
                methods(self.iterator_protos(True, True),
                        self.iterator_calls(False), self.iterator_template(),
                        self.iterator_initializers())

        default_args = ""
        if self.parameter_list.is_reordered():
            default_args = "\n" + \
                methods(self.protos(True, True), self.calls(False))

        # sys.stderr.write("REPLY:\n%s\n\n" % head \
        #      + default \
        #      + default_args \
        #      + wrapped \
        #      + self.make_accessors() \
        #      + tail)

        return head \
             + default \
             + default_args \
             + wrapped \
             + self.make_accessors() \
             + tail

    ########## REPLY_REQUEST ##########

# template<typename Connection = xcb_connection_t * const,
#          typename ... ErrorHandlers>
# class get_window_attributes_reply
#   : public xpp::generic::reply<Connection,
#                                xcb_get_window_attributes_cookie_t,
#                                SIGNATURE(xcb_get_window_attributes_reply),
#                                ErrorHandlers ...>
# {
#   public:
#     typedef xpp::generic::reply<Connection,
#                                 xcb_get_window_attributes_cookie_t,
#                                 SIGNATURE(xcb_get_window_attributes_reply),
#                                 ErrorHandlers ...>
#                                   base;
#     using base::base;
# 
#     xcb_colormap_t
#     colormap(void) const
#     {
#       return this->m_reply->colormap;
#     }
# };

########## REQUESTS  ##########
