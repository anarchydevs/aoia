/*=============================================================================
    Copyright (c) 2002-2003 Joel de Guzman
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

// *** See the section "Look Ma' No Rules" in
// *** chapter "Techniques" of the Spirit documentation
// *** for information regarding this snippet

#include <iostream>
#include <boost/spirit/core.hpp>

using namespace boost::spirit;

struct skip_grammar : grammar<skip_grammar>
{
    template <typename ScannerT>
    struct definition
    {
        definition(skip_grammar const& /*self*/)
        : skip
            (       space_p
                |   "//" >> *(anychar_p - '\n') >> '\n'
                |   "/*" >> *(anychar_p - "*/") >> "*/"
            )
        {
        }

        typedef
           alternative<alternative<space_parser, sequence<sequence<
           strlit<const char*>, kleene_star<difference<anychar_parser,
           chlit<char> > > >, chlit<char> > >, sequence<sequence<
           strlit<const char*>, kleene_star<difference<anychar_parser,
           strlit<const char*> > > >, strlit<const char*> > >
        skip_t;
        skip_t skip;

        skip_t const&
        start() const { return skip; }
    };
};

int
main()
{
    skip_grammar g;
    bool success = parse(
        "/*this is a comment*/\n//this is a c++ comment\n\n",
        *g).full;
    assert(success);
    std::cout << "SUCCESS!!!\n";
    return 0;
}
