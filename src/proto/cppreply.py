from utils import _n, _ext, _n_item, get_namespace
from resource_classes import _resource_classes

_templates = {}

_templates['reply_class'] = \
'''\
namespace reply {
template<typename Connection, typename Check>
class %s
  : public xpp::generic::reply<%s<Connection, Check>,
                               Connection,
                               Check,
                               SIGNATURE(%s_reply),
                               SIGNATURE(%s)>
{
  public:
    typedef xpp::generic::reply<%s<Connection, Check>,
                                Connection,
                                Check,
                                SIGNATURE(%s_reply),
                                SIGNATURE(%s)>
                                  base;

    template<typename C, typename ... Parameter>
    %s(C && c, Parameter && ... parameter)
      : base(std::forward<C>(c), std::forward<Parameter>(parameter) ...)
    {}

    void
    handle(const std::shared_ptr<xcb_generic_error_t> & error)
    {
      using error_handler =
        xpp::generic::error_handler<Connection, xpp::%s::error::dispatcher>;
      (error_handler(base::m_c))(error);
    }

%s\
%s\
}; // class %s
}; // namespace reply
'''

def _reply_class(name, c_name, ns, cookie, accessors):
    return _templates['reply_class'] % \
            ( name
            , name # base class
            , c_name # base class
            , c_name # base class
            , name # typedef
            , c_name # typedef
            , c_name # typedef base
            , name # c'tor
            , ns
            , cookie.make_static_getter()
            , accessors
            , name
            )

_templates['reply_member_accessor'] = \
'''\
    template<typename ReturnType = %s, typename ... Parameter>
    ReturnType
    %s(Parameter && ... parameter)
    {
      using make = xpp::generic::factory::make<Connection,
                                               decltype(this->get()->%s),
                                               ReturnType,
                                               Parameter ...>;
      return make()(this->m_c,
                    this->get()->%s,
                    std::forward<Parameter>(parameter) ...);
    }
'''

def _reply_member_accessor(request_name, name, c_type, template_type):
    return _templates['reply_member_accessor'] % \
            ( c_type
            , name
            , name
            , name
            )

class CppReply(object):
    def __init__(self, namespace, name, cookie, reply, accessors, parameter_list):
        self.namespace = namespace
        self.name = name
        self.reply = reply
        self.cookie = cookie
        self.accessors = accessors
        self.parameter_list = parameter_list
        self.request_name = _ext(_n_item(self.name[-1]))
        self.c_name = "xcb" \
            + (("_" + get_namespace(namespace)) if namespace.is_ext else "") \
            + "_" + self.request_name

    def make_accessors(self):
        return "\n".join(map(lambda a: "\n%s\n" % a, self.accessors))

    def make(self):
        accessors = [self.make_accessors()]
        naccessors = len(self.accessors)

        for field in self.reply.fields:
            if (field.field_type[-1] in _resource_classes
                and not field.type.is_list
                and not field.type.is_container):

                naccessors = naccessors + 1

                name = field.field_name.lower()
                c_type = field.c_field_type
                template_type = field.field_name.capitalize()

                accessors.append(_reply_member_accessor(self.request_name, name, c_type, template_type))

        result = ""
        result += _reply_class(
            self.request_name, self.c_name, get_namespace(self.namespace),
            self.cookie, "\n".join(accessors))
        return result
