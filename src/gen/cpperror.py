from utils import \
        get_namespace, \
        get_ext_name, \
        _n_item, \
        _ext, \
        _reserved_keywords

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
            typedef = [ "typedef xpp::extension::%s extension;" % ns ]
            opcode_accessor += \
                [ ""
                , "static uint8_t opcode(uint8_t first_error)"
                , "{"
                , "  return first_error + opcode();"
                , "}"
                , ""
                , "static uint8_t opcode(const xpp::extension::%s & extension)" % ns
                , "{"
                , "  return opcode(extension->first_error);"
                , "}"
                ]

        else:
            typedef = [ "typedef void extension;" ]

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
namespace %s { namespace error {
class %s
  : public xpp::generic::error<%s,
                               %s>
{
  public:
%s\
    using xpp::generic::error<%s, %s>::error;

    virtual ~%s(void) {}

%s\
}; // class %s
}; }; // namespace %s::error
''' % (ns, # namespace %s {
       self.get_name(), # class %s
       self.opcode_name, # : public xpp::generic::error<%s,
       self.c_name, # %s>
       typedef,
       self.opcode_name, self.c_name, # using xpp::generic::error<%s, %s>::error;
       self.get_name(), # virtual ~%s(void) {}
       opcode_accessor,
       self.get_name(), # // class %s
       ns) # }; // namespace %s
