Limitations
###########

pybind11 strives to be a general solution to binding generation, but it also has
certain limitations:

- pybind11 casts away ``const``-ness in function arguments and return values.
  This is in line with the Python language, which has no concept of ``const``
  values. This means that some additional care is needed to avoid bugs that
  would be caught by the type checker in a traditional C++ program.

- Multiple inheritance relationships on the C++ side cannot be mapped to
  Python.

Both of these features could be implemented but would lead to a significant
increase in complexity. I've decided to draw the line here to keep this project
simple and compact. Users who absolutely require these features are encouraged
to fork pybind11.

