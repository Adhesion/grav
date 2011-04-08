# @file AGTools.py
#
# Created November 2, 2010
# @author Andrew Ford
# @author Ralph Bean
# Copyright (C) 2011 Rochester Institute of Technology
#
# This file is part of grav.
#
# grav is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# grav is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with grav.  If not, see <http://www.gnu.org/licenses/>.

"""
Entry point for grav python integration. Calls given function from given module.
"""

def entryFunc( module, func, *args, **kwargs ):
    import sys, os, gc
    if os.path.sep not in module:
        # case for looking in working dir/py
        # old style - probably should be deprecated, all usage should be full
        # paths gotten via gravUtil
        path = os.getcwd() + os.path.sep + 'py'
        filename = module
    else:
        # case for full pathname, should be standard now via gravUtil
        toks = module.split(os.path.sep)
        # get path minus filename, reassemble with system path separator
        path = os.path.sep.join(toks[:-1])
        filename = toks[-1]
    try:
        # this will break on files with more than 1 '.', but that isn't even
        # supported by python itself so we're not caring about it here
        finalModuleName = filename.split('.')[0]
        sys.path.insert(0, path)
        m = __import__(finalModuleName, globals(), locals())
        f = getattr(m, func)
        res = f(*args, **kwargs)
        # remove added path from sys path
        sys.path.remove(path)
        return res
    except:
        import traceback
        traceback.print_exc()

