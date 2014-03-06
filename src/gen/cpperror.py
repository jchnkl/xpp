from utils import \
        get_namespace, \
        get_ext_name, \
        _n_item, \
        _ext, \
        _reserved_keywords

_templates = {}

_templates['error_dispatcher_class'] = \
'''\
namespace error {

class dispatcher {
  public:
%s\
%s\

    void
    operator()(const std::shared_ptr<xcb_generic_error_t> & error) const
    {
%s
    }

%s\
}; // class dispatcher

}; // namespace error
'''

_templates['error_dispatcher_class_impl'] = \
'''\
namespace xpp { namespace %s { namespace error {

void
dispatcher::operator()(xcb_generic_error_t * error) const
{
%s
}

}; }; // namespace xpp::%s::error
'''

def error_dispatcher_class(namespace, cpperrors):
    ns = get_namespace(namespace)

    typedef = []
    ctors = []
    members = []
    opcode_switch = "error->error_code & ~0x80"

    # >>> if begin <<<
    if namespace.is_ext:
        opcode_switch = "(error->error_code & ~0x80) - m_first_error"
        '''
        typedef = [ "typedef xpp::%s::extension extension;\n" % ns ]
        '''

        members += \
            [ "private:"
            , "  const uint8_t m_first_error;"
            ]

        ctor = "dispatcher"
        ctors += \
            [ "%s(uint8_t first_error)" % ctor
            , "  : m_first_error(first_error)"
            , "{}"
            , ""
            , "%s(const xpp::%s::extension & extension)" % (ctor, ns)
            , "  : %s(extension->first_error)" % ctor
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
        % (typedef,
           ctors,
           error_switch_cases(cpperrors, opcode_switch, "error"),
           members)

def error_switch_cases(cpperrors, arg_switch, arg_error):
    cases = ""
    errors = cpperrors
    templ = [ "        case %s: // %s"
            , "          throw %s" + "(%s);" % arg_error
            , ""
            , ""
            ]

    cases += "\n      switch (%s) {\n\n" % arg_switch
    for e in errors:
        cases += "\n".join(templ) % (e.opcode_name, e.opcode, e.scoped_name())
    cases += "      };\n"

    return cases


class CppError(object):
    def __init__(self, error, namespace, name, c_name, opcode, opcode_name):
        self.error = error
        self.namespace = namespace
        self.c_name = c_name
        self.opcode = opcode
        self.opcode_name = opcode_name

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

    def get_name(self):
        return _reserved_keywords.get(self.name, self.name)


    def scoped_name(self):
        ns = get_namespace(self.namespace)
        return "xpp::" + ns + "::error::" + self.get_name()

    def make_class(self):
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
                , "static uint8_t opcode(uint8_t first_error)"
                , "{"
                , "  return first_error + opcode();"
                , "}"
                , ""
                , "static uint8_t opcode(const xpp::%s::extension & extension)" % ns
                , "{"
                , "  return opcode(extension->first_error);"
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

        name = self.name
        if self.name in _reserved_keywords: name = self.name + "_"

        return \
'''
namespace error {
class %s
  : public xpp::generic::error<%s,
                               %s,
                               %s>
{
  public:
%s\
    using xpp::generic::error<%s, %s, %s>::error;

    virtual ~%s(void) {}

%s\
    static constexpr const char * opcode_literal = "%s";
}; // class %s
}; // namespace error
''' % (self.get_name(), # class %s
       self.get_name(), # : public xpp::generic::error<%s,
       self.c_name, # %s,
       self.opcode_name, # : %s>
       typedef,
       self.get_name(), self.c_name, self.opcode_name, # using xpp::generic::error<%s, %s, %s>::error;
       self.get_name(), # virtual ~%s(void) {}
       opcode_accessor,
       self.opcode_name, # static constexpr const char * opcode_literal
       self.get_name()) # // class %s
