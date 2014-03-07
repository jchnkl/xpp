import re # compile

_reserved_keywords = {'class' : '_class',
                      'new'   : '_new',
                      'delete': '_delete',
                      'default' : '_default',
                      'private' : '_private',
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

def _n(list, namespace):
    '''
    Does C-name conversion on a tuple of strings.
    Different behavior depending on length of tuple, extension/not extension, etc.
    Basically C-name converts the individual pieces, then joins with underscores.
    '''
    if len(list) == 1:
        parts = list
    elif len(list) == 2:
        parts = [list[0], _n_item(list[1])]
    elif namespace.is_ext:
        parts = [list[0], _ext(list[1])] + [_n_item(i) for i in list[2:]]
    else:
        parts = [list[0]] + [_n_item(i) for i in list[1:]]
    return '_'.join(parts).lower()
