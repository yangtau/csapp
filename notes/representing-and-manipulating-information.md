# Chapter 2 Representing and Manipulating Information

## Integers

### Conversion

Mappings between unsigned and two's complement numbers: **Keep bit representations and reinterpret**.

In C, if there is a mix of signed and unsigned in single expression, signed values implicitly converted to unsigned.

```c
// W = 32
-1 > 0U // true
```

**Don't use unsigned without understanding implications**

```c
#define DELTA sizeof(int)
...
int i;
for (i = CNT; i-DELTA >= 0; i-= DELTA)
    ...
```

