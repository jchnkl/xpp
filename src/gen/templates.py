# [["void", "foo"], ["int", "bar"]] -> "foo, bar"
def param_args(arg_list):
    return (", " if len(arg_list) > 0 else "") \
            + ", ".join(map(lambda x: x[1], arg_list))

# [["void", "foo"], ["int", "bar"]] -> "void foo, int bar"
def param_type_args(arg_list):
    return (", " if len(arg_list) > 0 else "") \
            + ", ".join(map(lambda x: " ".join(x), arg_list))



def ns_head(name):
    return "namespace %s {" % name

def ns_tail(name):
    return "}; // %s" % name



# reply requests

def reply_request_head(name, c_name, args):
    return """\
class %s : public generic::request<
    %s_cookie_t,
    %s_reply_t,
    &%s_reply>
{
  public:\
    """ % (name, c_name, c_name, c_name)

def reply_request_class(name, c_name, args):
    return """\
    %s(xcb_connection_t * c%s)\
    """ % (name, param_type_args(args))

def reply_request_body(name, c_name, args):
    return """\
      : request(c, &%s%s)
    {}\
    """ % (c_name, param_args(args))

def reply_request_tail(name, c_name, args):
    return "}; // class %s" % name

reply_requests = {
    'head'  : reply_request_head,
    'class' : reply_request_class,
    'body'  : reply_request_body,
    'tail'  : reply_request_tail,
    }



# void requests

def void_request_head(name, c_name, args):
    return """\
class %s {
  public:\
    """ % (name)

def void_request_class(name, c_name, args):
    return """\
    %s(xcb_connection_t * c%s)
    {
      %s(c%s);
    }\
    """ % (name, param_type_args(args), c_name, param_args(args))

def void_request_body(name, c_name, args):
    return """\
    void
    operator()(xcb_connection_t * c%s)
    {
      %s(c%s);
    }\
    """ % (param_type_args(args), c_name, param_args(args))

def void_request_tail(name, c_name, args):
    return "}; // class %s" % name

void_requests = {
    'head'  : void_request_head,
    'class' : void_request_class,
    'body'  : void_request_body,
    'tail'  : void_request_tail,
    }



# accessors

def fixed_size_iterator(member, type, iter_name, c_name):
    return """\



########## ACCESSORS ##########

class Accessor(object):
    def __init__(self, is_fixed=False, is_string=False, is_variable=False, \
                 member="", type="", return_type="", iter_name="", c_name=""):

        self.is_fixed = is_fixed
        self.is_string = is_string
        self.is_variable = is_variable

        self.member = member
        self.type = type
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
        return \
"""\
xpp::generic::fixed_size::iterator<
                                   %s,
                                   %s_reply_t,
                                   %s_%s,
                                   %s_%s_length>\
""" % (type, \
       c_name, \
       c_name, member, \
       c_name, member)


def variable_size_iterator(member, type, iter_name, c_name):
    return """\
xpp::generic::variable_size::iterator<
                                      %s,
                                      %s_reply_t,
                                      %s_iterator_t,
                                      &%s_next,
                                      &%s_sizeof,
                                      &%s_%s_iterator>\
""" % (type, \
       c_name, \
       iter_name, \
       iter_name, \
       iter_name, \
       c_name, member)


def list_accessor(member, type, iter_name, c_name, iterator_func):
    iterator = iterator_func(member, type, iter_name, c_name)
    return """\
    xpp::generic::list<%s_reply_t,
                       %s
                      >
    %s(void)
    {
      return xpp::generic::list<%s_reply_t,
                                %s
                               >(this->get());
    }\
""" % (c_name, iterator, \
       member, \
       c_name, iterator)


def string_accessor(member, c_name):
    string = """\
xpp::generic::string<
                     %s_reply_t,
                     &%s_%s,
                     &%s_%s_length>\
""" % (c_name, c_name, member, c_name, member)

    return """\
    %s
    %s(void)
    {
      return %s
               (this->get());
    }\
""" % (string, member, string)



########## REQUESTS  ##########

class CppRequest(object):
    def __init__(self, name, is_void, c_namespace=""):
        self.name = name
        self.is_void = is_void
        self.c_namespace = c_namespace
        self.accessors = []
        self.parameter_list = ParameterList()

    def make_proto(self):
        return "class " + self.name + ";"
        # self.parameter_list.make_wrapped()
        # return self.void_request() if self.is_void else self.reply_request()

    def make_class(self):
        self.parameter_list.make_wrapped()
        return self.void_request() if self.is_void else self.reply_request()

    def add(self, param):
        self.parameter_list.parameter.append(param)

    def comma(self):
        return self.parameter_list.comma()

    def c_name(self):
        ns = "" if self.c_namespace == "" else self.c_namespace + "_"
        return "xcb_" + ns + self.name

    def calls(self):
        return self.parameter_list.calls()

    def protos(self):
        return self.parameter_list.protos()

    def template(self):
        return "    template<typename " \
                + ", typename ".join(self.parameter_list.templates) \
                + ">\n" \
                if len(self.parameter_list.templates) > 0 \
                else ""


    def wrapped_calls(self):
        return self.parameter_list.wrapped_calls()

    def wrapped_protos(self):
        return self.parameter_list.wrapped_protos()

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


        wrapped = "\n" \
                + methods(self.wrapped_protos(), self.wrapped_calls(), self.template()) \
                if self.parameter_list.want_wrap \
                else ""

        return head \
             + methods(self.protos(), self.calls()) \
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


        wrapped = "\n" \
                + methods(self.wrapped_protos(), self.wrapped_calls(), self.template()) \
                if self.parameter_list.want_wrap \
                else ""

        return head \
             + methods(self.protos(), self.calls()) \
             + wrapped \
             + self.make_accessors() \
             + tail

########## REPLY_REQUEST ##########

########## REQUESTS  ##########



########## PARAMETER ##########

class ParameterList(object):
    def __init__(self):
        self.want_wrap = False
        self.parameter = []
        self.wrap_calls = []
        self.wrap_protos = []
        self.templates = []

    def comma(self):
        return "" if len(self.parameter) == 0 else ", "

    def calls(self, params=None):
        params = self.parameter if params == None else params
        calls = map(lambda p: p.call(), params)
        return "" if len(calls) == 0 else ", ".join(calls)

    def protos(self, params=None):
        params = self.parameter if params == None else params
        ps = sorted(params, cmp=lambda p1, p2: cmp(p1.default, p2.default))
        protos = map(lambda p: p.proto(), ps)
        return "" if len(protos) == 0 else ", ".join(protos)

    def make_wrapped(self):
        self.wrap_calls = []
        self.wrap_protos = []

        for index, param in enumerate(self.parameter):
            prev = index - 1

            if (param.is_const and param.is_pointer
                    and prev >= 0
                    and self.parameter[prev].name == param.name + "_len"):

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

    def wrapped_calls(self):
        return self.calls(self.wrap_calls)

    def wrapped_protos(self):
        return self.protos(self.wrap_protos)



_default_parameter_values =\
    { "xcb_timestamp_t" : "XCB_TIME_CURRENT_TIME" }

class Parameter(object):
    def __init__(self, type="", name="", is_const=False, is_pointer=False):
        self.type = type
        self.name = name
        self.is_const = is_const
        self.is_pointer = is_pointer
        self.default = _default_parameter_values.get(self.type)

    def call(self):
        return self.name

    def proto(self):
        type = ("const " if self.is_const else "") \
             + self.type \
             + (" *" if self.is_pointer else "")
        return type + " " + self.name \
                + ("" if self.default == None else " = " + self.default)

########## PARAMETER ##########
