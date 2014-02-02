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
        self.name = self.names[-1]

        self.nssopen = ""
        self.nssclose = ""
        self.scope = []
        for name in self.names[0:-1]:
            if name in _reserved_keywords: name += "_"
            self.nssopen += " namespace %s {" % name
            self.nssclose += " };"
            self.scope.append(name)

    def get_name(self):
        name = self.name
        if self.name in _reserved_keywords: name = self.name + "_"
        scope = ("::".join(self.scope)) if len(self.scope) > 0 else ""
        return (scope + "::" if len(scope) > 0 else "") + name

    def scoped_name(self):
        ns = get_namespace(self.namespace)
        # scope = ("::" + "::".join(self.scope)) if len(self.scope) > 0 else ""
        # scope = ("_".join(self.scope)) if len(self.scope) > 0 else ""
        # return "xpp::error::" + ns + "::" + scope + "_" + self.name
        # return "xpp::error::" + ns + "::" + self.get_name
        return "xpp::" + ns + "::" + self.get_name() + "::error"

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
            opcode_accessor = "\n".join(map(lambda s: "      " + s, opcode_accessor)) + "\n"
        else:
            opcode_accessor = ""

        if len(typedef) > 0:
            typedef = "\n".join(map(lambda s: "      " + s, typedef)) + "\n\n"
        else:
            typedef = ""

        name = self.name
        if self.name in _reserved_keywords: name = self.name + "_"

        return \
'''
namespace %s {%s namespace %s {
  class error
    : public xpp::error::generic<%s,
                                 %s>
  {
    public:
%s\
      using xpp::error::generic<%s, %s>::generic;

      virtual ~error(void) {}

%s\
  };
}; };%s // %s
''' % (ns, self.nssopen, # namespace error { namespace %s {%s
       name, # class %s
       self.opcode_name, # : public xpp::generic::error<%s,
       self.c_name, # %s>
       typedef,
       self.opcode_name, self.c_name, # using xpp::error::generic<%s, %s>::generic;
       # self.name, # virtual ~%s(void) {}
       opcode_accessor,
       self.nssclose, # }; };%s
       self.scoped_name())
       # ("::" + "::".join(self.scope)) if len(self.scope) > 0 else "")
