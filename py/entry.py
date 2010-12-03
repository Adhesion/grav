def entryFunc( module, func, *args, **kwargs ):
    import sys, os, gc
    #import pprint
    if os.path.sep not in module:
        # case for built-in modules - look in py dir
        # TODO make this a built-in thing, from build system rather than cwd
        path = os.getcwd() + os.path.sep + 'py'
        filename = module
    else:
        # case for user-supplied module, should have full pathname
        toks = module.split(os.path.sep)
        # get path minus filename, reassemble with system path separator
        path = os.path.sep.join(toks[:-1])
        filename = toks[-1]
    try:
        # TODO this will break on files with more than 1 '.' - support or not?
        # actually it looks like python doesn't even support that so nevermind
        finalModuleName = filename.split('.')[0]
        sys.path.insert(0, path)
        m = __import__(finalModuleName, globals(), locals())
        f = getattr(m, func)
        #pprint.pprint( args )
        print "entryFunc calling", func
        print "GC STUFF:", len(gc.get_objects())
        res = f(*args, **kwargs)
        print "entryFunc call to", func, "is done."
        # remove added path from sys path
        sys.path.remove(path)
        return res
    except:
        import traceback
        traceback.print_exc()

