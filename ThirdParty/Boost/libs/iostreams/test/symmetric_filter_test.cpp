// (C) Copyright Jonathan Turkanis 2004
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt.)

// See http://www.boost.org/libs/iostreams for documentation.

#include <boost/iostreams/detail/buffer.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filter/symmetric.hpp>
#include <boost/iostreams/filter/test.hpp>
#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>
#include "./detail/constants.hpp"
#include "./detail/temp_file.hpp"
#include "./detail/verification.hpp"

// Must come last.
#include <boost/iostreams/detail/config/disable_warnings.hpp> 

using namespace boost::iostreams;
using namespace boost::iostreams::test;
using boost::unit_test::test_suite;   

// Note: The filter is given an internal buffer -- unnecessary in this simple
// case -- to stress test symmetric_filter.
struct toupper_symmetric_filter_impl {
    typedef char char_type;
    explicit toupper_symmetric_filter_impl( 
                std::streamsize buffer_size = 
                    default_filter_buffer_size 
             ) 
        : buf_(default_filter_buffer_size) 
    {
        buf_.set(0, 0);
    }
    bool filter( const char*& src_begin, const char* src_end,
                 char*& dest_begin, char* dest_end, bool /* flush */ )
    {
        while ( can_read(src_begin, src_end) || 
                can_write(dest_begin, dest_end) ) 
        {
            if (can_read(src_begin, src_end)) 
                read(src_begin, src_end);
            if (can_write(dest_begin, dest_end)) 
                write(dest_begin, dest_end);
        }
        bool result = buf_.ptr() != buf_.eptr();
        return result;
    }
    void close() { buf_.set(0, 0); }
    void read(const char*& src_begin, const char* src_end)
    {
        std::ptrdiff_t count =
            (std::min) ( src_end - src_begin,
                         static_cast<std::ptrdiff_t>(buf_.size()) - 
                             (buf_.eptr() - buf_.data()) );
        while (count-- > 0)
            *buf_.eptr()++ = std::toupper(*src_begin++);
    }
    void write(char*& dest_begin, char* dest_end)
    {
        std::ptrdiff_t count =
            (std::min) ( dest_end - dest_begin,
                         buf_.eptr() - buf_.ptr() );
        while (count-- > 0)
            *dest_begin++ = *buf_.ptr()++;
        if (buf_.ptr() == buf_.eptr())
            buf_.set(0, 0);
    }
    bool can_read(const char*& src_begin, const char* src_end)
    { return src_begin != src_end && buf_.eptr() != buf_.end(); }
    bool can_write(char*& dest_begin, char* dest_end)
    { return dest_begin != dest_end && buf_.ptr() != buf_.eptr(); }
    boost::iostreams::detail::buffer<char> buf_;
};

typedef symmetric_filter<toupper_symmetric_filter_impl>
        toupper_symmetric_filter;

void read_symmetric_filter_test()
{
    test_file       test;
    uppercase_file  upper;
    BOOST_CHECK(
        test_input_filter( toupper_symmetric_filter(default_filter_buffer_size),
                           file_source(test.name(), in_mode),
                           file_source(upper.name(), in_mode) )
    );
} 

void write_symmetric_filter_test()
{
    test_file       test;
    uppercase_file  upper;
    BOOST_CHECK(
        test_output_filter( toupper_symmetric_filter(default_filter_buffer_size),
                            file_source(test.name(), in_mode),
                            file_source(upper.name(), in_mode) )
    );
}

test_suite* init_unit_test_suite(int, char* []) 
{
    test_suite* test = BOOST_TEST_SUITE("symmetric_filter test");
    test->add(BOOST_TEST_CASE(&read_symmetric_filter_test));
    test->add(BOOST_TEST_CASE(&write_symmetric_filter_test));
    return test;
}

#include <boost/iostreams/detail/config/enable_warnings.hpp>
