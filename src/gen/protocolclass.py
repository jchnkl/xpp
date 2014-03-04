# vim: set ts=4 sws=4 sw=4:

from utils import \
        get_namespace, \
        get_ext_name, \
        _n_item, \
        _ext

from cppevent import event_dispatcher_class
from cpperror import error_dispatcher_class

_templates = {}

_templates['protocol_class'] = \
"""\
// namespace %s {

template<typename Connection>
class protocol
  // : virtual protected xpp::xcb::type<xcb_connection_t * const>
  : virtual protected xpp::generic::connection<Connection>
{
  protected:
    using connection = xpp::generic::connection<Connection>;

  public:
%s\
    virtual ~protocol(void) {}

%s\
}; // class protocol

// }; // namespace %s
"""

_ignore_events = \
        { "XCB_PRESENT_GENERIC" }

########## PROTOCOLCLASS ##########

class ProtocolClass(object):
    def __init__(self):
        self.requests = []
        self.events = []
        self.errors = []

    def add(self, request):
        self.requests.append(request)

    def add_event(self, event):
        if event.opcode_name not in _ignore_events:
            self.events.append(event)

    def add_error(self, error):
        self.errors.append(error)

    def set_namespace(self, namespace):
        self.namespace = namespace

    def make_proto(self):
        ns = get_namespace(self.namespace)
        methods = ""
        for request in self.requests:
            methods += request.make_object_class_inline(True) + "\n\n"

        typedef = []
        if self.namespace.is_ext:
            typedef = [ "typedef xpp::%s::extension extension;" % ns ]

        if len(typedef) > 0:
            typedef = "".join(map(lambda s: "    " + s, typedef)) + "\n\n"
        else:
            typedef = ""

        return _templates['protocol_class'] \
            % (ns, # namespace %s {
               typedef,
               methods,
               ns) # \
            # + "\n\n" \
            # + event_dispatcher_class(self.namespace, self.events) \
            # + "\n\n" \
            # + error_dispatcher_class(self.namespace, self.errors)

########## PROTOCOLCLASS ##########
