from utils import \
        get_namespace, \
        get_ext_name, \
        _n_item, \
        _ext

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
namespace dispatcher { namespace event { class %s; }; };
namespace dispatcher { namespace error { class %s; }; };

namespace extension {

class %s%s{
  public:
    typedef xpp::protocol::%s protocol;
    typedef xpp::dispatcher::event::%s event_dispatcher;
    typedef xpp::dispatcher::error::%s error_dispatcher;
};

}; // namespace extension
''' % (ns, # namespace protocol { class %s; };
       ns, # namespace event { namespace dispatcher { class %s; }; };
       ns, # namespace error { namespace dispatcher { class %s; }; };
       ns, # class %s
       base,
       # ns, # : public xpp::extension::generic<&xcb_%s_id>
       ns, # typedef xpp::protocol::%s protocol;
       ns, # typedef xpp::event::dispatcher::%s dispatcher;
       ns) # typedef xpp::error::dispatcher::%s dispatcher;
