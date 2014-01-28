from utils import *
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
                + ", typename ".join(self.parameter_list.templates \
                                   + self.parameter_list.iterator_templates) \
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

            return template + \
"""\
    %s(xcb_connection_t * c%s)
      : request(c), m_c(c)
    {%s
      request::prepare(&%s%s);
    }
""" % (self.name,
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

        head = \
"""\
namespace request {%s namespace %s {

class %s
  : public generic::request<%s_cookie_t,
                            %s_reply_t,
                            &%s_reply>
{
  public:
""" % (unchecked_open, namespace,
       self.name,
       self.c_name(),
       self.c_name(),
       self.c_name())

        tail = \
"""\
%s\
  private:
    xcb_connection_t * m_c;
}; // class %s
%s\
}; };%s // request::%s%s
""" % (member_accessors,
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



########## PARAMETER ##########

class ParameterList(object):
    def __init__(self):
        self.want_wrap = False
        self.has_defaults = False
        self.parameter = []
        self.wrap_calls = []
        self.wrap_protos = []
        self.iter_calls = []
        self.iter_2nd_lvl_calls = []
        self.iter_protos = []
        self.templates = []
        self.iterator_templates = []
        self.initializer = []

    def add(self, param):
        self.has_defaults = param.default != None
        self.parameter.append(param)

    def comma(self):
        return "" if len(self.parameter) == 0 else ", "

    def is_reordered(self):
        tmp = sorted(self.parameter, cmp=lambda p1, p2: cmp(p1.default, p2.default))
        return tmp != self.parameter

    def calls(self, sort, params=None):
        ps = self.parameter if params == None else params
        if sort:
            tmp = sorted(ps, cmp=lambda p1, p2: cmp(p1.default, p2.default))
            ps = tmp
        calls = map(lambda p: p.call(), ps)
        return "" if len(calls) == 0 else ", ".join(calls)

    def protos(self, sort, defaults, params=None):
        if defaults: sort = True
        ps = self.parameter if params == None else params
        if sort:
            tmp = sorted(ps, cmp=lambda p1, p2: cmp(p1.default, p2.default))
            ps = tmp
        protos = map(lambda p: p.proto(defaults), ps)
        return "" if len(protos) == 0 else ", ".join(protos)

    def iterator_initializers(self):
        return self.initializer

    def make_wrapped(self):
        self.wrap_calls = []
        self.wrap_protos = []
        self.iter_calls = []
        self.iter_2nd_lvl_calls = []
        self.iter_protos = []
        self.initializer = []
        self.templates = []
        self.iterator_templates = []

        # if a parameter is removed, take reduced parameter size into account
        adjust = 0
        for index, param in enumerate(self.parameter):
            prev = index - adjust - 1

            if (param.is_const and param.is_pointer
                    and prev >= 0
                    and self.parameter[prev].c_name == param.c_name + "_len"):

                adjust = adjust + 1
                self.want_wrap = True
                self.wrap_calls.pop(prev)
                self.wrap_protos.pop(prev)
                self.iter_calls.pop(prev)
                self.iter_2nd_lvl_calls.pop(prev)
                self.iter_protos.pop(prev)

                prev_type = self.parameter[prev].c_type
                if param.c_type == 'char':

                    def append_proto_string(list):
                        list.append(Parameter(None, \
                            c_type='const std::string &',
                            c_name=param.c_name))

                    def append_call_string(list):
                        list.append(Parameter(None, \
                            c_name="static_cast<" + prev_type + ">(" \
                            + param.c_name + '.length())'))

                        list.append(Parameter(None, \
                            c_name=param.c_name + '.c_str()'))

                    append_proto_string(self.wrap_protos)
                    append_proto_string(self.iter_protos)
                    append_call_string(self.wrap_calls)
                    append_call_string(self.iter_calls)
                    append_call_string(self.iter_2nd_lvl_calls)

                else:
                    param_type = param.c_type
                    if param_type == "void":
                        param_type = "Type_" + str(index)
                        self.templates.append(param_type)

                    prev_type = self.parameter[prev].c_type

                    ### std::vector
                    self.wrap_protos.append(Parameter(None, \
                        c_type='const std::vector<' + param_type + '> &',
                        c_name=param.c_name))

                    self.wrap_calls.append(Parameter(None, \
                      c_name="static_cast<" + prev_type + ">(" \
                      + param.c_name + '.size())'))

                    self.wrap_calls.append(Parameter(None, \
                        c_name=param.c_name + '.data()'))

                    ### Iterator
                    iter_type = param.c_name.capitalize() + "_Iterator"
                    iter_begin = param.c_name + "_begin"
                    iter_end = param.c_name + "_end"

                    self.iterator_templates.append(iter_type)

                    self.iter_protos.append(Parameter(None, \
                            c_type=iter_type,
                            c_name=iter_begin))

                    self.iter_protos.append(Parameter(None, \
                            c_type=iter_type,
                            c_name=iter_end))

                    self.iter_calls.append(Parameter(None, \
                            c_name="static_cast<" + prev_type + ">(" \
                            + param.c_name + '.size())'))

                    self.iter_calls.append(Parameter(None, \
                            c_name='const_cast<const ' + param_type + ' *>(' \
                            + param.c_name + '.data())'))

                    self.iter_2nd_lvl_calls.append(Parameter(None, \
                            c_name=iter_begin))

                    self.iter_2nd_lvl_calls.append(Parameter(None, \
                            c_name=iter_end))

                    self.initializer.append( \
                            "std::vector<%s> %s = { %s, %s };" \
                            % (param_type, param.c_name, iter_begin, iter_end))

            else:
                self.wrap_calls.append(param)
                self.wrap_protos.append(param)
                self.iter_calls.append(param)
                self.iter_2nd_lvl_calls.append(param)
                self.iter_protos.append(param)

    def wrapped_calls(self, sort):
        return self.calls(sort, params=self.wrap_calls)

    def wrapped_protos(self, sort, defaults):
        return self.protos(sort, defaults, params=self.wrap_protos)

    def iterator_calls(self, sort):
        return self.calls(sort, params=self.iter_calls)

    def iterator_2nd_lvl_calls(self, sort):
        return self.calls(sort, params=self.iter_2nd_lvl_calls)

    def iterator_protos(self, sort, defaults):
        return self.protos(sort, defaults, params=self.iter_protos)



_default_parameter_values = \
    { "xcb_timestamp_t" : "XCB_TIME_CURRENT_TIME" }

class Parameter(object):
    def __init__(self, field, c_type="", c_name="", verbose=False):
        self.field = field
        if field != None:
          self.c_type = field.c_field_type
          self.c_name = field.c_field_name
          self.is_const = field.c_field_const_type == "const " + field.c_field_type
          self.is_pointer = field.c_pointer != " "
          # self.serialize = field.type.need_serialize
          self.default = _default_parameter_values.get(self.c_type)
          self.with_default = True
          if verbose:
              sys.stderr.write("c_type: %s; c_name: %s; default: %s\n" \
                      % (self.c_type, self.c_name, self.default))

        else:
          self.c_type = c_type
          self.c_name = c_name
          self.is_const = False
          self.is_pointer = False
          # self.serialize = field.type.need_serialize
          self.default = _default_parameter_values.get(self.c_type)
          self.with_default = True

    def call(self):
        return self.c_name

    def proto(self, with_default):
        c_type = ("const " if self.is_const else "") \
             + self.c_type \
             + (" *" if self.is_pointer else "")
        param = " = " + self.default if with_default and self.default != None else ""
        return c_type + " " + self.c_name + param

########## PARAMETER ##########
