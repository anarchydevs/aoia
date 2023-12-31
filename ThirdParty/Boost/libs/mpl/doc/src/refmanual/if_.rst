.. Metafunctions/Type Selection//if_ |10

if\_
====

Synopsis
--------

.. parsed-literal::
    
    template< 
          typename C
        , typename T1
        , typename T2
        >
    struct if\_
    {
        typedef |unspecified| type;
    };



Description
-----------

Returns one of its two arguments, ``T1`` or ``T2``, depending on the value ``C``.


Header
------

.. parsed-literal::
    
    #include <boost/mpl/if.hpp>


Parameters
----------

+---------------+-----------------------------------+-----------------------------------------------+
| Parameter     | Requirement                       | Description                                   |
+===============+===================================+===============================================+
| ``C``         | |Integral Constant|               | A selection condition.                        |
+---------------+-----------------------------------+-----------------------------------------------+
| ``T1``, ``T2``| Any type                          | Types to select from.                         |
+---------------+-----------------------------------+-----------------------------------------------+


Expression semantics
--------------------

For any |Integral Constant| ``c`` and arbitrary types ``t1``, ``t2``:


.. parsed-literal::

    typedef if_<c,t1,t2>::type t;

:Return type:
    Any type.

:Semantics:
    If ``c::value == true``, ``t`` is identical to ``t1``; otherwise ``t`` is 
    identical to ``t2``.


Example
-------

.. parsed-literal::
    
    typedef if\_<true\_,char,long>::type t1;
    typedef if\_<false\_,char,long>::type t2;

    BOOST_MPL_ASSERT(( is_same<t1, char> ));
    BOOST_MPL_ASSERT(( is_same<t2, long> ));


See also
--------

|Metafunctions|, |Integral Constant|, |if_c|, |eval_if|
