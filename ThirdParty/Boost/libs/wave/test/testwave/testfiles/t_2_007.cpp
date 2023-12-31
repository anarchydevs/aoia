/*=============================================================================
    Boost.Wave: A Standard compliant C++ preprocessor library
    http://www.boost.org/

    Copyright (c) 2001-2005 Hartmut Kaiser. Distributed under the Boost
    Software License, Version 1.0. (See accompanying file
    LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

//O --c99

// Tests correctness of macro expansion inside #pragma directives
// Note: in C99 mode the STDC prefix in pragmas supresses macro expansion

#define PRAGMA_BODY preprocessed pragma body

//R #line 19 "t_2_007.cpp"
//R #pragma some pragma body
#pragma some pragma body
//R #line 22 "t_2_007.cpp"
//R #pragma preprocessed pragma body
#pragma PRAGMA_BODY
//R #line 25 "t_2_007.cpp"
//R #pragma STDC some C99 standard pragma body
#pragma STDC some C99 standard pragma body
//R #line 28 "t_2_007.cpp"
//R #pragma STDC PRAGMA_BODY
#pragma STDC PRAGMA_BODY
