
A *view* is a sequence adaptor delivering an altered presentation of 
one or more underlying sequences. Views are lazy, meaning that their 
elements are only computed on demand. Similarly to the short-circuit 
|logical operations| and |eval_if|, views make it possible to avoid 
premature errors and inefficiencies from computations whose results 
will never be used. When approached with views in mind, many 
algorithmic problems can be solved in a simpler, more conceptually 
precise, more expressive way.

.. |Views| replace:: `Views`_
