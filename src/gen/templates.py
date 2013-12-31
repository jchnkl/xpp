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
