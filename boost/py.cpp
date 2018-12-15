/**
 * py interpreter invocation tests and snippets
 * @author Tobias Weber
 * @date 15-dec-18
 * @license: see 'LICENSE.EUPL' file
 *
 * py3: g++ -std=c++17 -I/usr/include/python3.7m -o py py.cpp -lpython3.7m -lboost_python3 -lboost_numpy3
 * py2: g++ -std=c++17 -I/usr/include/python2.7 -o py py.cpp -lpython2.7 -lboost_python -lboost_numpy
 *
 * References:
 *  * https://www.boost.org/doc/libs/1_69_0/libs/python/doc/html/index.html
 *  * https://github.com/boostorg/python/tree/develop/example
 */

#include <iostream>

#include <boost/python.hpp>
#include <boost/python/numpy.hpp>
namespace py = boost::python;
namespace np = boost::python::numpy;


/**
 * py::extract wrapper with implicit type cast
 */
template<typename ty, class t_obj>
ty extract_cast(const t_obj& obj)
{
	return /*static_cast<ty>(*/py::extract<ty>(obj)/*)*/;
}


/**
 * imports a python module and gets its dictionary
 */
auto import_module = [](const char* modulename) -> auto
{
	auto pymod = py::import(modulename);
	auto pydict = extract_cast<py::dict>(pymod.attr("__dict__"));

	return std::make_tuple(pymod, pydict);
};


/**
 * gets python interpreter version
 */
template<class t_obj>
auto get_py_version(const t_obj& pysys)
{
	auto verinfo = pysys.attr("version_info");
	int major = extract_cast<int>(verinfo.attr("major"));
	int minor = extract_cast<int>(verinfo.attr("minor"));
	int micro = extract_cast<int>(verinfo.attr("micro"));

	std::ostringstream ostrver;
	ostrver << major << "." << minor << "." << micro;

	return std::make_tuple(major, minor, micro, ostrver.str());
}


/**
 * returns the type of a py obj
 */
template<class t_obj>
std::string get_pyobj_type(const t_obj& obj)
{
	return extract_cast<std::string>(
		obj.attr("__class__").attr("__name__"));
}


/**
 * converts a py obj to a (representation) string
 */
template<class t_obj>
std::string get_pyobj_repr(const t_obj& obj, bool bRepr=true)
{
	py::str reprfunc = bRepr ? "__repr__" : "__str__";
	return extract_cast<std::string>(obj.attr(reprfunc)());
}


/**
 * gets description of current python exception
 */
auto get_py_err()
{
	if(!::PyErr_Occurred())
		return std::make_tuple(std::string(), std::string());

	::PyObject *errTypeObj=nullptr, *errValObj=nullptr, *errStackTraceObj=nullptr;
	::PyErr_Fetch(&errTypeObj, &errValObj, &errStackTraceObj);

	auto errval = extract_cast<std::string>(::PyObject_Str(errValObj));
	auto errtype = extract_cast<std::string>(::PyObject_Repr(errTypeObj));

	return std::make_tuple(errval, errtype);
}


/**
 * gets a representation of the items in a list
 */
template<class t_list>
std::string print_list(const t_list& list)
{
	std::ostringstream ostr;

	// variant 1
	/*for(std::size_t i=0; i<py::len(list); ++i)
	{
		ostr << get_pyobj_repr(list[i], false);
		if(i < py::len(list)-1)
			ostr << "; ";
	}*/

	// variant 2
	for(py::stl_input_iterator<py::object> iter(list);
		iter != py::stl_input_iterator<py::object>();
		++iter)
	{
		ostr << get_pyobj_repr(*iter, false) << "; ";
	}

	return ostr.str();
}


/**
 * gets a representation of the items in a dict
 */
template<class t_dict>
std::string print_dict(const t_dict& dict, bool bOnlyKeys=true)
{
	std::ostringstream ostr;

	if(bOnlyKeys)
		ostr << print_list(dict.keys());
	else	// dict.items() returns a list of [key, value] pairs
		ostr << print_list(dict.items());

	return ostr.str();
}


int main()
{
	try
	{
		// init
		::Py_Initialize();
		np::initialize();


		// append current working dir
		auto [pysys, pysys_dict] = import_module("sys");
		std::cout << "Sys dict: " << print_dict(pysys_dict) << std::endl;
		extract_cast<py::list>(pysys_dict["path"]).insert(0, "./");
		std::cout << "Py paths: " << print_list(pysys_dict["path"]) << std::endl;


		// interpreter version
		auto pyver = get_py_version(pysys);
		std::cerr << "Py version: " << std::get<3>(pyver) << "." << std::endl;


		// import script
		auto [pyscr, pyscr_dict] = import_module("tstscr");
		std::cout << "Script dict: " << print_dict(pyscr_dict) << std::endl;


		// function 1
		{
			auto func = pyscr_dict["tstfunc_noparams"];
			func();
		}

		// function 2
		{
			auto func = pyscr_dict["tstfunc"];
			func("123");
			func(123);
			func(py::make_tuple("abc", 123));
			func(np::array(py::make_tuple(1,2,3)));
		}

		// function 3
		{
			auto func = pyscr_dict["tstfunc_ret"];

			auto ret = func(1, 2);
			std::cout << "ret = " << get_pyobj_repr(ret, false)
				<< " (type: " << get_pyobj_type(ret) << ")"
				<< std::endl;;

			auto ret2 = func("abc", "123");
			std::cout << "ret = " << get_pyobj_repr(ret2, false)
				<< " (type: " << get_pyobj_type(ret2) << ")"
				<< std::endl;;

			auto ret3 = func(np::array(py::make_tuple(1,2,3)), np::array(py::make_tuple(9,8,7)));
			std::cout << "ret = " << get_pyobj_repr(ret3, false)
				<< " (type: " << get_pyobj_type(ret3) << ")"
				<< std::endl;;
		}

	}
	catch(const py::error_already_set&)
	{
		auto [errval, errty] = get_py_err();
		std::cerr << "Python error: " << errval << " " << errty << std::endl;
		return -1;
	}

	return 0;
}
