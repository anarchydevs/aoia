/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// binary_iarchive.cpp:

// (C) Copyright 2002 Robert Ramey - http://www.rrsd.com . 
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for updates, documentation, and revision history.

#include <istream>

#define BOOST_ARCHIVE_SOURCE
#include <boost/archive/binary_iarchive.hpp>

#include <boost/archive/impl/basic_binary_iprimitive.ipp>
#include <boost/archive/impl/basic_binary_iarchive.ipp>
#include <boost/archive/impl/archive_pointer_iserializer.ipp>

namespace boost {
namespace archive {

// explicitly instantiate for this type of stream
template class basic_binary_iprimitive<binary_iarchive, std::istream> ;
template class basic_binary_iarchive<binary_iarchive> ;
template class binary_iarchive_impl<binary_iarchive> ;
template class detail::archive_pointer_iserializer<binary_iarchive> ;

} // namespace archive
} // namespace boost
