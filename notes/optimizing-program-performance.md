# Chapter 5 Optimizing program performance

## Optimization blockers

**Memory aliasing**

```c
int *xp, *yp;
// a
*xp += *yp;
*xp += *yp;
// b
*xp = 2 * *yp;
```

The behaviors of `a` and `b` are different if `yp` and `xp` reference to the same position in memory.

```c
void sum_row(double *a, double *b, long n) {
    long i, j;
    for (i=0; i<n; i++) {
        b[i]=0;
        for (j=0; j<n; j++) {
            b[i]+=a[i*n+j];
        }
    }
}

// a better one
void sum_row(double *a, double *b, long n) {
    long i, j;
    for (i=0; i<n; i++) {
        double val=0;
        for (j=0; j<n; j++) {
            val+=a[i*n+j];
        }
        b[i]=val;
    }
}
```

**restrict** promises the compiler that pointers do not alias.

**Procedure side effect**	

A procedure may depend on the global program state. 

## Modern Processors

**Instruction-level parallelism**

*poly1.c*

```c
double poly(double x, double *a, int n) {
    double xpwr = x;
    double res = a[0];
    for (int i = 1; i < n; i++) {
        res = res + a[i] * xpwr;
        xpwr = x * xpwr;
    }
    return res;
}
```

*poly2.c*

```c
double poly(double x, double *a, int n) {
    double res = a[n - 1];
    for (int i = n - 2; i >= 0; i--) {
        res = res * x + a[i];
    }
    return res;
}
```

*result of running two function*

```bash
poly1:
$ ./poly1.o data-10000000.txt 10000000
5001294.138296
time: 0.014179s
poly2:
$ ./poly2.o data-10000000.txt 10000000
5001294.138295
time: 0.024363s
```

Even though *poly1.c* requires more operations, it runs fast than *poly2.c*. The reason lies on the design of modern processors. 

In *poly1.c*, `a[i] * xpwr; xpwr = x * xpwr;` can be computed simultaneously, and they do not depend on `res`. The updating of `res` only requires an addition. So the time of every iteration is just the time of a multiplication of double.

In *poly2.c*, `res = res * x + a[i]` requires `res` int previous iteration, and multiplication and addition cannot be processed simultaneously. Thus, the time of every iterations is the time of a multiplication and an addition.

## Loop Unrolling

## Enhancing Parallelism

**Multiple Accumulators**

```
acc1 = acc1 OP data[i]
acc2 = acc2 OP data[i+1]
...
res = acc1 OP acc2
```

**Reassociation Transformation**

```
acc = acc OP (data[i] OP data[i+1])
```

