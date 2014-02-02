# vim: set ts=4 sws=4 sw=4:

from utils import *
from parameter import *
from resource_classes import _resource_classes

_templates = {}

_templates['inline_class'] = \
"""\
    %s
    %s(%s) const
    {
      %s
    }\
"""

_templates['void_constructor'] = \
"""\
    %s(xcb_connection_t * c%s)
    {%s
      %s(c%s);
    }
"""

_templates['void_operator'] = \
"""\
    void
    operator()(xcb_connection_t * c%s)
    {%s
      %s(c%s);
    }
"""

_templates['void_request_head'] = \
"""\
namespace request {%s namespace %s {

class %s {
  public:
    %s(void) {}

"""

_templates['void_request_tail'] = \
"""\
}; // class %s

}; };%s // request::%s%s
"""

_templates['reply_request'] = \
"""\
    %s(xcb_connection_t * c%s)
      : request(c), m_c(c)
    {%s
      request::prepare(&%s%s);
    }
"""

_templates['reply_request_head'] = \
"""\
namespace request {%s namespace %s {

class %s
  : public generic::request<%s_cookie_t,
                            %s_reply_t,
                            &%s_reply>
{
  public:
"""

_templates['reply_request_tail'] = \
"""\
%s\
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
        if self.is_void:
            return self.void_request(True) + "\n\n" + self.void_request(False)
        else:
            return self.reply_request(True) + "\n\n" + self.reply_request(False)

    def make_object_class_inline(self, is_connection, class_name=""):
        appendix = ("checked" if self.is_void else "unchecked")

        # "%s": ::{un,}checked
        request_name = "xpp::request::" + "%s" + get_namespace(self.namespace) + "::"
        return_type = self.iterator_template(indent="")
        return_type += "virtual\n" if return_type == "" else ""
        return_type += "    "

        if self.is_void:
            return_type += "void"
        else:
            return_type += request_name + self.name

        method = self.name
        if not is_connection:
            method = replace_class(method, class_name)

        calls = self.iterator_2nd_lvl_calls(False)
        proto_params = self.iterator_protos(True, True)

        call_params = ["*this"]
        if not is_connection: call_params += ["*this"]

        if len(calls) > 0: call_params += [calls]

        call = ("" if self.is_void else "return ") \
             + request_name + self.name \
             + ("()" if self.is_void else "") \
             + "(" + ", ".join(call_params) + ");"

        return \
            (_templates['inline_class'] \
                % (return_type if self.is_void else return_type % (appendix + "::"),
                   method + "_" + appendix, proto_params,
                   call % (appendix + "::"))) \
            + "\n\n" + \
            (_templates['inline_class'] \
                % (return_type if self.is_void else return_type % "",
                   method, proto_params,
                   call % ""))

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
            initializer = "\n      ".join([""] + initializer)

            ctor = _templates['void_constructor'] \
                % (self.name, self.comma() + protos,
                   initializer,
                   self.c_name(regular), self.comma() + calls)

            operator = _templates['void_operator'] \
                % (self.comma() + protos,
                   initializer,
                   self.c_name(regular), self.comma() + calls)

            return template + ctor + "\n" + template + operator
        ############ def methods(...) ############

        namespace = get_namespace(self.namespace)
        # checked = void and not regular
        checked_open = ""
        checked_close = ""
        checked_comment = ""
        if not regular:
            checked_open = " namespace checked {"
            checked_close = " };"
            checked_comment = "checked::"

        head = _templates['void_request_head'] \
            % (checked_open, namespace, self.name, self.name)

        tail = _templates['void_request_tail'] \
            % (self.name, checked_close, checked_comment, namespace)

        default = methods(self.protos(False, False), self.calls(False))

        if (self.parameter_list.has_defaults):
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

        return head \
             + default \
             + default_args \
             + wrapped \
             + self.make_accessors() \
             + tail

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
                       self.c_name(regular), self.comma() + calls)

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
        if not regular:
            unchecked_open = " namespace unchecked {"
            unchecked_close = " };"
            unchecked_comment = "unchecked::"

        head = _templates['reply_request_head'] \
                % (unchecked_open, namespace,
                   self.name,
                   self.c_name(),
                   self.c_name(),
                   self.c_name())

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

        return head \
             + default \
             + default_args \
             + wrapped \
             + self.make_accessors() \
             + tail

    ########## REPLY_REQUEST ##########

########## REQUESTS  ##########
