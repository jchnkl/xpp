# vim: set ts=4 sws=4 sw=4:

import sys # stderr
import copy # deepcopy

from utils import \
        get_namespace, \
        get_ext_name, \
        _n_item, \
        _ext

# _ctor_dtor_kws = { "Create", "Destroy", "Open", "Close", "Free" }

class ObjectClass(object):
    def __init__(self, name, use_ns=True):
        self.name = name
        self.use_ns = use_ns
        self.requests = []

    def add(self, request):
        if (len(request.parameter_list.parameter) > 0
                and request.parameter_list.parameter[0].c_type == self.c_name):
            request_copy = copy.deepcopy(request)
            request_copy.parameter_list.parameter.pop(0)
            request_copy.make_wrapped()
            self.requests.append(request_copy)

    def set_namespace(self, namespace):
        self.namespace = namespace
        name = (get_namespace(namespace) + "_") if self.use_ns else ""
        self.c_name = "xcb_%s_t" % (name + self.name.lower())

    def make_inline(self):
        ns = get_namespace(self.namespace)
        name = self.name.lower()
        c_name = self.c_name
        methods = ""

        # sys.stderr.write("ObjectClass %s\n\n" % self.name)
        for request in self.requests:
            # for kw in _ctor_dtor_kws:
            #     if request.request.name[-1].startswith(kw):
            #       sys.stderr.write("request: %s, opcode: %s\n\n" \
            #           % (request.request.name, request.request.opcode))

            methods += request.make_object_class_inline(False, self.name.lower()) + "\n\n"

        if methods == "":
            return ""
        else:
            return \
"""\
// namespace %s {

template<typename Connection>
class %s
  : virtual public xpp::xcb::type<const %s &>
  // , virtual protected xpp::xcb::type<xcb_connection_t * const>
  , virtual protected xpp::generic::connection<Connection>
{
  protected:
    using connection = xpp::generic::connection<Connection>;

  public:
    virtual ~%s(void) {}

%s
}; // class %s

// }; // namespace %s
""" % (ns, # namespace %s {
       name,   # class %s
       c_name, # public xpp::xcb::type<const %s &>
       name, # virtual ~%s(void)
       methods,
       name, # }; // class %s
       ns) # }; // namespace %s
