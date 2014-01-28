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

from utils import \
        get_namespace, \
        get_ext_name, \
        _n_item, \
        _ext

from parameter import *
from resource_classes import _resource_classes

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
