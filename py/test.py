#!/usr/bin/python
# coding: utf-8
""" A module quite full of test functions """

def test_function(quite_a_list):
    """ Just a little test function """
    return ["%s-sauce" % e for e in quite_a_list]

def test_dict_function(testd):
    """ Test function for dict -> map"""
    return dict([("%s sauce" % i, "%s boss" % testd[i]) for i in testd])

def test_unicode():
    return u'ünicode string'

def test_string():
    return 'a string'
