//  (C) Copyright Gennadiy Rozental 2001-2005.
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at 
//  http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org/libs/test for the library home page.

// Boost.Runtime.Param
#include <boost/test/utils/runtime/env/variable.hpp>
#include <boost/test/utils/basic_cstring/io.hpp>

namespace rt  = boost::runtime;
namespace env = boost::runtime::env;

// STL
#include <iostream>

int main() {
    env::variable<int> TEMP( "abc", (env::global_id = "temp_dir_location", env::default_value = 5) );

    std::cout << TEMP << std::endl;

    return 0;
}

// EOF

