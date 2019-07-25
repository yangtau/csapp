
double poly(double x, double *a, int n) {
    double res = a[n - 1];
    for (int i = n - 2; i >= 0; i--) {
        res = res * x + a[i];
    }
    return res;
}