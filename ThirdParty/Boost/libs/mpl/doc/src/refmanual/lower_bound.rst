.. Algorithms/Querying Algorithms//lower_bound |60

lower_bound
===========

Synopsis
--------

.. parsed-literal::
    
    template<
          typename Sequence
        , typename T
        , typename Pred = less<_1,_2>
        >
    struct lower_bound
    {
        typedef |unspecified| type;
    };



Description
-----------

Returns the first position in the sorted ``Sequence`` where ``T`` could be inserted without 
violating the ordering.


Header
------

.. parsed-literal::
    
    #include <boost/mpl/lower_bound.hpp>


Parameters
----------

+---------------+-------------------------------+-----------------------------------+
| Parameter     | Requirement                   | Description                       |
+===============+===============================+===================================+
|``Sequence``   | |Forward Sequence|            | A sorted sequence to search in.   |
+---------------+-------------------------------+-----------------------------------+
|``T``          | Any type                      | A type to search a position for.  |
+---------------+-------------------------------+-----------------------------------+
|``Pred``       | Binary |Lambda Expression|    | A search criteria.                |
+---------------+-------------------------------+-----------------------------------+


Expression semantics
--------------------

For any sorted |Forward Sequence| ``s``, binary |Lambda Expression| ``pred``, and
arbitrary type ``x``:


.. parsed-literal::

    typedef lower_bound< s,x,pred >::type i; 

:Return type:
    |Forward Iterator|.

:Semantics:
    ``i`` is the furthermost iterator in |begin/end<s>| such that, for every iterator 
    ``j`` in [``begin<s>::type``, ``i``),
    
    .. parsed-literal::
    
        apply< pred, deref<j>::type, x >::type::value == true



Complexity
----------

The number of comparisons is logarithmic: at most log\ :sub:`2`\ ( ``size<s>::value`` ) + 1. 
If ``s`` is a |Random Access Sequence| then the number of steps through the range 
is also logarithmic; otherwise, the number of steps is proportional to 
``size<s>::value``.


Example
-------

.. parsed-literal::
    
    typedef vector_c<int,1,2,3,3,3,5,8> numbers;
    typedef lower_bound< numbers, int_<3> >::type iter;

    BOOST_MPL_ASSERT_RELATION( 
          (distance< begin<numbers>::type,iter >::value), ==, 2
        );
    
    BOOST_MPL_ASSERT_RELATION( deref<iter>::type::value, ==, 3 );


See also
--------

|Querying Algorithms|, |upper_bound|, |find|, |find_if|, |min_element|
