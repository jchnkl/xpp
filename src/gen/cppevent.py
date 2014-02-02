from utils import \
        get_namespace, \
        get_ext_name, \
        _n_item, \
        _ext, \
        _reserved_keywords

from resource_classes import _resource_classes

_field_accessor_template = \
'''\
      template<typename %s = %s>
      %s
      %s(void) const
      {
        return %s(*this, %s);
      }\
'''

_field_accessor_template_specialization = \
'''\
template<>
%s
%s::%s<%s>(void) const
{
  return %s;
}\
'''

########## EVENT ##########

class CppEvent(object):
    def __init__(self, opcode, opcode_name, c_name, namespace, name, fields):
        self.opcode = opcode
        self.opcode_name = opcode_name
        self.c_name = c_name
        self.namespace = namespace
        self.fields = fields

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

    def __cmp__(self, other):
        if self.opcode == other.opcode:
            return 0
        elif self.opcode < other.opcode:
            return -1
        else:
            return 1

    def get_name(self):
        name = self.name
        if self.name in _reserved_keywords: name = self.name + "_"
        scope = ("::".join(self.scope)) if len(self.scope) > 0 else ""
        return (scope + "::" if len(scope) > 0 else "") + name

    def scoped_name(self):
        ns = get_namespace(self.namespace)
        return "xpp::" + ns + "::" + self.get_name() + "::event"

    def make_class(self):
        member_accessors = []
        member_accessors_special = []
        for field in self.fields:
            if field.field_type[-1] in _resource_classes:
                template_name = field.field_name.capitalize()
                c_name = field.c_field_type
                method_name = field.field_name.lower()
                if method_name == "event": method_name += "_"
                member = "(*this)->" + field.c_field_name

                member_accessors.append(_field_accessor_template % \
                    ( template_name, c_name # template<typename %s = %s>
                    , template_name # return type
                    , method_name
                    , template_name, member # return %s(m_c, %s);
                    ))

                member_accessors_special.append(_field_accessor_template_specialization % \
                    ( c_name
                    , "event", method_name, c_name
                    , member
                    ))

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
                , "static uint8_t opcode(uint8_t first_event)"
                , "{"
                , "  return first_event + opcode();"
                , "}"
                , ""
                , "static uint8_t opcode(const xpp::extension::%s & extension)" % ns
                , "{"
                , "  return opcode(extension->first_event);"
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

        if len(member_accessors) > 0:
            member_accessors = "\n" + "\n\n".join(member_accessors) + "\n\n"
            member_accessors_special = "\n" + "\n\n".join(member_accessors_special) + "\n\n"
        else:
            member_accessors = ""
            member_accessors_special = ""

        name = self.name
        if self.name in _reserved_keywords: name = self.name + "_"

        return \
'''
namespace %s {%s namespace %s {
  class event
    : public xpp::event::generic<%s,
                                 %s>
  {
    public:
%s\
      using xpp::event::generic<%s, %s>::generic;

      virtual ~event(void) {}

%s\
%s\
  };
%s\
}; };%s // %s
''' % (ns, self.nssopen, # namespace event { namespace %s {%s
       name, # class %s
       self.opcode_name, # : public xpp::generic::event<%s,
       self.c_name, # %s>
       typedef,
       self.opcode_name, self.c_name, # using xpp::event::generic<%s, %s>::generic;
       # self.name, # virtual ~%s(void) {}
       opcode_accessor,
       member_accessors,
       member_accessors_special,
       self.nssclose, # }; };%s
       self.scoped_name())
       # ("::" + "::".join(self.scope)) if len(self.scope) > 0 else "")
