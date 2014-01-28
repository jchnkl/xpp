# TODO:
"""
* valueparams

* serialized fields (e.g. xcb_sync_create_alarm_value_list_serialize)
  (is this necessary?)

* specialize iterator for non-vector data structures:
  Instead of converting to vector, check if it is possible to send the data
  directly through the socket (e.g. map { key, value }:
  for (k,v : map) { socket_send(v); } ...

* XInput Event handling: Am I doing this right? (Multiple switches etc.)

* Adapter classes for drawable, window, pixmap, atom, font, etc.

$ grep xidtype *.xml
damage.xml:  <xidtype name="DAMAGE" />
glx.xml:     <xidtype name="PIXMAP" />
glx.xml:     <xidtype name="CONTEXT" />
glx.xml:     <xidtype name="PBUFFER" />
glx.xml:     <xidtype name="WINDOW" />
glx.xml:     <xidtype name="FBCONFIG" />
present.xml: <xidtype name="EVENT"/>
randr.xml:   <xidtype name="MODE" />
randr.xml:   <xidtype name="CRTC" />
randr.xml:   <xidtype name="OUTPUT" />
randr.xml:   <xidtype name="PROVIDER" />
record.xml:  <xidtype name="CONTEXT" />
render.xml:  <xidtype name="GLYPHSET" />
render.xml:  <xidtype name="PICTURE" />
render.xml:  <xidtype name="PICTFORMAT" />
shm.xml:     <xidtype name="SEG" />
sync.xml:    <xidtype name="ALARM" />
sync.xml:    <xidtype name="COUNTER" />
sync.xml:    <xidtype name="FENCE" />
xfixes.xml:  <xidtype name="REGION" />
xfixes.xml:  <xidtype name="BARRIER" />
xprint.xml:  <xidtype name="PCONTEXT" />
xproto.xml:  <xidtype name="WINDOW" />
xproto.xml:  <xidtype name="PIXMAP" />
xproto.xml:  <xidtype name="CURSOR" />
xproto.xml:  <xidtype name="FONT" />
xproto.xml:  <xidtype name="GCONTEXT" />
xproto.xml:  <xidtype name="COLORMAP" />
xproto.xml:  <xidtype name="ATOM" />
xvmc.xml:    <xidtype name="CONTEXT" />
xvmc.xml:    <xidtype name="SURFACE" />
xvmc.xml:    <xidtype name="SUBPICTURE" />
xv.xml:      <xidtype name="PORT" />
xv.xml:      <xidtype name="ENCODING" />

"""

import sys # sys.stderr.write
import copy # deepcopy

from utils import \
        get_namespace, \
        get_ext_name, \
        _n_item, \
        _ext

from parameter import *
from resource_classes import _resource_classes

_reserved_keywords = {'class' : '_class',
                      'new'   : '_new',
                      'delete': '_delete',
                      'default' : '_default',
                      'explicit': '_explicit'}

########## ACCESSORS ##########

class Accessor(object):
    def __init__(self, is_fixed=False, is_string=False, is_variable=False, \
                 member="", c_type="", return_type="", iter_name="", c_name=""):

        self.is_fixed = is_fixed
        self.is_string = is_string
        self.is_variable = is_variable

        self.member = member
        self.c_type = c_type
        self.return_type = return_type
        self.iter_name = iter_name
        self.c_name = c_name

        self.object_type = self.c_type.replace("xcb_", "").replace("_t", "").upper()

        if self.c_type == "void":
          self.return_type = "Type"
        elif self.object_type in _resource_classes:
          self.return_type = self.member.capitalize()
        else:
          self.return_type = self.c_type

    def __str__(self):
        if self.is_fixed:
            return self.list(self.iter_fixed())
        elif self.is_variable:
            return self.list(self.iter_variable())
        elif self.is_string:
            return self.string()
        else:
            return ""


    def iter_fixed(self):
        return_type = self.return_type

        return \
"""\
                     xpp::iterator<%s,
                                   %s,
                                   %s_reply_t,
                                   CALLABLE(%s_%s),
                                   CALLABLE(%s_%s_length)>\
""" % (self.c_type, \
       return_type, \
       self.c_name, \
       self.c_name, self.member, \
       self.c_name, self.member)


    def iter_variable(self):
        return \
"""\
                        xpp::iterator<%s,
                                      %s,
                                      %s_reply_t,
                                      %s_iterator_t,
                                      CALLABLE(%s_next),
                                      CALLABLE(%s_sizeof),
                                      CALLABLE(%s_%s_iterator)>\
""" % (self.c_type, \
       self.return_type, \
       self.c_name, \
       self.iter_name, \
       self.iter_name, \
       self.iter_name, \
       self.c_name, self.member)


    def list(self, iterator):

        template = "    template<typename Type" if self.c_type == "void" else ""

        # template<typename Children = xcb_window_t>
        if self.object_type in _resource_classes:
          template += ", " if template != "" else "    template<typename "
          template += self.member.capitalize() + " = " + self.c_type

        template += ">\n" if template != "" else ""

        c_tor_params = "m_c, this->get()"

        return template + \
"""\
    xpp::generic::list<%s_reply_t,
                       %s
                      >
    %s(void)
    {
      return xpp::generic::list<%s_reply_t,
                                %s
                               >(%s);
    }\
""" % (self.c_name,
       iterator,
       self.member,
       self.c_name,
       iterator,
       c_tor_params)


    def string(self):
        string = \
"""\
xpp::generic::string<
                     %s_reply_t,
                     &%s_%s,
                     &%s_%s_length>\
""" % (self.c_name, \
       self.c_name, self.member, \
       self.c_name, self.member)

        return \
"""\
    %s
    %s(void)
    {
      return %s
               (this->get());
    }\
""" % (string, self.member, string)

########## ACCESSORS ##########



########## EXTENSIONCLASS ##########

class ExtensionClass(object):
    def __init__(self, namespace):
        self.namespace = namespace

    def make_class(self):
        # if not self.namespace.is_ext:
        #     return ""
        # else:
        ns = get_namespace(self.namespace)
        if self.namespace.is_ext:
            base = "\n  : public xpp::extension::generic<&xcb_%s_id>\n" % ns
        else:
            base = " "

        return \
'''
namespace protocol { class %s; };
namespace dispatcher { class %s; };

namespace extension {

class %s%s{
  public:
    typedef xpp::protocol::%s protocol;
    typedef xpp::dispatcher::%s dispatcher;
};

}; // namespace extension
''' % (ns, # namespace protocol { class %s; };
       ns, # namespace dispatcher { class %s; };
       ns, # class %s
       base,
       # ns, # : public xpp::extension::generic<&xcb_%s_id>
       ns, # typedef xpp::protocol::%s protocol;
       ns) # typedef xpp::dispatcher::%s dispatcher;

########## EXTENSIONCLASS ##########


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

    def scoped_name(self):
        ns = get_namespace(self.namespace)
        scope = ("::" + "::".join(self.scope)) if len(self.scope) > 0 else ""
        return "xpp::event::" + ns + scope + "::" + self.name

    def make_class(self):
        member_accessors = []
        member_accessors_special = []
        for field in self.fields:
            if field.field_type[-1] in _resource_classes:
                template_name = field.field_name.capitalize()
                c_name = field.c_field_type
                method_name = field.field_name.lower()
                if method_name == self.name: method_name += "_"
                member = "(*this)->" + field.c_field_name

                member_accessors.append(_field_accessor_template % \
                    ( template_name, c_name # template<typename %s = %s>
                    , template_name # return type
                    , method_name
                    , template_name, member # return %s(m_c, %s);
                    ))

                member_accessors_special.append(_field_accessor_template_specialization % \
                    ( c_name
                    , self.name, method_name, c_name
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

        return \
'''
namespace event { namespace %s {%s
  class %s
    : public xpp::event::generic<%s,
                                 %s>
  {
    public:
%s\
      using xpp::event::generic<%s, %s>::generic;

      virtual ~%s(void) {}

%s\
%s\
  };
%s\
}; };%s // xpp::event%s
''' % (ns, self.nssopen, # namespace event { namespace %s {%s
       self.name, # class %s
       self.opcode_name, # : public xpp::generic::event<%s,
       self.c_name, # %s>
       typedef,
       self.opcode_name, self.c_name, # using xpp::event::generic<%s, %s>::generic;
       self.name, # virtual ~%s(void) {}
       opcode_accessor,
       member_accessors,
       member_accessors_special,
       self.nssclose, # }; };%s
       ("::" + "::".join(self.scope)) if len(self.scope) > 0 else "")


########## EVENT ##########



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



########## OBJECTCLASS ##########

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
        for request in self.requests:
            methods += request.make_object_class_inline(False, self.name.lower()) + "\n\n"

        if methods == "":
            return ""
        else:
            return \
"""\
namespace resource { namespace %s {

class %s
  : virtual public xpp::xcb::type<const %s &>
  , virtual protected xpp::xcb::type<xcb_connection_t * const>
{
  public:
    virtual ~%s(void) {}

%s
}; // class %s

}; }; // namespace resource::%s
""" % (name, # namespace %s {
       ns,   # class %s
       c_name, # public resource<%s>
       ns, # virtual ~%s(void)
       methods,
       ns, # }; // class %s
       name) # }; // namespace %s

########## OBJECTCLASS ##########
