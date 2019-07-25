

# Chapter 6 The Memory Hierarchy

Main problem: reduce the performance gap between different hierarchy.

## Locality

**Temporal Locality**

Recently referenced items are likely to be referenced again in the near future.

**Spatial Locality**

Items with nearby addresses tend to be referenced close together in time.

## Cache

**Miss**

- Compulsory miss

  access data for the first time. (No useful data is in cache.)

- Conflict miss

  Two or more items map to the same position in cache. 

- Capacity miss

  The capacity of cache is limited.

### Cache-friendly Code

Properly arrangement of loops can make cache more efficient.

Focus on the inner loops, where bulk computations and memory accesses occur.

**Matrix Multiplication Example**

Block size = 32B. a, b, c are 2-dimension array of double.

**ijk**

```c
for (i=0; i<n; i++)
    for (j=0; j<n; j++)
        for (k=0; k<n; k++)
            c[i][j] += a[i][k] * b[k][j];
```

Misses per inner loop:

| A    | B    | C    |
| ---- | ---- | ---- |
| 0.25 | 1    | 0    |

**ikj**

```c
for (i=0; i<n; i++)
    for (k=0; k<n; k++)
        for (j=0; j<n; j++)
            c[i][j] += a[i][k] * b[k][j];
```

Misses per inner loop:

| A    | B    | C    |
| ---- | ---- | ---- |
| 0    | 0.25 | 0.25 |


