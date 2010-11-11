#!/usr/bin/python
# coding: utf-8
""" A module quite full of test functions """

def closure(f, *args, **kwargs):
    try:
        res = f(*args, **kwargs)
        def handle_dict(d):
            return d
        def handle_list(l):
            return [v + '-bob' for v in l]
        def handle_str(s):
            return s + '-is-a-string'
        
        lookup = {
            list : handle_list,
            dict : handle_dict,
        }
        return lookup.get(type(res), handle_str)(res)
        lookup[type(res)](res)
    except:
        import traceback
        traceback.print_exc()
        return "not null"

def handler(func):
    return lambda *args, **kwargs : closure(func, *args, **kwargs)

@handler
def test_function(quite_a_list):
    """ Just a little test function """
    return ["%s-sauce" % e for e in quite_a_list]

@handler
def test_dict_function(testd):
    """ Test function for dict -> map"""
    return dict([("%s sauce" % i, "%s boss" % testd[i]) for i in testd])

#@handler
#def test_unicode():
#   return u'ünicode string'

@handler
def test_string():
    return 'a string'

@handler
def test_exception():
    raise NotImplementedError, "not a chance"

