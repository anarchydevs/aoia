// Copyright David Abrahams 2002.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <boost/python/object/enum_base.hpp>
#include <boost/python/cast.hpp>
#include <boost/python/scope.hpp>
#include <boost/python/object.hpp>
#include <boost/python/tuple.hpp>
#include <boost/python/dict.hpp>
#include <boost/python/str.hpp>
#include <boost/python/extract.hpp>
#include <boost/python/object_protocol.hpp>
#include <boost/python/detail/api_placeholder.hpp>
#include <structmember.h>
#include <cstdio>

namespace boost { namespace python { namespace objects { 

struct enum_object
{
    PyIntObject base_object;
    PyObject* name;
};

static PyMemberDef enum_members[] = {
    {"name", T_OBJECT_EX, offsetof(enum_object,name),READONLY},
    {0}
};


extern "C"
{
    static int
    enum_print(PyObject *v, BOOST_CSTD_::FILE *fp, int flags)
    {
        PyObject* s
            = (flags & Py_PRINT_RAW) ? v->ob_type->tp_str(v) : v->ob_type->tp_repr(v);
        if (s == 0)
            return -1;
        
        char const* text = PyString_AsString(s);
        if (text == 0)
            return -1;
        
        BOOST_CSTD_::fprintf(fp, text);
        return 0;
    }
    
     /* flags -- not used but required by interface */
    static PyObject* enum_repr(PyObject* self_)
    {
        enum_object* self = downcast<enum_object>(self_);
        if (!self->name)
        {
            return PyString_FromFormat("%s(%ld)", self_->ob_type->tp_name, PyInt_AS_LONG(self_));
        }
        else
        {
            char* name = PyString_AsString(self->name);
            if (name == 0)
                return 0;
            
            return PyString_FromFormat("%s.%s", self_->ob_type->tp_name, name);
        }
    }

    static PyObject* enum_str(PyObject* self_)
    {
        enum_object* self = downcast<enum_object>(self_);
        if (!self->name)
        {
            return PyInt_Type.tp_str(self_);
        }
        else
        {
            return incref(self->name);
        }
    }
}

static PyTypeObject enum_type_object = {
    PyObject_HEAD_INIT(0) // &PyType_Type
    0,
    "Boost.Python.enum",
    sizeof(enum_object),                    /* tp_basicsize */
    0,                                      /* tp_itemsize */
    0,                                      /* tp_dealloc */
    enum_print,                             /* tp_print */
    0,                                      /* tp_getattr */
    0,                                      /* tp_setattr */
    0,                                      /* tp_compare */
    enum_repr,                              /* tp_repr */
    0,                                      /* tp_as_number */
    0,                                      /* tp_as_sequence */
    0,                                      /* tp_as_mapping */
    0,                                      /* tp_hash */
    0,                                      /* tp_call */
    enum_str,                               /* tp_str */
    0,                                      /* tp_getattro */
    0,                                      /* tp_setattro */
    0,                                      /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT
    | Py_TPFLAGS_CHECKTYPES
    | Py_TPFLAGS_HAVE_GC
    | Py_TPFLAGS_BASETYPE,                  /* tp_flags */
    0,                                      /* tp_doc */
    0,                                      /* tp_traverse */
    0,                                      /* tp_clear */
    0,                                      /* tp_richcompare */
    0,                                      /* tp_weaklistoffset */
    0,                                      /* tp_iter */
    0,                                      /* tp_iternext */
    0,                                      /* tp_methods */
    enum_members,                           /* tp_members */
    0,                                      /* tp_getset */
    0, //&PyInt_Type,                       /* tp_base */
    0,                                      /* tp_dict */
    0,                                      /* tp_descr_get */
    0,                                      /* tp_descr_set */
    0,                                      /* tp_dictoffset */
    0,                                      /* tp_init */
    0,                                      /* tp_alloc */
    0                                       /* tp_new */
};

object module_prefix();

namespace
{
  object new_enum_type(char const* name)
  {
      if (enum_type_object.tp_dict == 0)
      {
          enum_type_object.ob_type = incref(&PyType_Type);
          enum_type_object.tp_base = &PyInt_Type;
          if (PyType_Ready(&enum_type_object))
              throw_error_already_set();
      }

      type_handle metatype(borrowed(&PyType_Type));
      type_handle base(borrowed(&enum_type_object));

      // suppress the instance __dict__ in these enum objects. There
      // may be a slicker way, but this'll do for now.
      dict d;
      d["__slots__"] = tuple();
      d["values"] = dict();

      object module_name = module_prefix();
      if (module_name)
          module_name += '.';
      
      object result = (object(metatype))(
          module_name + name, make_tuple(base), d);
      
      scope().attr(name) = result;

      return result;
  }
}

enum_base::enum_base(
    char const* name
    , converter::to_python_function_t to_python
    , converter::convertible_function convertible
    , converter::constructor_function construct
    , type_info id
    )
    : object(new_enum_type(name))
{
    converter::registration& converters
        = const_cast<converter::registration&>(
            converter::registry::lookup(id));
            
    converters.m_class_object = downcast<PyTypeObject>(this->ptr());
    converter::registry::insert(to_python, id);
    converter::registry::insert(convertible, construct, id);
}

void enum_base::add_value(char const* name_, long value)
{
    // Convert name to Python string
    object name(name_);

    // Create a new enum instance by calling the class with a value
    object x = (*this)(value);

    // Store the object in the enum class
    (*this).attr(name_) = x;

    dict d = extract<dict>(this->attr("values"))();
    d[value] = x;
    
    // Set the name field in the new enum instanec
    enum_object* p = downcast<enum_object>(x.ptr());
    Py_XDECREF(p->name);
    p->name = incref(name.ptr());
}

void enum_base::export_values()
{
    dict d = extract<dict>(this->attr("values"))();
    list values = d.values();
    scope current;
    
    for (unsigned i = 0, max = len(values); i < max; ++i)
    {
        api::setattr(current, object(values[i].attr("name")), values[i]);
    }
 }

PyObject* enum_base::to_python(PyTypeObject* type_, long x)
{
    object type((type_handle(borrowed(type_))));

    dict d = extract<dict>(type.attr("values"))();
    object v = d.get(x, object());
    return incref(
        (v == object() ? type(x) : v).ptr());
}

}}} // namespace boost::python::object
