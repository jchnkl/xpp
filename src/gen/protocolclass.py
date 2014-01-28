from utils import \
        get_namespace, \
        get_ext_name, \
        _n_item, \
        _ext

_ignore_events = \
        { "XCB_PRESENT_GENERIC" }

########## PROTOCOLCLASS ##########

class ProtocolClass(object):
    def __init__(self):
        self.requests = []
        self.events = []

    def add(self, request):
        self.requests.append(request)

    def add_event(self, event):
        if event.opcode_name not in _ignore_events:
            self.events.append(event)

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

        return \
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
""" % (ns, # class %s {
       typedef,
       ns,
       methods,
       ns) + "\n\n" + self.event_dispatcher_class()

    def event_dispatcher_class(self):
        ns = get_namespace(self.namespace)

        fst_param = ""
        snd_param = ""
        typedef = []

        ctors = []
        members = []

        opcode_switch = "event->response_type & ~0x80"

        # >>> if begin <<<
        if self.namespace.is_ext:
            opcode_switch = "(event->response_type & ~0x80) - m_first_event"
            fst_param = ", m_first_event"
            snd_param = ", uint8_t first_event"
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

        return \
'''\
namespace dispatcher {

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

}; // namespace dispatcher
''' % (ns, # class %s {
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

########## PROTOCOLCLASS ##########
