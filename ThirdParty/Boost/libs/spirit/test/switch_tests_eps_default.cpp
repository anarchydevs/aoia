/*=============================================================================
    Copyright (c) 2003 Hartmut Kaiser
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#include <iostream>
#include <cassert>

using namespace std;

#define BOOST_SPIRIT_SWITCH_CASE_LIMIT 6
#define BOOST_SPIRIT_SELECT_LIMIT 6
#define PHOENIX_LIMIT 6

//#define BOOST_SPIRIT_DEBUG
#include <boost/mpl/list.hpp>
#include <boost/mpl/for_each.hpp>

#include <boost/spirit/core/primitives/primitives.hpp>
#include <boost/spirit/core/primitives/numerics.hpp>
#include <boost/spirit/core/composite/actions.hpp>
#include <boost/spirit/core/composite/operators.hpp>
#include <boost/spirit/core/non_terminal/rule.hpp>
#include <boost/spirit/core/non_terminal/grammar.hpp>
#include <boost/spirit/dynamic/switch.hpp>
#include <boost/spirit/dynamic/select.hpp>
#include <boost/spirit/attribute/closure.hpp>

using namespace boost::spirit;

namespace test_grammars {

///////////////////////////////////////////////////////////////////////////////
//  Test the direct switch_p usage (with default_p)
    struct switch_grammar_direct_default4 
    :   public grammar<switch_grammar_direct_default4>
    {
        template <typename ScannerT>
        struct definition 
        {
            definition(switch_grammar_direct_default4 const& /*self*/)
            {
                r = switch_p [
                        case_p<'a'>(int_p),
                        case_p<'b'>(ch_p(',')),
                        case_p<'c'>(str_p("bcd")),
                        default_p
                    ];
            }

            rule<ScannerT> r;
            rule<ScannerT> const& start() const { return r; }
        };
    };

    struct switch_grammar_direct_default5 
    :   public grammar<switch_grammar_direct_default5>
    {
        template <typename ScannerT>
        struct definition 
        {
            definition(switch_grammar_direct_default5 const& /*self*/)
            {
                r = switch_p [
                        case_p<'a'>(int_p),
                        case_p<'b'>(ch_p(',')),
                        default_p,
                        case_p<'c'>(str_p("bcd"))
                    ];
            }

            rule<ScannerT> r;
            rule<ScannerT> const& start() const { return r; }
        };
    };

    struct switch_grammar_direct_default6 
    :   public grammar<switch_grammar_direct_default6>
    {
        template <typename ScannerT>
        struct definition 
        {
            definition(switch_grammar_direct_default6 const& /*self*/)
            {
                r = switch_p [
                        default_p,
                        case_p<'a'>(int_p),
                        case_p<'b'>(ch_p(',')),
                        case_p<'c'>(str_p("bcd"))
                    ];
            }

            rule<ScannerT> r;
            rule<ScannerT> const& start() const { return r; }
        };
    };

///////////////////////////////////////////////////////////////////////////////
//  Test the switch_p usage given a parser as the switch condition
    struct switch_grammar_parser_default4 
    :   public grammar<switch_grammar_parser_default4>
    {
        template <typename ScannerT>
        struct definition 
        {
            definition(switch_grammar_parser_default4 const& /*self*/)
            {
                r = switch_p(anychar_p) [
                        case_p<'a'>(int_p),
                        case_p<'b'>(ch_p(',')),
                        case_p<'c'>(str_p("bcd")),
                        default_p
                    ];
            }

            rule<ScannerT> r;
            rule<ScannerT> const& start() const { return r; }
        };
    };

    struct switch_grammar_parser_default5 
    :   public grammar<switch_grammar_parser_default5>
    {
        template <typename ScannerT>
        struct definition 
        {
            definition(switch_grammar_parser_default5 const& /*self*/)
            {
                r = switch_p(anychar_p) [
                        case_p<'a'>(int_p),
                        case_p<'b'>(ch_p(',')),
                        default_p,
                        case_p<'c'>(str_p("bcd"))
                    ];
            }

            rule<ScannerT> r;
            rule<ScannerT> const& start() const { return r; }
        };
    };

    struct switch_grammar_parser_default6 
    :   public grammar<switch_grammar_parser_default6>
    {
        template <typename ScannerT>
        struct definition 
        {
            definition(switch_grammar_parser_default6 const& /*self*/)
            {
                r = switch_p(anychar_p) [
                        default_p,
                        case_p<'a'>(int_p),
                        case_p<'b'>(ch_p(',')),
                        case_p<'c'>(str_p("bcd"))
                    ];
            }

            rule<ScannerT> r;
            rule<ScannerT> const& start() const { return r; }
        };
    };

///////////////////////////////////////////////////////////////////////////////
//  Test the switch_p usage given an actor as the switch condition
    struct select_result : public boost::spirit::closure<select_result, int>
    {
        member1 val;
    };

    struct switch_grammar_actor_default4 
    :   public grammar<switch_grammar_actor_default4>
    {
        template <typename ScannerT>
        struct definition 
        {
            definition(switch_grammar_actor_default4 const& /*self*/)
            {
                using phoenix::arg1;
                r = select_p('a', 'b', 'c', 'd')[r.val = arg1] >>
                    switch_p(r.val) [
                        case_p<0>(int_p),
                        case_p<1>(ch_p(',')),
                        case_p<2>(str_p("bcd")),
                        default_p
                    ];
            }

            rule<ScannerT, select_result::context_t> r;
            rule<ScannerT, select_result::context_t> const& 
            start() const { return r; }
        };
    };

    struct switch_grammar_actor_default5 
    :   public grammar<switch_grammar_actor_default5>
    {
        template <typename ScannerT>
        struct definition 
        {
            definition(switch_grammar_actor_default5 const& /*self*/)
            {
                using phoenix::arg1;
                r = select_p('a', 'b', 'c', 'd')[r.val = arg1] >>
                    switch_p(r.val) [
                        case_p<0>(int_p),
                        case_p<1>(ch_p(',')),
                        default_p,
                        case_p<2>(str_p("bcd"))
                    ];
            }

            rule<ScannerT, select_result::context_t> r;
            rule<ScannerT, select_result::context_t> const& 
            start() const { return r; }
        };
    };

    struct switch_grammar_actor_default6 
    :   public grammar<switch_grammar_actor_default6>
    {
        template <typename ScannerT>
        struct definition 
        {
            definition(switch_grammar_actor_default6 const& /*self*/)
            {
                using phoenix::arg1;
                r = select_p('a', 'b', 'c', 'd')[r.val = arg1] >>
                    switch_p(r.val) [
                        default_p,
                        case_p<0>(int_p),
                        case_p<1>(ch_p(',')),
                        case_p<2>(str_p("bcd"))
                    ];
            }

            rule<ScannerT, select_result::context_t> r;
            rule<ScannerT, select_result::context_t> const& 
            start() const { return r; }
        };
    };
    
}   // namespace test_grammars

///////////////////////////////////////////////////////////////////////////////
namespace tests {

    //  Tests for known (to the grammars) sequences
    struct check_grammar_known {
    
        template <typename GrammarT>
        void operator()(GrammarT)
        {
            GrammarT g;
            
            assert(parse("a1", g).full);
            assert(!parse("a,", g).hit);
            assert(!parse("abcd", g).hit);
            
            assert(parse("a 1", g, space_p).full);
            assert(!parse("a ,", g, space_p).hit);
            assert(!parse("a bcd", g, space_p).hit);
            
            assert(!parse("b1", g).hit);
            assert(parse("b,", g).full);
            assert(!parse("bbcd", g).hit);
            
            assert(!parse("b 1", g, space_p).hit);
            assert(parse("b ,", g, space_p).full);
            assert(!parse("b bcd", g, space_p).hit);
            
            assert(!parse("c1", g).hit);
            assert(!parse("c,", g).hit);
            assert(parse("cbcd", g).full);
            
            assert(!parse("c 1", g, space_p).hit);
            assert(!parse("c ,", g, space_p).hit);
            assert(parse("c bcd", g, space_p).full);
        }
    };

    //  Tests for known (to the grammars) sequences
    //  Tests for the default branches (without parsers) of the grammars
    struct check_grammar_default_plain {
    
        template <typename GrammarT>
        void operator()(GrammarT)
        {
            GrammarT g;
            
            assert(parse("d", g).full);
            assert(parse("d ", g, space_p).full);
        }
    };
    
}   // namespace tests

int 
main()
{
    //  Test switch_p parsers containing special (epsilon) default_p case 
    //  branches
    typedef boost::mpl::list<
    // switch_p syntax
        test_grammars::switch_grammar_direct_default4,
        test_grammars::switch_grammar_direct_default5,
        test_grammars::switch_grammar_direct_default6,
        
    // switch_p(parser) syntax
        test_grammars::switch_grammar_parser_default4,
        test_grammars::switch_grammar_parser_default5,
        test_grammars::switch_grammar_parser_default6,
        
    // switch_p(actor) syntax
        test_grammars::switch_grammar_actor_default4,
        test_grammars::switch_grammar_actor_default5,
        test_grammars::switch_grammar_actor_default6
    > default_epsilon_list_t;
    
    boost::mpl::for_each<default_epsilon_list_t>(tests::check_grammar_known());
    boost::mpl::for_each<default_epsilon_list_t>(
        tests::check_grammar_default_plain());

    return 0;
}
