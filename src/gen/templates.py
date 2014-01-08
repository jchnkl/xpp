# TODO:
"""
* object types (windows, pixmaps)
* connection class for all calls not in object
* Iterator begin, Iterator end instead of std::vector<Type>
* object iterators: e.g. query_tree.children():
* xpp::window & instead of xcb_window_t
* valueparams
* event objects
"""

import sys # sys.stderr.write
import copy # deepcopy

_base_classes = \
    { "WINDOW" : "DRAWABLE"
    , "PIXMAP" : "DRAWABLE"
    }


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
        object_type = self.c_type.replace("xcb_", "").replace("_t", "").upper()
        if _base_classes.has_key(object_type):
            return_type = "xpp::" + object_type.lower()
        else:
            return_type = self.return_type

        return \
"""\
xpp::generic::fixed_size::iterator<
                                   %s,
                                   %s,
                                   %s_reply_t,
                                   %s_%s,
                                   %s_%s_length>\
""" % (self.c_type, \
       return_type, \
       self.c_name, \
       self.c_name, self.member, \
       self.c_name, self.member)


    def iter_variable(self):
        return \
"""\
xpp::generic::variable_size::iterator<
                                      %s,
                                      %s,
                                      %s_reply_t,
                                      %s_iterator_t,
                                      &%s_next,
                                      &%s_sizeof,
                                      &%s_%s_iterator>\
""" % (self.c_type, \
       self.return_type, \
       self.c_name, \
       self.iter_name, \
       self.iter_name, \
       self.iter_name, \
       self.c_name, self.member)


    def list(self, iterator):
        self.return_type = "Type" if self.c_type == "void" else self.c_type
        template = "    template<typename Type>\n" if self.c_type == "void" else ""

        object_type = self.c_type.replace("xcb_", "").replace("_t", "").upper()

        c_tor_params = "m_c, " if _base_classes.has_key(object_type) else ""
        c_tor_params += "this->get()"

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
""" % (self.c_name, iterator, \
       self.member, \
       self.c_name, iterator,
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



########## TEXTUAL_MIXIN ##########

def make_mixin_macro(macro_args, namespace, class_name, requests):
    methods = "#define %s_%s_METHODS(%s) \\\n" \
            % (namespace.upper(), class_name.upper(), macro_args)

    for request in requests:
        wrapped_protos = request.wrapped_protos(True, False)
        wrapped_calls = request.comma() + request.wrapped_calls(False)

        scoped_request = "request"
        if len(namespace) > 0:
            scoped_request = namespace + "::" + scoped_request

        return_type = request.template(indent="", tail=' \\\n')
        if request.is_void:
            return_type += "void"
        else:
            return_type += scoped_request + "::" + request.name

        method_call = scoped_request + "::" + request.name
        if request.is_void:
            method_call += "()"

        return_kw = "" if request.is_void else "return "

        request_name = request.name.replace("_" + class_name.lower(), "")

        methods += \
'''\\
%s \\
%s(%s) \\
{ \\
%s%s(%s%s); \\
} \\
''' % (return_type,
   request_name, wrapped_protos,
   return_kw, method_call, macro_args, wrapped_calls)

    return methods

########## TEXTUAL_MIXIN ##########



########## CONNECTIONCLASS ##########

class ConnectionClass(object):
    def __init__(self, name="connection", c_name="xcb_connection_t"):
        self.name = name
        self.c_name = c_name
        self.requests = []

    def add(self, request):
        self.requests.append(request)

    def set_ns(self, namespace):
        self.namespace = namespace

    def make_mixins(self):
        return make_mixin_macro( \
                self.name.upper(), self.namespace, self.name, self.requests)

    def make_proto(self):
        name = self.name
        c_name = self.c_name
        methods = ""
        for request in self.requests:
            methods += request.make_object_class_proto("")

        return \
"""\
class %s
  // : virtual public xpp::generic::connection<xcb_connection_t>
  : virtual public xpp::generic::connection
{
  public:
    // using xpp::generic::connection::connection;

    %s(void)
    {}

    %s(%s * c)
      : xpp::generic::connection::connection(c)
    {}

    /*
    %s * operator*(void)
    {
      return m_c;
    }

    %s & set(%s * c)
    {
      m_c = c;
      return *this;
    }

    const %s * operator*(void) const
    {
      return m_c;
    }
    */

%s
  protected:
  /*
    xcb_connection_t * m_c = NULL;
    xcb_connection_t * & m_c = xpp::generic::connection::m_c;
  */
}; // class %s
""" % (name, # class
       name, # ctor 1
       name, c_name, # ctor 2
       c_name, # operator*
       name, c_name,
       c_name, # operator* const
       methods,
       name)

    def make_methods(self):
        methods = ""
        for request in self.requests:
            methods += request.make_object_class_call( \
                    True, "", self.name.lower(), "connection") + "\n"
        return methods

########## CONNECTIONCLASS ##########



########## OBJECTCLASS ##########


class ObjectClass(object):
    def __init__(self, namespace, name):
        self.name = name
        # self.namespace = namespace
        self.c_name = 'xcb_' + ("" if namespace == "" else namespace + "_") + name.lower() + '_t'
        self.requests = []

    def get_base_class(self):
        return _base_classes.get(self.name)

    def add(self, request):
        if (len(request.parameter_list.parameter) > 0
                and request.parameter_list.parameter[0].type == self.c_name):
            request_copy = copy.deepcopy(request)
            request_copy.parameter_list.parameter.pop(0)
            request_copy.make_wrapped()
            self.requests.append(request_copy)

    def set_ns(self, namespace):
        self.namespace = namespace

    def make_mixins(self):
        return make_mixin_macro("CONNECTION, " + self.name.upper(), \
                self.namespace, self.name, self.requests)

    def make_proto(self):
        name = self.name.lower()
        methods = ""
        for request in self.requests:
            methods += request.make_object_class_proto(self.name.lower())

        if _base_classes.has_key(self.name):
            return self.ctors_with_inheritance(methods)
        else:
            return self.ctors_without_inheritance(methods)

    def make_methods(self):
        base = _base_classes.get(self.name, self.name)
        methods = ""
        for request in self.requests:
            methods += request.make_object_class_call( \
                    False, "", self.name.lower(), base.lower()) + "\n"
        return methods

    def ctors_without_inheritance(self, methods):
        name = self.name.lower()
        c_name = self.c_name
        return \
"""\
class %s
  : virtual public xpp::generic::connection
{
  public:
    // using xpp::generic::connection::connection;
    // using xpp::generic::connection<xcb_connection_t>::connection;

    %s(void)
    {}

    %s(xcb_connection_t * c)
      // : m_c(c)
      // : xpp::generic::connection<xcb_connection_t>::connection(c)
      : xpp::generic::connection(c)
    {
      connection::set(c);
    }

    %s(xcb_connection_t * c, const %s & %s)
      : xpp::generic::connection(c), m_%s(%s)
      // : m_c(c)
    {}

    /*
    %s(const %s & other)
      : m_c(other.m_c), m_%s(other.m_%s)
    {}
    */

    const %s & get(void)
    {
      return m_%s;
    }

    %s & set(const %s & %s)
    {
      m_%s = %s;
      return *this;
    }

    const %s & operator*(void) const
    {
      return m_%s;
    }

%s
  private:
    /*
    xcb_connection_t * m_c;
    xcb_connection_t * & m_c = xpp::generic::connection::m_c;
    */
    %s m_%s;
}; // class %s
""" % (name, # class
       name, # ctor 0
       name, # ctor 1
       name, c_name, name, # ctor 2
       name, name, # ctor 2
       name, name, # copy ctor
       name, name, # copy ctor
       c_name, name, # get()
       name, c_name, name, # set()
       name, name, # set()
       c_name, name, # const & operator*
       methods,
       c_name, name,
       name)

    def ctors_with_inheritance(self, methods):
        name = self.name.lower()
        base = _base_classes[self.name].lower()
        return \
"""\
class %s : virtual public %s {
  public:
    // using %s::%s;
    %s(void) {}

    %s(xcb_connection_t * c) : connection(c), %s(c)
    {
      // connection::set(c);
    }

    %s(xcb_connection_t * c, const %s & %s)
      : connection(c)
      , %s(c, %s)
    {
      // %s::set(%s);
      // connection::set(c);
    }

%s
  protected:
    /*
    xcb_connection_t * & m_c = xpp::generic::connection::m_c;
    %s & m_%s = %s::m_%s;
    */
}; // class %s
""" % (name, base, # class %s : public %s
       base, base, # using %s::%s
       name, # %s(void) {}
       name, base, # %s(xcb_connection_t * c) : %s( ..
       name, self.c_name, name, # %s(xcb_connection_t * c, const %s & %s) ..
       base, name, # %s::set(%s)
       base, name, # : %s(c, %s)
       methods,
       self.c_name, name, base, base,
       name)

########## OBJECTCLASS ##########



########## REQUESTS  ##########

class CppRequest(object):
    def __init__(self, name, is_void, c_namespace=""):
        self.name = name
        self.is_void = is_void
        self.c_namespace = c_namespace
        self.accessors = []
        self.parameter_list = ParameterList()

    def make_wrapped(self):
        self.parameter_list.make_wrapped()

    def make_proto(self):
        return "  class " + self.name + ";"

    def make_class(self):
        return self.void_request() if self.is_void else self.reply_request()

    def make_object_class_proto(self, class_name):
        return_type = self.template(indent="    ") + "    " \
                + ("void" if self.is_void else "request::" + self.name)
        name = self.name if class_name == "" \
                else self.name.replace("_" + class_name, "")
        return \
"""\
%s %s(%s);
""" % (return_type,
       name, # self.name.replace("_" + class_name, ""),
       self.wrapped_protos(True, True))

    def make_object_class_call(self, is_conn, class_name, base_class_name):
        return_type = self.template(indent="") \
                + "void" if self.is_void else "request::" + self.name

        scoped_class = class_name + "::" if len(class_name) > 0 else class_name

        name = self.name.replace("_" + class_name, "")

        return_kw = "" if self.is_void else "return "

        method_call = "request::" + self.name + ("()" if self.is_void else "")

        call_head = "xpp::generic::connection::get()"
        if not is_conn:
            call_head += ", " + base_class_name + "::get()"

        wrapped_protos = self.wrapped_protos(True, False)
        wrapped_calls = self.comma() + self.wrapped_calls(False)

        return \
"""\
%s
%s%s(%s)
{
  %s%s(%s%s);
}
""" % (return_type,
       scoped_class, name, wrapped_protos,
       return_kw, method_call, call_head, wrapped_calls)

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

    def template(self, indent="    "):
        return indent + "template<typename " \
                + ", typename ".join(self.parameter_list.templates) \
                + ">\n" \
                if len(self.parameter_list.templates) > 0 \
                else ""

    def wrapped_calls(self, sort):
        return self.parameter_list.wrapped_calls(sort)

    def wrapped_protos(self, sort, defaults):
        return self.parameter_list.wrapped_protos(sort, defaults)

    def make_accessors(self):
        return "\n".join(map(lambda a: "\n%s\n" % a, self.accessors))


########## VOID REQUEST  ##########

    def void_request(self):
        def methods(protos, calls, template=""):
            ctor = \
"""\
    %s(xcb_connection_t * c%s)
    {
      %s(c%s);
    }
""" % (self.name, self.comma() + protos, \
       self.c_name(), self.comma() + calls)

            operator = \
"""\
    void
    operator()(xcb_connection_t * c%s)
    {
      %s(c%s);
    }
""" % (self.comma() + protos, \
       self.c_name(), self.comma() + calls)

            return template + ctor + "\n" + template + operator


        head = \
"""\
class %s {
  public:
    %s(void) {}

""" % (self.name, self.name)

        tail = \
"""\
}; // class %s
""" % (self.name)



        wrapped = ""
        if self.parameter_list.want_wrap:
            wrapped = "\n" + \
                methods(self.wrapped_protos(True, True),
                        self.wrapped_calls(False), self.template())

        defaults = ""
        if self.parameter_list.has_defaults and self.parameter_list.is_reordered:
            defaults = "\n" + \
                methods(self.protos(True, True), self.calls(False))

        return head \
             + methods(self.protos(False, False), self.calls(False)) \
             + defaults \
             + wrapped \
             + self.make_accessors() \
             + tail

########## VOID REQUEST  ##########



########## REPLY_REQUEST ##########

    def reply_request(self):
        def methods(protos, calls, template=""):
            return template + \
"""\
    %s(xcb_connection_t * c%s)
      : request(c, &%s%s)
    {}
""" % (self.name, \
       self.comma() + protos, \
       self.c_name(), self.comma() + calls)

        head = \
"""\
class %s
  : public generic::request<%s_cookie_t,
                            %s_reply_t,
                            &%s_reply>
{
  public:
""" % (self.name, \
       self.c_name(), \
       self.c_name(), \
       self.c_name())

        tail = \
"""\
}; // class %s\
""" % (self.name)


        wrapped = ""
        if self.parameter_list.want_wrap:
            wrapped = "\n" + \
                methods(self.wrapped_protos(True, True),
                        self.wrapped_calls(False), self.template())

        defaults = ""
        if self.parameter_list.has_defaults and self.parameter_list.is_reordered:
            defaults = "\n" + \
                methods(self.protos(True, True), self.calls(False))

        return head \
             + methods(self.protos(False, False), self.calls(False)) \
             + defaults \
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
        self.is_reordered = False
        self.parameter = []
        self.wrap_calls = []
        self.wrap_protos = []
        self.templates = []

    def add(self, param):
        if param.default != None:
            self.has_defaults = True
        self.parameter.append(param)

    def comma(self):
        return "" if len(self.parameter) == 0 else ", "

    def calls(self, sort, params=None):
        ps = self.parameter if params == None else params
        if sort:
            tmp = sorted(ps, cmp=lambda p1, p2: cmp(p1.default, p2.default))
            is_reordered = tmp == ps
            ps = tmp
        calls = map(lambda p: p.call(), ps)
        return "" if len(calls) == 0 else ", ".join(calls)

    def protos(self, sort, defaults, params=None):
        if defaults: sort = True
        ps = self.parameter if params == None else params
        if sort:
            tmp = sorted(ps, cmp=lambda p1, p2: cmp(p1.default, p2.default))
            is_reordered = tmp == ps
            ps = tmp
        protos = map(lambda p: p.proto(defaults), ps)
        return "" if len(protos) == 0 else ", ".join(protos)

    def make_wrapped(self):
        self.wrap_calls = []
        self.wrap_protos = []

        # if a parameter is removed, take reduced parameter size into account
        adjust = 0
        for index, param in enumerate(self.parameter):
            prev = index - adjust - 1

            if (param.is_const and param.is_pointer
                    and prev >= 0
                    and self.parameter[prev].name == param.name + "_len"):

                adjust = adjust + 1
                self.want_wrap = True
                self.wrap_calls.pop(prev)
                self.wrap_protos.pop(prev)

                prev_type = self.parameter[prev].type
                if param.type == 'char':
                    self.wrap_calls.append(Parameter(
                        name="static_cast<" + prev_type + ">(" + param.name + '.length())'))
                    self.wrap_calls.append(
                            Parameter(name=param.name + '.c_str()'))
                    self.wrap_protos.append(
                            Parameter(type='const std::string &', name=param.name))

                else:
                    param_type = param.type
                    if param_type == "void":
                        param_type = "Type_" + str(index)
                        self.templates.append(param_type)

                    prev_type = self.parameter[prev].type
                    self.wrap_calls.append(Parameter(
                        name="static_cast<" + prev_type + ">(" + param.name + '.size())'))
                    self.wrap_calls.append(
                            Parameter(name=param.name + '.data()'))
                    self.wrap_protos.append(
                            Parameter(type='const std::vector<' + param_type + '> &',
                                      name=param.name))

            else:
                self.wrap_calls.append(param)
                self.wrap_protos.append(param)

    def wrapped_calls(self, sort):
        return self.calls(sort, params=self.wrap_calls)

    def wrapped_protos(self, sort, defaults):
        return self.protos(sort, defaults, params=self.wrap_protos)



_default_parameter_values =\
    { "xcb_timestamp_t" : "XCB_TIME_CURRENT_TIME" }

class Parameter(object):
    def __init__(self, type="", name="", is_const=False, is_pointer=False):
        self.type = type
        self.name = name
        self.is_const = is_const
        self.is_pointer = is_pointer
        self.default = _default_parameter_values.get(self.type)
        self.with_default = True

    def call(self):
        return self.name

    def proto(self, with_default):
        type = ("const " if self.is_const else "") \
             + self.type \
             + (" *" if self.is_pointer else "")
        param = " = " + self.default if with_default and self.default != None else ""
        return type + " " + self.name + param

########## PARAMETER ##########
