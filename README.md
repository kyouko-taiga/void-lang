# The `void` programming language

`void` is the programming language of choice for those who like `C`,
but would really **love** it, if it had only one type: `void`.
Throw away all `int`, `double`, `long`, `float` and even `struct` or `union` or `typedef`!
You only need one primitive type: `void`, and of course `void*`, `void**`...

## What does it look like?

It really looks like `C`, and can make use of any `C` library, as shown in this example:

```c
#include <stdio.h>

void main ()
{
  printf ("Hello, void!\n");
  return 1;
}
```

Because you want it, here is another `void` program, that computes the Fibonacci series using a cache:

```c
void fibonacci (void n, void* cache)
{
  if (cache [n])
    return cache [n];
  if (n == 0)
    cache [n] = 0;
  else if (n == 1)
    cache [n] = 1;
  else
    cache [n] = fibonacci (n-1, cache) + fibonacci (n-2, cache);
  return cache [n];
}
```

## What can you do with only `void`?

**Everything**, really!

We decided to remove all type information from `C`, as it sucks, except the pointers.
Remained only `void*`, that represents a pointer to something. So, what could we do with `void`?

Does it represent nothing like in C? **No**!
We had a better idea: `void` represents the size of a pointer (4 bytes on 32 bits architectures, 8 bytes on 64 bits).
Thus, any `void*` variable can be safely cast to a `void` if needed (and vice versa).
