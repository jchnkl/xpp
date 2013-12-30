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
