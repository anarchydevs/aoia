/*=============================================================================
    Boost.Wave: A Standard compliant C++ preprocessor library
    http://www.boost.org/

    Copyright (c) 2001-2005 Hartmut Kaiser. Distributed under the Boost
    Software License, Version 1.0. (See accompanying file
    LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

// Tests, if a diagnostic is emitted, if a predefined macro is to be undefined.

//R
//E t_3_001.cpp(14): warning: #undef may not be used on this predefined name: __cplusplus
#undef __cplusplus     // should emit a warning
