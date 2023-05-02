#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {
    /*
    double* ptr = sf_malloc(sizeof(double));

    
    *ptr = 320320320e-320;

    printf("%f\n", *ptr);

    sf_free(ptr);
    */
    size_t sz_1 = 321, sz_2 = 5,sz_3 = 456;
    sf_malloc(sz_1);
    void *y = sf_malloc(sz_2);
    void *z = sf_malloc(sz_3);

    sf_free(y);
    sf_free(z);
    sf_show_heap();
    return EXIT_SUCCESS;
}
