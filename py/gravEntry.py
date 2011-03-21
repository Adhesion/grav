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

