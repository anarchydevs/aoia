/* Copyright Bruno da Silva de Oliveira 2003. Use, modification and 
 distribution is subject to the Boost Software License, Version 1.0.
 (See accompanying file LICENSE_1_0.txt or copy at 
 http:#www.boost.org/LICENSE_1_0.txt) 
 */
namespace inherit4 {
    
struct A
{
    int x;
};  

struct B: A
{
    int y;
};

struct C: B
{
    int z;
};   

}
