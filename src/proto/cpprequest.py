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
  xpp::generic::check<Connection, xpp::%s::error::dispatcher>(
      std::forward<Connection>(c),
      xcb_%s_checked(
          std::forward<Connection>(c),
          std::forward<Parameter>(parameter) ...));
}

template<typename ... Parameter>
void
%s(Parameter && ... parameter)
{
  xcb_%s(std::forward<Parameter>(parameter) ...);
}
'''

def _void_request_function(ns, name):
    return _templates['void_request_function'] % \
            ( name
            , ns
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

    def add(self, param):
        self.parameter_list.add(param)

    def make_wrapped(self):
        self.parameter_list.make_wrapped()

    def make_class(self):
        cppcookie = CppCookie(self.namespace, self.is_void, self.request.name, self.reply, self.parameter_list)

        if self.is_void:
            void_functions = cppcookie.make_void_functions()
            if len(void_functions) > 0:
                return void_functions
            else:
                return _void_request_function(get_namespace(self.namespace), self.request_name)

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
