/*=============================================================================
    Boost.Wave: A Standard compliant C++ preprocessor library
    http://www.boost.org/

    Copyright (c) 2001-2005 Hartmut Kaiser. Distributed under the Boost
    Software License, Version 1.0. (See accompanying file
    LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

    The tests included in this file were initially taken from the mcpp V2.5
    preprocessor validation suite and were modified to fit into the Boost.Wave 
    unit test requirements.
    The original files of the mcpp preprocessor are distributed under the 
    license reproduced at the end of this file.
=============================================================================*/

// Tests operator "defined" in #if or #elif directives.

#define MACRO_abc   abc
#define MACRO_0     2
#define ZERO_TOKEN

// 11.1:
//R #line 28 "t_5_013.cpp"
//R true
#if defined a
false
#else
true
#endif

//R #line 34 "t_5_013.cpp"
//R true
#if defined (MACRO_abc)
true
#else
false
#endif

// 11.2:  "defined" is an unary operator whose result is 1 or 0.

//R #line 46 "t_5_013.cpp"
//R true
#if defined MACRO_0 * 3 != 3
false
#else
true
#endif

//R #line 54 "t_5_013.cpp"
//R true
#if (!defined ZERO_TOKEN != 0) || (-defined ZERO_TOKEN != -1)
false
#else
true
#endif

/*-
 * Copyright (c) 1998, 2002-2005 Kiyoshi Matsui <kmatsui@t3.rim.or.jp>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

