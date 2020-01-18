#
# multiple-argument dispatch (not only on "this" argument and virtual functions as in C++)
# @author Tobias Weber
# @date 18-jan-20
# @license: see 'LICENSE.EUPL' file
# @see https://en.wikipedia.org/wiki/Visitor_pattern
#


# -----------------------------------------------------------------------------
# types
abstract type t_base end


mutable struct t_derived1 <: t_base
	i::Int;
end


mutable struct t_derived2 <: t_base
	f::Real;
end
# -----------------------------------------------------------------------------




# -----------------------------------------------------------------------------
# test 1
function dispatch_tst(x::t_base, y::t_base)
	println("dispatch: base, base");
end

function dispatch_tst(x::t_derived1, y::t_base)
	println("dispatch: derived1, base");
end

function dispatch_tst(x::t_derived1, y::t_derived1)
	println("dispatch: derived1, derived1");
end

function dispatch_tst(x::t_derived1, y::t_derived2)
	println("dispatch: derived1, derived2");
end

function dispatch_tst(x::t_derived2, y::t_derived2)
	println("dispatch: derived2, derived2");
end



function tst1()
	println("Test 1");

	d1::t_base = t_derived1(123);
	d2::t_base = t_derived2(234);
	#d2.f = d1.i*2;

	println("typeof(d1) = ", typeof(d1));
	println("typeof(d2) = ", typeof(d2));

	dispatch_tst(d1, d2);
	dispatch_tst(d2, d1);
	dispatch_tst(d1, d1);
	dispatch_tst(d2, d2);

	print("\n\n");
end
# -----------------------------------------------------------------------------




# -----------------------------------------------------------------------------
# test 2
function dispatch_tst2(x::t_base, y::t_base)
	println("dispatch: base, base");

	dispatch_tst2(y, x);
end

function dispatch_tst2(x::t_derived1, y::t_derived2)
	println("dispatch: derived1, base");
end



function tst2()
	println("Test 2");

	d1::t_base = t_derived1(123);
	d2::t_base = t_derived2(234);

	dispatch_tst2(d1, d2);
	dispatch_tst2(d2, d1);

	print("\n\n");
end
# -----------------------------------------------------------------------------




# -----------------------------------------------------------------------------
# test 3
function dispatch_tst3(x, y)
	println("dispatch: notype, notype");
end

function dispatch_tst3(x::t_derived1, y::t_derived2)
	println("dispatch: derived1, derived2");
end

function tst3()
	println("Test 3");

	d1::t_base = t_derived1(123);
	d2::t_base = t_derived2(234);

	dispatch_tst3(d1, d2);
	dispatch_tst3(d2, d1);

	print("\n\n");
end
# -----------------------------------------------------------------------------




tst1()
tst2()
tst3()
