// (C) Copyright Jonathan Turkanis 2004
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt.)

// See http://www.boost.org/libs/iostreams for documentation.

#ifndef BOOST_IOSTREAMS_TEST_READ_BIDIRECTIONAL_FILTER_HPP_INCLUDED
#define BOOST_IOSTREAMS_TEST_READ_BIDIRECTIONAL_FILTER_HPP_INCLUDED

#include <fstream>
#include <boost/iostreams/combine.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/test/test_tools.hpp>
#include "detail/filters.hpp"
#include "detail/sequence.hpp"
#include "detail/temp_file.hpp"
#include "detail/verification.hpp"

void read_bidirectional_filter_test()
{
    using namespace std;
    using namespace boost;
    using namespace boost::iostreams;
    using namespace boost::iostreams::test;

    uppercase_file upper;

    //{
    //    test_file                        src;
    //    temp_file                        dest; // Dummy.
    //    filtering_stream<bidirectional>  first;
    //    first.push(combine(toupper_filter(), tolower_filter()));
    //    first.push(
    //        combine(file_source(src.name()), file_sink(dest.name()))
    //    );
    //    ifstream second(upper.name().c_str());
    //    BOOST_CHECK_MESSAGE(
    //        compare_streams_in_chars(first, second),
    //        "failed reading from filtering_stream<bidirectional> in chars with an "
    //        "input filter"
    //    );
    //}

    {
        test_file                        src;
        temp_file                        dest; // Dummy.
        filtering_stream<bidirectional>  first;
        first.push(combine(toupper_filter(), tolower_filter()));
        first.push(
            combine(file_source(src.name()), file_sink(dest.name()))
        );
        ifstream second(upper.name().c_str());
        BOOST_CHECK_MESSAGE(
            compare_streams_in_chunks(first, second),
            "failed reading from filtering_stream<bidirectional> in chunks with an "
            "input filter"
        );
    }

    //{
    //    test_file                        src;
    //    temp_file                        dest; // Dummy.
    //    filtering_stream<bidirectional>  first(
    //        combine(toupper_multichar_filter(), tolower_filter()), 0
    //    );
    //    first.push(
    //        combine(file_source(src.name()), file_sink(dest.name()))
    //    );
    //    ifstream second(upper.name().c_str());
    //    BOOST_CHECK_MESSAGE(
    //        compare_streams_in_chars(first, second),
    //        "failed reading from filtering_stream<bidirectional> in chars with "
    //        "a multichar input filter with no buffer"
    //    );
    //}

    //{
    //    test_file                        src;
    //    temp_file                        dest; // Dummy.
    //    filtering_stream<bidirectional>  first(
    //        combine(toupper_multichar_filter(), tolower_filter()), 0
    //    );
    //    first.push(
    //        combine(file_source(src.name()), file_sink(dest.name()))
    //    );
    //    ifstream second(upper.name().c_str());
    //    BOOST_CHECK_MESSAGE(
    //        compare_streams_in_chunks(first, second),
    //        "failed reading from filtering_stream<bidirectional> in chunks "
    //        "with a multichar input filter with no buffer"
    //    );
    //}

    //{
    //    test_file                        src;
    //    temp_file                        dest; // Dummy.
    //    filtering_stream<bidirectional>  first(
    //        combine(toupper_multichar_filter(), tolower_filter())
    //    );
    //    first.push(
    //        combine(file_source(src.name()), file_sink(dest.name()))
    //    );
    //    ifstream second(upper.name().c_str());
    //    BOOST_CHECK_MESSAGE(
    //        compare_streams_in_chars(first, second),
    //        "failed reading from filtering_stream<bidirectional> in chars with a "
    //        "multichar input filter"
    //    );
    //}

    //{
    //    test_file                        src;
    //    temp_file                        dest; // Dummy.
    //    filtering_stream<bidirectional>  first(
    //        combine(toupper_multichar_filter(), tolower_filter())
    //    );
    //    first.push(
    //        combine(file_source(src.name()), file_sink(dest.name()))
    //    );
    //    ifstream second(upper.name().c_str());
    //    BOOST_CHECK_MESSAGE(
    //        compare_streams_in_chunks(first, second),
    //        "failed reading from filtering_stream<bidirectional> in chunks "
    //        "with a multichar input filter"
    //    );
    //}
}

#endif // #ifndef BOOST_IOSTREAMS_TEST_READ_BIDIRECTIONAL_FILTER_HPP_INCLUDED
