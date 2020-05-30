# Chapter 3 Machine-Level Representation of Programs

## Arrays

`int (*arr)[ROW]` equals to`int arr[][ROW]` as function arguments, which is different from `int* arr[ROW]`(In fact, `Row` in this term is not necessary).

*func1.c*

```c
int func(int* arr[], int x, int y) { 
    return arr[x][y]; 
}
```

*func2.c*

```c
#define ROW 3
int func(int arr[][ROW], int x, int y) { 
    return arr[x][y]; 
}
// or euaqlly
int func(int (*arr)[ROW], int x, int y) { 
    return arr[x][y]; 
}
```

Use `objdump` to see the assembly code.

*func1.s*

```assembly
movslq %esi,%rsi          # x => %rsi
mov    (%rdi,%rsi,8),%rax # get the pointer of the sub-array
                          # arr + 8 * x => %rax
movslq %edx,%rdx          # y => %rdx
mov    (%rax,%rdx,4),%eax # %rax + 4 * y
retq   
```

*func2.s*

```assembly
movslq %esi,%rsi
lea    (%rsi,%rsi,2),%rcx # 3 * x => %rcx
lea    0x0(,%rcx,4),%rax  # 4 * %rcx => %rax (12 * x)
add    %rax,%rdi          # (12 * x) + arr
movslq %edx,%rdx
mov    (%rdi,%rdx,4),%eax # [arr + 12 * x + 4 * y]
retq   
```



**`int arr[][ROW]` VS  `int* arr[ROW]`**

- They are different types. The former is two-dimensional array, and the latter is  an  array of pointer.
- They can access by the same way.(`arr[x][y]`)



**Understanding Pointers and Arrays**

Size of different types.

|                   | `An​` | `*An​` | `**An` | `***An` |
| ----------------- | ---- | :---- | ------ | ------- |
| `int A1[3][5]`    | 60   | 20    | 4      | -       |
| `int *A2[3][5]`   | 120  | 40    | 8      | 4       |
| `int (*A3)[3][5]` | 8    | 60    | 20     | 4       |
| `int *(A4[3][5])` | 120  | 40    | 8      | 4       |
| `int (*A5[3])[5]` | 24   | 8     | 20     | 4       |

`int (*A3)[3][5]`

> declare A3 as pointer to array 3 of array 5 of int

`int (*A5[3])[5]`

> declare A5 as array 3 of pointer to array 5 of int

[cdecl](https://cdecl.org/) is a good tool to understand complex declarations.

**Something more crazy...**

`int (*(*f)[13])()` 

> `f ` is pointer to array[13] of pointer to function returning int

`int (**f[13])()`

> `f ` is array[13] of pointer of pointer to function returning int

***Note***: **`->`, `()`, and `[]` have high precedence, with `*` and `&` just below.**

`int (*(*x[3])()) [5]`

> x is array[3] of pointer to function returning pointer to array[5] of int

## Overflow

### Overflow Attacks

Overwrite the return address; Input string contains byte representation of executable code.

#### Solution

1. Avoid overflow vulnerabilities in code

   - ` fgets` instead of `gets`
   - `strncpy` instead of `strcpy`
   - ...

2. System-Level Protections

   - add `execute` permission
   - stack marked as non-executable
   - make it hard to insert binary code

3. Stack Canaries

   - `canary` is positioned between array `buf` and the saved state including return address. 

   - check the `canary` to determine whether or not the stack state has been corrupted.

4. Stack Address Randomization

   - make it hard to predict the buffer location


