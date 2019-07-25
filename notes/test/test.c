#include <stdio.h>
#include <stdlib.h>
#include <time.h>

extern double poly(double x, double *a, int n);

int main(int argc, char const *argv[]) {
    if (argc != 3) {
        printf("argument should be : filename length\n");
        exit(-1);
    }
    char *file = argv[1];
    const int len = atoi(argv[2]);
    // char *file = "data-10000000.txt";
    // const int len = 10000000;
    freopen(file, "r", stdin);
    double *arr = (double *)malloc(len * sizeof(double));
    for (int i = 0; i < len; i++) {
        scanf("%lf", &arr[i]);
    }
    double x = 1;
    clock_t start = clock();
    printf("%lf\n", poly(x, arr, len));
    clock_t finish = clock();
    printf("time: %lfs\n", (double)(finish - start) / CLOCKS_PER_SEC);
    return 0;
}
