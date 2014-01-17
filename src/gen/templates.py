# TODO:
"""
* object types (windows, pixmaps)
* connection class for all calls not in object
* Iterator begin, Iterator end instead of std::vector<Type>
* object iterators: e.g. query_tree.children():
* xpp::window & instead of xcb_window_t
* valueparams
* event objects
* serialized fields (e.g. xcb_sync_create_alarm_value_list_serialize)
"""

import re # compile
import sys # sys.stderr.write
import copy # deepcopy

_resource_classes = \
    { "DRAWABLE"
    , "WINDOW"
    , "PIXMAP"
    , "ATOM"
    , "CURSOR"
    , "FONT"
    , "GCONTEXT"
    , "FONTABLE"
### XPROTO ###

### RANDR ###
    , "MODE"
    , "CRTC"
    , "OUTPUT"
    , "PROVIDER"
### RANDR ###

    }

def get_namespace(namespace):
    if namespace.is_ext:
        return get_ext_name(namespace.ext_name)
    else:
        return "x"

def get_ext_name(str):
    return _ext(str)

_cname_re = re.compile('([A-Z0-9][a-z]+|[A-Z0-9]+(?![a-z])|[a-z]+)')
_cname_special_cases = {'DECnet':'decnet'}

def _n_item(str):
    '''
    Does C-name conversion on a single string fragment.
    Uses a regexp with some hard-coded special cases.
    '''
    if str in _cname_special_cases:
        return _cname_special_cases[str]
    else:
        split = _cname_re.finditer(str)
        name_parts = [match.group(0) for match in split]
        return '_'.join(name_parts)

_extension_special_cases = ['XPrint', 'XCMisc', 'BigRequests']

def _ext(str):
    '''
    Does C-name conversion on an extension name.
    Has some additional special cases on top of _n_item.
    '''
    if str in _extension_special_cases:
        return _n_item(str).lower()
    else:
        return str.lower()


########## ACCESSORS ##########

class Accessor(object):
    def __init__(self, is_fixed=False, is_string=False, is_variable=False, \
                 member="", c_type="", return_type="", iter_name="", c_name=""):

        self.is_fixed = is_fixed
        self.is_string = is_string
        self.is_variable = is_variable

        self.member = member
        self.c_type = c_type
        self.return_type = return_type
        self.iter_name = iter_name
        self.c_name = c_name

        self.object_type = self.c_type.replace("xcb_", "").replace("_t", "").upper()

        if self.c_type == "void":
          self.return_type = "Type"
        elif self.object_type in _resource_classes:
          self.return_type = self.member.capitalize()
        else:
          self.return_type = self.c_type

    def __str__(self):
        if self.is_fixed:
            return self.list(self.iter_fixed())
        elif self.is_variable:
            return self.list(self.iter_variable())
        elif self.is_string:
            return self.string()
        else:
            return ""


    def iter_fixed(self):
        return_type = self.return_type

        return \
"""\
                     xpp::iterator<%s,
                                   %s,
                                   %s_reply_t,
                                   CALLABLE(%s_%s),
                                   CALLABLE(%s_%s_length)>\
""" % (self.c_type, \
       return_type, \
       self.c_name, \
       self.c_name, self.member, \
       self.c_name, self.member)


    def iter_variable(self):
        return \
"""\
                        xpp::iterator<%s,
                                      %s,
                                      %s_reply_t,
                                      %s_iterator_t,
                                      CALLABLE(%s_next),
                                      CALLABLE(%s_sizeof),
                                      CALLABLE(%s_%s_iterator)>\
""" % (self.c_type, \
       self.return_type, \
       self.c_name, \
       self.iter_name, \
       self.iter_name, \
       self.iter_name, \
       self.c_name, self.member)


    def list(self, iterator):

        template = "    template<typename Type" if self.c_type == "void" else ""

        # template<typename Children = xcb_window_t>
        if self.object_type in _resource_classes:
          template += ", " if template != "" else "    template<typename "
          template += self.member.capitalize() + " = " + self.c_type

        template += ">\n" if template != "" else ""

        c_tor_params = "m_c, this->get()"

        return template + \
"""\
    xpp::generic::list<%s_reply_t,
                       %s
                      >
    %s(void)
    {
      return xpp::generic::list<%s_reply_t,
                                %s
                               >(%s);
    }\
""" % (self.c_name,
       iterator,
       self.member,
       self.c_name,
       iterator,
       c_tor_params)


    def string(self):
        string = \
"""\
xpp::generic::string<
                     %s_reply_t,
                     &%s_%s,
                     &%s_%s_length>\
""" % (self.c_name, \
       self.c_name, self.member, \
       self.c_name, self.member)

        return \
"""\
    %s
    %s(void)
    {
      return %s
               (this->get());
    }\
""" % (string, self.member, string)

########## ACCESSORS ##########



########## PROTOCOLCLASS ##########

class ProtocolClass(object):
    def __init__(self):
        self.requests = []

    def add(self, request):
        self.requests.append(request)

    def set_namespace(self, namespace):
        self.namespace = namespace

    def make_proto(self):
        ns = get_namespace(self.namespace)
        methods = ""
        for request in self.requests:
            methods += request.make_object_class_inline(True) + "\n\n"

        head  = ["class %s" % ns]
        head += ["  : virtual public xpp::xcb::type<xcb_connection_t * const>"]
        if self.namespace.is_ext:
            head += ["  , virtual public extension<&xcb_%s_id>" % ns]
        head += ["{"]

        return \
"""\
namespace protocol {

%s
  public:
    virtual ~%s(void) {}

%s
  private:

}; // class %s

}; // namespace protocol
""" % ("\n".join(head),
       # "\n".join(body),
       ns,
       methods,
       ns)

########## PROTOCOLCLASS ##########



########## OBJECTCLASS ##########

# _ignored = set(
#     'create_window'
#     )

class ObjectClass(object):
    def __init__(self, name): # namespace, name):
        self.name = name
        # self.c_name = \
        #     'xcb_' + ("" if namespace == "" else namespace + "_") + name.lower() + '_t'
        self.requests = []
        # self.namespace = namespace

        # try:
        #     base_class = _base_classes[self.name].lower()
        #     member = ""
        # except: pass


    def add(self, request):
        if (len(request.parameter_list.parameter) > 0
                and request.parameter_list.parameter[0].c_type == self.c_name):
            request_copy = copy.deepcopy(request)
            request_copy.parameter_list.parameter.pop(0)
            request_copy.make_wrapped()
            self.requests.append(request_copy)

    def set_namespace(self, namespace):
        self.namespace = namespace
        self.c_name = "xcb_%s_t"
        name = get_namespace(namespace) + "_" if namespace.is_ext else ""

        self.c_name = self.c_name % (name + self.name.lower())
        # self.c_name = \
        #     'xcb_' + ("_" if namespace.is_ext else namespace + "_") + name.lower() + '_t'

    def make_inline(self):
        ns = get_namespace(self.namespace)
        name = self.name.lower()
        c_name = self.c_name
        methods = ""
        for request in self.requests:
            methods += request.make_object_class_inline(False, self.name.lower()) + "\n\n"

        return \
"""\
namespace %s {

class %s
  : public xpp::xcb::type<xcb_connection_t * const>
  , public xpp::xcb::type<const %s &>
{
  public:
    virtual ~%s(void) {}

%s
}; // class %s

}; // namespace %s
""" % (ns, # namespace %s {
       name,   # class %s
       c_name, # public resource<%s>
       name, # virtual ~%s(void)
       methods,
       name, # }; // class %s
       ns) # }; // namespace %s

########## OBJECTCLASS ##########


_replace_special_classes = \
        { "gcontext" : "gc" }

def replace_class(method, class_name):
    cn = _replace_special_classes.get(class_name, class_name)
    return method.replace("_" + cn, "")

########## REQUESTS  ##########

class CppRequest(object):
    def __init__(self, name, is_void, namespace):
        self.name = name
        self.is_void = is_void
        self.namespace = namespace
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
        return self.void_request() if self.is_void else self.reply_request()

    def make_object_class_inline(self, is_connection, class_name=""):
        request_name = "xpp::request::" + get_namespace(self.namespace) + "::"
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
"""\
    %s
    %s(%s) const
    {
      %s
    }\
""" % (return_type,
       method, proto_params,
       call)

    def add(self, param):
        self.parameter_list.add(param)

    def comma(self):
        return self.parameter_list.comma()

    def c_name(self):
        ns = "" if self.c_namespace == "" else self.c_namespace + "_"
        return "xcb_" + ns + self.name

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

    def void_request(self):
        ############ def methods(...) ############
        def methods(protos, calls, template="", initializer=[]):
            initializer = "\n      ".join([""] + initializer)

            ctor = \
"""\
    %s(xcb_connection_t * c%s)
    {%s
      %s(c%s);
    }
""" % (self.name, self.comma() + protos,
       initializer,
       self.c_name(), self.comma() + calls)

            operator = \
"""\
    void
    operator()(xcb_connection_t * c%s)
    {%s
      %s(c%s);
    }
""" % (self.comma() + protos,
       initializer,
       self.c_name(), self.comma() + calls)

            return template + ctor + "\n" + template + operator
        ############ def methods(...) ############

        namespace = get_namespace(self.namespace)

        head = \
"""\
namespace request { namespace %s {

class %s {
  public:
    %s(void) {}

""" % (namespace, self.name, self.name)

        tail = \
"""\
}; // class %s

}; }; // request::%s
""" % (self.name, namespace)

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
            sys.stderr.write("void %s default_args: %s\n" % (self.name, default_args))

        return head \
             + default \
             + default_args \
             + wrapped \
             + self.make_accessors() \
             + tail

########## VOID REQUEST  ##########



########## REPLY_REQUEST ##########

    def reply_request(self):
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
       self.c_name(), self.comma() + calls)

        ############ def methods(...) ############

        namespace = get_namespace(self.namespace)

        head = \
"""\
namespace request { namespace %s {

class %s
  : public generic::request<%s_cookie_t,
                            %s_reply_t,
                            &%s_reply>
{
  public:
""" % (namespace,
       self.name,
       self.c_name(),
       self.c_name(),
       self.c_name())

        tail = \
"""\
  private:
    xcb_connection_t * m_c;
}; // class %s

}; }; // request::%s
""" % (self.name, namespace)

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
            sys.stderr.write("reply %s default_args: %s\n" % (self.name, default_args))

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
