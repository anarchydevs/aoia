/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// test_non_default_ctor2.cpp

// (C) Copyright 2002 Martin Ecker. 
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// should pass compilation and execution

// this tests complex usage of non-default constructor.  Specifically
// the case where a constructor serializes a pointer member.

#include <fstream>
#include <cstdlib> // for remove()

#include <boost/config.hpp>
#if defined(BOOST_NO_STDC_NAMESPACE)
namespace std{
    using ::remove;
}
#endif

#include "test_tools.hpp"
#include <boost/preprocessor/stringize.hpp>
#include BOOST_PP_STRINGIZE(BOOST_ARCHIVE_TEST)

class IntValueHolder
{
public:
    IntValueHolder()
        : value(0)
    {}

    IntValueHolder(int newvalue)
        : value(newvalue)
    {}

    int GetValue() const { return value; }

private:
    int value;

    friend class boost::serialization::access;

    template <class ArchiveT>
    void serialize(ArchiveT& ar, const unsigned int /* file_version */)
    {
        ar & BOOST_SERIALIZATION_NVP(value);
    }
};

class FloatValueHolder
{
public:
    FloatValueHolder()
        : value(0)
    {}

    FloatValueHolder(float newvalue)
        : value(newvalue)
    {}

    float GetValue() const { return value; }

private:
    float value;

    friend class boost::serialization::access;

    template <class ArchiveT>
    void serialize(ArchiveT& ar, const unsigned int /* file_version */)
    {
        ar & BOOST_SERIALIZATION_NVP(value);
    }
};

class A
{
public:
    A(const IntValueHolder& initialValue)
        : value(initialValue), floatValue(new FloatValueHolder(10.0f))
    {}

    ~A()
    {
        delete floatValue;
    }

    IntValueHolder value;
    FloatValueHolder* floatValue;

private:
    friend class boost::serialization::access;

    template <class ArchiveT>
    void serialize(ArchiveT& ar, const unsigned int /* file_version */)
    {
        ar & BOOST_SERIALIZATION_NVP(floatValue);
    }
};

namespace boost { 
namespace serialization {

template <class ArchiveT>
void save_construct_data(ArchiveT& archive, const A* p, unsigned int version)
{
    archive & boost::serialization::make_nvp("initialValue", p->value);
}

template <class ArchiveT>
void load_construct_data(ArchiveT& archive, A* p, unsigned int version)
{
    IntValueHolder initialValue;
    archive & boost::serialization::make_nvp("initialValue", initialValue);

    ::new (p) A(initialValue);
}

} // serialization
} // namespace boost

int test_main( int /* argc */, char* /* argv */[] )
{
    const char * testfile = boost::archive::tmpnam(NULL);
    BOOST_REQUIRE(NULL != testfile);
    A* a = new A(5);

    {   
        test_ostream os(testfile, TEST_STREAM_FLAGS);
        test_oarchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(a);
    }

    A* a_new;

    {
        test_istream is(testfile, TEST_STREAM_FLAGS);
        test_iarchive ia(is);
        ia >> BOOST_SERIALIZATION_NVP(a_new);
    }
    delete a;
    delete a_new;
    return EXIT_SUCCESS;
}
