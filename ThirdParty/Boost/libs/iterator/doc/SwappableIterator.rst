Swappable Iterator Concept
..........................

A class or built-in type ``X`` models the *Swappable Iterator* concept
if, in addition to ``X`` being Copy Constructible, the following
expressions are valid and respect the stated semantics.

+---------------------------------------------------------------------+
|Swappable Iterator Requirements (in addition to Copy Constructible)  |
+-------------------------+-------------+-----------------------------+
|Expression               |Return Type  |Postcondition                |
+=========================+=============+=============================+
|``iter_swap(a, b)``      |``void``     |the pointed to values are    |
|                         |             |exchanged                    |
+-------------------------+-------------+-----------------------------+

[*Note:* An iterator that is a model of the *Readable* and *Writable Iterator* concepts
  is also a model of *Swappable Iterator*.  *--end note*]

