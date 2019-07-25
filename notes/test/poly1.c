
double poly(double x, double *a, int n) {
    double xpwr = x;
    double res = a[0];
    for (int i = 1; i < n; i++) {
        res = res + a[i] * xpwr;
        xpwr = x * xpwr;
    }
    return res;
}