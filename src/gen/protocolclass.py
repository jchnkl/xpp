# vim: set ts=4 sws=4 sw=4:

from utils import \
        get_namespace, \
        get_ext_name, \
        _n_item, \
        _ext

_templates = {}

_templates['protocol_class'] = \
"""\
namespace protocol {

class %s
  : virtual protected xpp::xcb::type<xcb_connection_t * const>
{
  public:
%s\
    virtual ~%s(void) {}

%s\
}; // class %s

}; // namespace protocol
"""

_templates['event_dispatcher_class'] = \
'''\
namespace dispatcher { namespace event {

class %s
  : virtual protected xpp::xcb::type<xcb_connection_t * const>
{
  public:
%s\
%s\

    template<typename Handler>
    bool
    operator()(const Handler & handler, xcb_generic_event_t * const event) const
    {
%s
      return false;
    }
%s\
}; // class %s

}; }; // namespace dispatcher::event
'''

_templates['error_dispatcher_class'] = \
'''\
namespace dispatcher { namespace error {

class %s {
  public:
%s\
%s\

    void
    operator()(xcb_generic_error_t * const error) const
    {
%s
    }

%s\
}; // class %s

}; }; // namespace dispatcher::error
'''


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
            typedef = [ "typedef xpp::extension::%s extension;" % ns ]

        if len(typedef) > 0:
            typedef = "".join(map(lambda s: "    " + s, typedef)) + "\n\n"
        else:
            typedef = ""

        return _templates['protocol_class'] \
            % (ns, # class %s {
               typedef,
               ns,
               methods,
               ns) \
            + "\n\n" \
            + self.event_dispatcher_class() \
            + "\n\n" \
            + self.error_dispatcher_class()

    ### event_dispatcher_class

    def event_dispatcher_class(self):
        ns = get_namespace(self.namespace)

        typedef = []
        ctors = []
        members = []

        opcode_switch = "event->response_type & ~0x80"

        # >>> if begin <<<
        if self.namespace.is_ext:
            opcode_switch = "(event->response_type & ~0x80) - m_first_event"
            typedef = [ "typedef xpp::extension::%s extension;\n" % ns ]

            members += \
                [ "private:"
                , "  const uint8_t m_first_event;"
                ]

            ctors += \
                [ "%s(uint8_t first_event)" % ns
                , "  : m_first_event(first_event)"
                , "{}"
                , ""
                , "%s(const xpp::extension::%s & extension)" % (ns, ns)
                , "  : %s(extension->first_event)" % ns
                , "{}"
                ]

        # >>> if end <<<

        if len(typedef) > 0:
            typedef = "\n".join(map(lambda s: "    " + s, typedef)) + "\n"
        else:
            typedef = ""

        if len(ctors) > 0:
            ctors = "\n".join(map(lambda s: "    " + s, ctors)) + "\n"
        else:
            ctors = ""

        if len(members) > 0:
            members = "\n".join(map(lambda s: "  " + s, members)) + "\n"
        else:
            members = ""

        return _templates['event_dispatcher_class'] \
            % (ns, # class %s {
               typedef,
               ctors,
               self.event_switch_cases(opcode_switch, "handler", "event"),
               members,
               ns) # }; // class %s

    def event_switch_cases(self, arg_switch, arg_handler, arg_event):
        cases = ""
        templ = [ "        case %s:"
                , "          %s(" % arg_handler + "%s" + "(*this, %s));" % arg_event
                , "          return true;"
                , ""
                , ""
                ]

        distinct_events = [[]]
        for e in self.events:
            done = False
            for l in distinct_events:
                if e in l:
                    continue
                else:
                    l.append(e)
                    done = True
                    break

            if not done:
                distinct_events.append([e])
            else:
                continue

        for l in distinct_events:
            cases += "\n      switch (%s) {\n\n" % arg_switch
            for e in l:
                cases += "\n".join(templ) % (e.opcode_name, e.scoped_name())
            cases += "      };\n"

        return cases

    ### error_dispatcher_class

    def error_dispatcher_class(self):
        ns = get_namespace(self.namespace)

        typedef = []
        ctors = []
        members = []

        # >>> if begin <<<
        if self.namespace.is_ext:
            typedef = [ "typedef xpp::extension::%s extension;\n" % ns ]

            members += \
                [ "private:"
                , "  const uint8_t m_first_error;"
                ]

            ctors += \
                [ "%s(uint8_t first_error)" % ns
                , "  : m_first_error(first_error)"
                , "{}"
                , ""
                , "%s(const xpp::extension::%s & extension)" % (ns, ns)
                , "  : %s(extension->first_error)" % ns
                , "{}"
                ]

        # >>> if end <<<

        if len(typedef) > 0:
            typedef = "\n".join(map(lambda s: "    " + s, typedef)) + "\n"
        else:
            typedef = ""

        if len(ctors) > 0:
            ctors = "\n".join(map(lambda s: "    " + s, ctors)) + "\n"
        else:
            ctors = ""

        if len(members) > 0:
            members = "\n".join(map(lambda s: "  " + s, members)) + "\n"
        else:
            members = ""

        return _templates['error_dispatcher_class'] \
            % (ns, # class %s {
               typedef,
               ctors,
               self.error_switch_cases("error->error_code", "error"),
               members,
               ns) # }; // class %s

    def error_switch_cases(self, arg_switch, arg_error):
        cases = ""
        errors = self.errors
        templ = [ "        case %s:"
                , "          throw %s" + "(%s);" % arg_error
                , ""
                , ""
                ]

        cases += "\n      switch (%s) {\n\n" % arg_switch
        for e in errors:
            cases += "\n".join(templ) % (e.opcode_name, e.scoped_name())
            # cases += "\n".join(templ) % (e.opcode_name, e.get_name())
        cases += "      };\n"

        return cases


########## PROTOCOLCLASS ##########
