//  minmax_check header  -------------------------------------------------------//

//  Copyright Beman Dawes 2002
//  Copyright Rene Rivera 2004.
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_MINMAX_CHECK_HPP
#define BOOST_MINMAX_CHECK_HPP

#include "inspector.hpp"

namespace boost
{
  namespace inspect
  {
    class minmax_check : public inspector
    {
      long m_errors;

    public:

      minmax_check();
      virtual const char * name() const { return "minmax-check"; }
      virtual const char * desc() const { return "uses of min and max that have not been protected from the min/max macros"; }

      virtual void inspect(
        const std::string & library_name,
        const path & full_path,
        const std::string & contents);

      virtual ~minmax_check()
      {
        std::cout << "  " << m_errors << " violations of the Boost min/max guidelines\n";
      }
    };
  }
}

#endif // BOOST_MINMAX_CHECK_HPP
