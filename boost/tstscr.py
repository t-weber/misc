#
# py interpreter invocation tests and snippets
# @author Tobias Weber
# @date 15-dec-18
# @license: see 'LICENSE.EUPL' file
#


def tstfunc_noparams():
	print("In tstfunc_noparams().")


def tstfunc(a):
	params = "param type: %s, value: %s" % (type(a).__name__, repr(a))
	print("In tstfunc(); %s." % params)


def tstfunc_ret(a, b):
	return a + b


print("Loaded %s." % __name__)
