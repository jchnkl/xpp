# TODO:
"""
* valueparams

* serialized fields (e.g. xcb_sync_create_alarm_value_list_serialize)

* specialize iterator for non-vector data structures:
  Instead of converting to vector, check if it is possible to send send the data
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

import re # compile
import sys # sys.stderr.write
import copy # deepcopy

_resource_classes = \
    {
    ### XPROTO ###
      "WINDOW"
    , "PIXMAP"
    , "CURSOR"
    , "FONT"
    , "GCONTEXT"
    , "COLORMAP"
    , "ATOM"
    , "DRAWABLE"
    , "FONTABLE"
    ### XPROTO ###

    ### DAMAGE ###
    , "DAMAGE"
    ### DAMAGE ###

    ### GLX, RECORD, XVMC ###
    , "CONTEXT"
    ### GLX, RECORD, XVMC ###

    ### GLX ###
    # , "PIXMAP" # already in XPROTO
    # , "CONTEXT"
    , "PBUFFER"
    # , "WINDOW" # already in XPROTO
    , "FBCONFIG"
    ### GLX ###

    ### PRESENT ###
    , "EVENT"
    ### PRESENT ###

    ### RANDR ###
    , "MODE"
    , "CRTC"
    , "OUTPUT"
    , "PROVIDER"
    ### RANDR ###

    ### RECORD ###
    # , "CONTEXT"
    ### RECORD ###

    ### RENDER ###
    , "GLYPHSET"
    , "PICTURE"
    , "PICTFORMAT"
    ### RENDER ###

    ### SHM ###
    , "SEG"
    ### SHM ###

    ### SYNC ###
    , "ALARM"
    , "COUNTER"
    , "FENCE"
    ### SYNC ###

    ### XFIXES ###
    , "REGION"
    , "BARRIER"
    ### XFIXES ###

    ### XPRINT ###
    , "PCONTEXT"
    ### XPRINT ###

    ### XVMC ###
    # , "CONTEXT"
    , "SURFACE"
    , "SUBPICTURE"
    ### XVMC ###

    ### XV ###
    , "PORT"
    , "ENCODING"
    ### XV ###
    }

_reserved_keywords = {'class' : '_class',
                      'new'   : '_new',
                      'delete': '_delete',
                      'default' : '_default',
                      'explicit': '_explicit'}

def get_namespace(namespace):
    if namespace.is_ext:
        return get_ext_name(namespace.ext_name)
    else:
        return "x"

def get_ext_name(str):
    return _ext(str)

_cname_re = re.compile('([A-Z0-9][a-z]+|[A-Z0-9]+(?![a-z])|[a-z]+)')
_cname_special_cases = {'DECnet':'decnet'}

def _n_item(str, parts=False):
    '''
    Does C-name conversion on a single string fragment.
    Uses a regexp with some hard-coded special cases.
    '''
    if str in _cname_special_cases:
        return _cname_special_cases[str]
    else:
        split = _cname_re.finditer(str)
        name_parts = [match.group(0) for match in split]
        if parts:
          return name_parts
        else:
          return '_'.join(name_parts)

_extension_special_cases = ['XPrint', 'XCMisc', 'BigRequests']

def _ext(str):
    '''
    Does C-name conversion on an extension name.
    Has some additional special cases on top of _n_item.
    '''
    if str in _extension_special_cases:
        return _n_item(str).lower()
    else:
        return str.lower()


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
                cases += "\n".join(templ) % (e.opcode, e.scoped_name())
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


_replace_special_classes = \
        { "gcontext" : "gc" }

def replace_class(method, class_name):
    cn = _replace_special_classes.get(class_name, class_name)
    return method.replace("_" + cn, "")

########## REQUESTS  ##########

class CppRequest(object):
    def __init__(self, name, is_void, namespace, reply):
        self.name = name
        self.is_void = is_void
        self.namespace = namespace
        self.reply = reply
        self.c_namespace = \
            "" if namespace.header.lower() == "xproto" \
            else get_namespace(namespace)
        self.accessors = []
        self.parameter_list = ParameterList()

    def make_wrapped(self):
        self.parameter_list.make_wrapped()

    def make_proto(self):
        return "  class " + self.name + ";"

    def make_class(self):
        if self.is_void:
            return self.void_request(True) + "\n\n" + self.void_request(False)
        else:
            return self.reply_request(True) + "\n\n" + self.reply_request(False)

    def make_object_class_inline(self, is_connection, class_name=""):
        request_name = "xpp::request::" + get_namespace(self.namespace) + "::"
        return_type = self.iterator_template(indent="")
        return_type += "virtual\n" if return_type == "" else ""
        return_type += "    "
        if self.is_void:
            return_type += "void"
        else:
            return_type += request_name + self.name

        method = self.name
        if not is_connection:
            method = replace_class(method, class_name)

        calls = self.iterator_2nd_lvl_calls(False)
        proto_params = self.iterator_protos(True, True)

        call_params = ["*this"]
        if not is_connection: call_params += ["*this"]

        if len(calls) > 0: call_params += [calls]

        call = ("" if self.is_void else "return ") \
             + request_name + self.name \
             + ("()" if self.is_void else "") \
             + "(" + ", ".join(call_params) + ");"

        return \
"""\
    %s
    %s(%s) const
    {
      %s
    }\
""" % (return_type,
       method, proto_params,
       call)

    def add(self, param):
        self.parameter_list.add(param)

    def comma(self):
        return self.parameter_list.comma()

    def c_name(self, regular=True):
        ns = "" if self.c_namespace == "" else (self.c_namespace + "_")
        name = "xcb_" + ns + self.name

        # checked = void and not regular
        # unchecked = not void and not regular
        if not regular:
            if self.is_void:
                return name + "_checked"
            else:
                return name + "_unchecked"
        else:
            return name

    def calls(self, sort):
        return self.parameter_list.calls(sort)

    def protos(self, sort, defaults):
        return self.parameter_list.protos(sort, defaults)

    def template(self, indent="    ", tail="\n"):
        return indent + "template<typename " \
                + ", typename ".join(self.parameter_list.templates) \
                + ">" + tail \
                if len(self.parameter_list.templates) > 0 \
                else ""

    def iterator_template(self, indent="    ", tail="\n"):
        return indent + "template<typename " \
                + ", typename ".join(self.parameter_list.templates \
                                   + self.parameter_list.iterator_templates) \
                + ">" + tail \
                if len(self.parameter_list.iterator_templates) > 0 \
                else ""

    def wrapped_calls(self, sort):
        return self.parameter_list.wrapped_calls(sort)

    def wrapped_protos(self, sort, defaults):
        return self.parameter_list.wrapped_protos(sort, defaults)

    def iterator_calls(self, sort):
        return self.parameter_list.iterator_calls(sort)

    def iterator_2nd_lvl_calls(self, sort):
        return self.parameter_list.iterator_2nd_lvl_calls(sort)

    def iterator_protos(self, sort, defaults):
        return self.parameter_list.iterator_protos(sort, defaults)

    def iterator_initializers(self):
        return self.parameter_list.iterator_initializers()

    def make_accessors(self):
        return "\n".join(map(lambda a: "\n%s\n" % a, self.accessors))


########## VOID REQUEST  ##########

    def void_request(self, regular):
        ############ def methods(...) ############
        def methods(protos, calls, template="", initializer=[]):
            initializer = "\n      ".join([""] + initializer)

            ctor = \
"""\
    %s(xcb_connection_t * c%s)
    {%s
      %s(c%s);
    }
""" % (self.name, self.comma() + protos,
       initializer,
       self.c_name(regular), self.comma() + calls)

            operator = \
"""\
    void
    operator()(xcb_connection_t * c%s)
    {%s
      %s(c%s);
    }
""" % (self.comma() + protos,
       initializer,
       self.c_name(regular), self.comma() + calls)

            return template + ctor + "\n" + template + operator
        ############ def methods(...) ############

        namespace = get_namespace(self.namespace)
        # checked = void and not regular
        checked_open = ""
        checked_close = ""
        checked_comment = ""
        if not regular:
            checked_open = " namespace checked {"
            checked_close = " };"
            checked_comment = "checked::"

        head = \
"""\
namespace request {%s namespace %s {

class %s {
  public:
    %s(void) {}

""" % (checked_open, namespace, self.name, self.name)

        tail = \
"""\
}; // class %s

}; };%s // request::%s%s
""" % (self.name, checked_close, checked_comment, namespace)

        default = methods(self.protos(False, False), self.calls(False))

        if (self.parameter_list.has_defaults):
            default = methods(self.protos(True, True), self.calls(False))

        wrapped = ""
        if self.parameter_list.want_wrap:
            wrapped = "\n" + \
                methods(self.iterator_protos(True, True),
                        self.iterator_calls(False), self.iterator_template(),
                        self.iterator_initializers())

        default_args = ""
        if self.parameter_list.is_reordered():
            default_args = "\n" + \
                methods(self.protos(True, True), self.calls(False))

        return head \
             + default \
             + default_args \
             + wrapped \
             + self.make_accessors() \
             + tail

########## VOID REQUEST  ##########



########## REPLY_REQUEST ##########

    def reply_request(self, regular):
        ############ def methods(...) ############
        def methods(protos, calls, template="", initializer=[]):
            initializer = "\n      ".join([""] + initializer)

            return template + \
"""\
    %s(xcb_connection_t * c%s)
      : request(c), m_c(c)
    {%s
      request::prepare(&%s%s);
    }
""" % (self.name,
       self.comma() + protos,
       initializer,
       self.c_name(regular), self.comma() + calls)

        ############ def methods(...) ############

        member_accessors = []
        member_accessors_special = []
        for field in self.reply.fields:
            if (field.field_type[-1] in _resource_classes
                and not field.type.is_list
                and not field.type.is_container):

                template_name = field.field_name.capitalize()
                c_name = field.c_field_type
                method_name = field.field_name.lower()
                if method_name == self.name: method_name += "_"
                member = "m_reply->" + field.c_field_name

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

        if len(member_accessors) > 0:
            member_accessors = "\n" + "\n\n".join(member_accessors) + "\n\n"
            member_accessors_special = "\n" + "\n\n".join(member_accessors_special) + "\n\n"
        else:
            member_accessors = ""
            member_accessors_special = ""

        namespace = get_namespace(self.namespace)

        # unchecked = not void and not regular
        unchecked_open = ""
        unchecked_close = ""
        unchecked_comment = ""
        if not regular:
            unchecked_open = " namespace unchecked {"
            unchecked_close = " };"
            unchecked_comment = "unchecked::"

        head = \
"""\
namespace request {%s namespace %s {

class %s
  : public generic::request<%s_cookie_t,
                            %s_reply_t,
                            &%s_reply>
{
  public:
""" % (unchecked_open, namespace,
       self.name,
       self.c_name(),
       self.c_name(),
       self.c_name())

        tail = \
"""\
%s\
  private:
    xcb_connection_t * m_c;
}; // class %s
%s\
}; };%s // request::%s%s
""" % (member_accessors,
       self.name,
       member_accessors_special,
       unchecked_close,
       unchecked_comment,
       namespace)

        default = methods(self.protos(False, False), self.calls(False))

        if self.parameter_list.has_defaults:
            default = methods(self.protos(True, True), self.calls(False))

        wrapped = ""
        if self.parameter_list.want_wrap:
            wrapped = "\n" + \
                methods(self.iterator_protos(True, True),
                        self.iterator_calls(False), self.iterator_template(),
                        self.iterator_initializers())

        default_args = ""
        if self.parameter_list.is_reordered():
            default_args = "\n" + \
                methods(self.protos(True, True), self.calls(False))

        return head \
             + default \
             + default_args \
             + wrapped \
             + self.make_accessors() \
             + tail

########## REPLY_REQUEST ##########

########## REQUESTS  ##########



########## PARAMETER ##########

class ParameterList(object):
    def __init__(self):
        self.want_wrap = False
        self.has_defaults = False
        self.parameter = []
        self.wrap_calls = []
        self.wrap_protos = []
        self.iter_calls = []
        self.iter_2nd_lvl_calls = []
        self.iter_protos = []
        self.templates = []
        self.iterator_templates = []
        self.initializer = []

    def add(self, param):
        self.has_defaults = param.default != None
        self.parameter.append(param)

    def comma(self):
        return "" if len(self.parameter) == 0 else ", "

    def is_reordered(self):
        tmp = sorted(self.parameter, cmp=lambda p1, p2: cmp(p1.default, p2.default))
        return tmp != self.parameter

    def calls(self, sort, params=None):
        ps = self.parameter if params == None else params
        if sort:
            tmp = sorted(ps, cmp=lambda p1, p2: cmp(p1.default, p2.default))
            ps = tmp
        calls = map(lambda p: p.call(), ps)
        return "" if len(calls) == 0 else ", ".join(calls)

    def protos(self, sort, defaults, params=None):
        if defaults: sort = True
        ps = self.parameter if params == None else params
        if sort:
            tmp = sorted(ps, cmp=lambda p1, p2: cmp(p1.default, p2.default))
            ps = tmp
        protos = map(lambda p: p.proto(defaults), ps)
        return "" if len(protos) == 0 else ", ".join(protos)

    def iterator_initializers(self):
        return self.initializer

    def make_wrapped(self):
        self.wrap_calls = []
        self.wrap_protos = []
        self.iter_calls = []
        self.iter_2nd_lvl_calls = []
        self.iter_protos = []
        self.initializer = []
        self.templates = []
        self.iterator_templates = []

        # if a parameter is removed, take reduced parameter size into account
        adjust = 0
        for index, param in enumerate(self.parameter):
            prev = index - adjust - 1

            if (param.is_const and param.is_pointer
                    and prev >= 0
                    and self.parameter[prev].c_name == param.c_name + "_len"):

                adjust = adjust + 1
                self.want_wrap = True
                self.wrap_calls.pop(prev)
                self.wrap_protos.pop(prev)
                self.iter_calls.pop(prev)
                self.iter_2nd_lvl_calls.pop(prev)
                self.iter_protos.pop(prev)

                prev_type = self.parameter[prev].c_type
                if param.c_type == 'char':

                    def append_proto_string(list):
                        list.append(Parameter(None, \
                            c_type='const std::string &',
                            c_name=param.c_name))

                    def append_call_string(list):
                        list.append(Parameter(None, \
                            c_name="static_cast<" + prev_type + ">(" \
                            + param.c_name + '.length())'))

                        list.append(Parameter(None, \
                            c_name=param.c_name + '.c_str()'))

                    append_proto_string(self.wrap_protos)
                    append_proto_string(self.iter_protos)
                    append_call_string(self.wrap_calls)
                    append_call_string(self.iter_calls)
                    append_call_string(self.iter_2nd_lvl_calls)

                else:
                    param_type = param.c_type
                    if param_type == "void":
                        param_type = "Type_" + str(index)
                        self.templates.append(param_type)

                    prev_type = self.parameter[prev].c_type

                    ### std::vector
                    self.wrap_protos.append(Parameter(None, \
                        c_type='const std::vector<' + param_type + '> &',
                        c_name=param.c_name))

                    self.wrap_calls.append(Parameter(None, \
                      c_name="static_cast<" + prev_type + ">(" \
                      + param.c_name + '.size())'))

                    self.wrap_calls.append(Parameter(None, \
                        c_name=param.c_name + '.data()'))

                    ### Iterator
                    iter_type = param.c_name.capitalize() + "_Iterator"
                    iter_begin = param.c_name + "_begin"
                    iter_end = param.c_name + "_end"

                    self.iterator_templates.append(iter_type)

                    self.iter_protos.append(Parameter(None, \
                            c_type=iter_type,
                            c_name=iter_begin))

                    self.iter_protos.append(Parameter(None, \
                            c_type=iter_type,
                            c_name=iter_end))

                    self.iter_calls.append(Parameter(None, \
                            c_name="static_cast<" + prev_type + ">(" \
                            + param.c_name + '.size())'))

                    self.iter_calls.append(Parameter(None, \
                            c_name='const_cast<const ' + param_type + ' *>(' \
                            + param.c_name + '.data())'))

                    self.iter_2nd_lvl_calls.append(Parameter(None, \
                            c_name=iter_begin))

                    self.iter_2nd_lvl_calls.append(Parameter(None, \
                            c_name=iter_end))

                    self.initializer.append( \
                            "std::vector<%s> %s = { %s, %s };" \
                            % (param_type, param.c_name, iter_begin, iter_end))

            else:
                self.wrap_calls.append(param)
                self.wrap_protos.append(param)
                self.iter_calls.append(param)
                self.iter_2nd_lvl_calls.append(param)
                self.iter_protos.append(param)

    def wrapped_calls(self, sort):
        return self.calls(sort, params=self.wrap_calls)

    def wrapped_protos(self, sort, defaults):
        return self.protos(sort, defaults, params=self.wrap_protos)

    def iterator_calls(self, sort):
        return self.calls(sort, params=self.iter_calls)

    def iterator_2nd_lvl_calls(self, sort):
        return self.calls(sort, params=self.iter_2nd_lvl_calls)

    def iterator_protos(self, sort, defaults):
        return self.protos(sort, defaults, params=self.iter_protos)



_default_parameter_values = \
    { "xcb_timestamp_t" : "XCB_TIME_CURRENT_TIME" }

class Parameter(object):
    def __init__(self, field, c_type="", c_name="", verbose=False):
        self.field = field
        if field != None:
          self.c_type = field.c_field_type
          self.c_name = field.c_field_name
          self.is_const = field.c_field_const_type == "const " + field.c_field_type
          self.is_pointer = field.c_pointer != " "
          # self.serialize = field.type.need_serialize
          self.default = _default_parameter_values.get(self.c_type)
          self.with_default = True
          if verbose:
              sys.stderr.write("c_type: %s; c_name: %s; default: %s\n" \
                      % (self.c_type, self.c_name, self.default))

        else:
          self.c_type = c_type
          self.c_name = c_name
          self.is_const = False
          self.is_pointer = False
          # self.serialize = field.type.need_serialize
          self.default = _default_parameter_values.get(self.c_type)
          self.with_default = True

    def call(self):
        return self.c_name

    def proto(self, with_default):
        c_type = ("const " if self.is_const else "") \
             + self.c_type \
             + (" *" if self.is_pointer else "")
        param = " = " + self.default if with_default and self.default != None else ""
        return c_type + " " + self.c_name + param

########## PARAMETER ##########
