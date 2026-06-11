# Triangle

C implementation of the Khronos triangle tutorial (C++). The codebase is messy
as a lot is object oriented code patterns translated to a procedural language.
As the primary goal is learning, no LLM has been used at any point - it's an
all-handmade serving of spaghetti.

## Some reflection

The application works! The implementation is messy and there are several things
I'd change, were I to do it again, but it works! There are some things
remaining, like the fact that I should've stuck to snake-case from the
beginning, rather than hearing the tutorials case on camel. I'll probably get
around to doing it later (meaning it won't get done, ever). Of these the most
important one is likely error handling. In its current state it's all binary,
meaning a function either fails or proceeds, and any information beyond that is
provided by a generous amount of print statements. This should probably be
replaced with a more verbose error system, and the current messages could be
elevated to a log level, bringing me to the next point:

Logging. I love well designed log modules, and have written some header only
ones myself. I really want to implement configurable log states, covering
printing for setup, debug and verbose runtime information. This is quite a
hassle to retrofit, but were I to restart from scratch I'd probably get it
running before anything else.

In the end, nothing beats seeing the triangle appear at the end. This joy was
further accentuated by Wayland's quirk of not showing windows unless drawn to,
meaning the complete application was my first graphical result.
