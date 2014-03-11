import sys # stderr

from utils import \
        get_namespace, \
        get_ext_name, \
        _n_item, \
        _ext, \
        _reserved_keywords

from resource_classes import _resource_classes

_field_accessor_template_specialization = \
'''\
template<typename Connection>
template<>
%s
%s<Connection>::%s<%s>(void) const
{
  return %s;
}\
'''

_templates = {}

_templates['field_accessor_template'] = \
'''\
    template<typename ReturnType = %s, typename ... Parameter>
    ReturnType
    %s(Parameter && ... parameter) const
    {
      using make = xpp::generic::factory::make<Connection,
                                               decltype((*this)->%s),
                                               ReturnType,
                                               Parameter ...>;
      return make()((*this)->m_c,
                    this->%s,
                    std::forward<Parameter>(parameter) ...);
    }\
'''

def _field_accessor_template(c_type, method_name, member):
    return _templates['field_accessor_template'] % \
        ( c_type
        , method_name
        , member
        , member
        )

_templates['event_dispatcher_class'] = \
'''\
namespace event {

template<typename Connection>
class dispatcher
  : virtual protected xpp::generic::connection<Connection>
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
}; // class dispatcher

}; // namespace event
'''

def event_dispatcher_class(namespace, cppevents):
    ns = get_namespace(namespace)

    typedef = []
    ctors = []
    members = []

    opcode_switch = "event->response_type & ~0x80"

    # >>> if begin <<<
    if namespace.is_ext:
        opcode_switch = "(event->response_type & ~0x80) - m_first_event"
        '''
        typedef = [ "typedef xpp::%s::extension extension;\n" % ns ]
        '''

        members += \
            [ "private:"
            , "  const uint8_t m_first_event;"
            ]

        ctor = "dispatcher"
        ctors += \
            [ "%s(uint8_t first_event)" % ctor
            , "  : m_first_event(first_event)"
            , "{}"
            , ""
            , "%s(const xpp::%s::extension & extension)" % (ctor, ns)
            , "  : %s(extension->first_event)" % ctor
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
        % (typedef,
           ctors,
           event_switch_cases(cppevents, opcode_switch, "handler", "event"),
           members)

def event_switch_cases(cppevents, arg_switch, arg_handler, arg_event):
    cases = ""
    templ = [ "        case %s:"
            , "          %s(" % arg_handler + "%s<Connection>" + "(static_cast<xpp::generic::connection<Connection &>>(*this).get(), %s));" % arg_event
            , "          return true;"
            , ""
            , ""
            ]

    distinct_events = [[]]
    for e in cppevents:
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

########## EVENT ##########

class CppEvent(object):
    def __init__(self, opcode, opcode_name, c_name, namespace, name, fields):
        self.opcode = opcode
        self.opcode_name = opcode_name
        self.c_name = c_name
        self.namespace = namespace
        self.fields = fields

        self.names = map(str.lower, _n_item(name[-1], True))
        self.name = "_".join(map(str.lower, self.names))

        self.nssopen = ""
        self.nssclose = ""
        self.scope = []
        for name in self.names[0:-1]:
            if name in _reserved_keywords: name += "_"
            self.nssopen += " namespace %s {" % name
            self.nssclose += " };"
            self.scope.append(name)

    def __cmp__(self, other):
        if self.opcode == other.opcode:
            return 0
        elif self.opcode < other.opcode:
            return -1
        else:
            return 1

    def get_name(self):
        return _reserved_keywords.get(self.name, self.name)


    def scoped_name(self):
        ns = get_namespace(self.namespace)
        return "xpp::" + ns + "::event::" + self.get_name()

    def make_class(self):
        member_accessors = []
        member_accessors_special = []
        for field in self.fields:
            if field.field_type[-1] in _resource_classes:
                template_name = field.field_name.capitalize()
                c_type = field.c_field_type
                method_name = field.field_name.lower()
                if (method_name == self.get_name()
                    or method_name in _reserved_keywords):
                    method_name += "_"
                member = field.c_field_name

                member_accessors.append(_field_accessor_template(c_type, method_name, member))

        ns = get_namespace(self.namespace)

        typedef = []
        opcode_accessor = \
            [ "static uint8_t opcode(void)"
            , "{"
            , "  return %s;" % self.opcode_name
            , "}"
            ]

        if self.namespace.is_ext:
            '''
            typedef = [ "typedef xpp::%s::extension extension;" % ns ]
            '''
            opcode_accessor += \
                [ ""
                , "static uint8_t opcode(uint8_t first_event)"
                , "{"
                , "  return first_event + opcode();"
                , "}"
                , ""
                , "static uint8_t opcode(const xpp::%s::extension & extension)" % ns
                , "{"
                , "  return opcode(extension->first_event);"
                , "}"
                ]

        else:
            pass
            '''
            typedef = [ "typedef void extension;" ]
            '''

        if len(opcode_accessor) > 0:
            opcode_accessor = "\n".join(map(lambda s: "    " + s, opcode_accessor)) + "\n"
        else:
            opcode_accessor = ""

        if len(typedef) > 0:
            typedef = "\n".join(map(lambda s: "    " + s, typedef)) + "\n\n"
        else:
            typedef = ""

        if len(member_accessors) > 0:
            member_accessors = "\n" + "\n\n".join(member_accessors) + "\n\n"
            member_accessors_special = "\n" + "\n\n".join(member_accessors_special) + "\n\n"
        else:
            member_accessors = ""
            member_accessors_special = ""

        return \
'''
namespace event {
template<typename Connection>
class %s
  : public xpp::generic::event<Connection,
                               %s,
                               %s>
{
  public:
%s\
    typedef xpp::generic::event<Connection, %s, %s> base;

    using base::base;

    virtual ~%s(void) {}

%s\
%s\
}; // class %s
%s\
}; // namespace event
''' % (self.get_name(), # class %s
       self.c_name, # : public xpp::generic::event<%s,
       self.opcode_name, # %s>
       typedef,
       self.c_name, self.opcode_name, # typedef xpp::generic::event<%s, %s>::base;
       self.get_name(), # virtual ~%s(void) {}
       opcode_accessor,
       member_accessors,
       self.get_name(), # // class %s
       member_accessors_special)
