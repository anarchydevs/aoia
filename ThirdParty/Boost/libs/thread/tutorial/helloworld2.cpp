// Copyright (C) 2001-2003
// William E. Kempf
//
// Permission to use, copy, modify, distribute and sell this software
// and its documentation for any purpose is hereby granted without fee,
// provided that the above copyright notice appear in all copies and
// that both that copyright notice and this permission notice appear
// in supporting documentation.  William E. Kempf makes no representations
// about the suitability of this software for any purpose.
// It is provided "as is" without express or implied warranty.

#include <boost/thread/thread.hpp>
#include <iostream>

struct helloworld
{
    helloworld(const char* who) : m_who(who) { }
    void operator()()
    {
        std::cout << m_who << "says, \"Hello World.\"" << std::endl;
    }
    const char* m_who;
};

int main()
{
    boost::thread thrd(helloworld("Bob"));
    thrd.join();
}
