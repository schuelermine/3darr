#include <errno.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

void report(size_t allocs) {
    printf("successfully allocated %zu times\n", allocs);
}

unsigned long getularg(int argno, char *argid, char **argv) {
    char *endptr;
    errno = 0;
    unsigned long val = strtoul(argv[argno], &endptr, 10);
    if (argv[1][0] == '\0' || *endptr != '\0') {
        fprintf(stderr, "failed to parse argument %s\n", argid);
        exit(EXIT_FAILURE);
    }
    if (errno == ERANGE) {
        fprintf(stderr, "argument %s is too large\n", argid);
        exit(EXIT_FAILURE);
    }
    return val;
}

void cleanup(unsigned long ***arr, size_t x, size_t y, size_t i, size_t j) {
    for (size_t i_ = 0; i_ < i; i_++) {
        for (size_t j_ = 0; j_ < y; j_++)
            free(arr[i_][j_]);
        free(arr[i_]);
    }
    if (i < x) {
        for (size_t j_ = 0; j_ < j; j_++)
            free(arr[i][j_]);
        free(arr[i]);
    }
    free(arr);
}

int main(int argc, char **argv) {
    if (argc != 4) {
        // char *pname = argc == 0 ? "<program>" : argv[0];
        char *pname = argv[0];
        fprintf(stderr,
                "wrong usage!\n"
                "usage: %s <x> <y> <z>\n",
                pname);
        return EXIT_FAILURE;
    }
    unsigned long x = getularg(1, "x", argv);
    unsigned long y = getularg(2, "y", argv);
    unsigned long z = getularg(3, "z", argv);
    size_t allocs = 0;
    unsigned long ***arr = malloc(x * sizeof(unsigned long **));
    if (arr == NULL) {
        perror("array allocation");
        report(allocs);
        return EXIT_FAILURE;
    }
    allocs++;
    for (size_t i = 0; i < x; i++) {
        arr[i] = malloc(y * sizeof(unsigned long *));
        if (arr[i] == NULL) {
            perror("array allocation");
            report(allocs);
            cleanup(arr, x, y, i, 0);
            return EXIT_FAILURE;
        }
        allocs++;
        for (size_t j = 0; j < y; j++) {
            arr[i][j] = malloc(z * sizeof(unsigned long));
            if (arr[i][j] == NULL) {
                perror("array allocation");
                report(allocs);
                cleanup(arr, x, y, i, j);
                return EXIT_FAILURE;
            }
            allocs++;
            for (size_t k = 0; k < z; k++)
                arr[i][j][k] = pow(2, i) + pow(3, j) + pow(5, k);
        }
    }
    report(allocs);
    for (size_t i = 0; i < x; i++)
        for (size_t j = 0; j < y; j++)
            for (size_t k = 0; k < z; k++)
                if (printf("arr[%zu][%zu][%zu] = %lu\n", i, j, k,
                           arr[i][j][k]) < 0) {
                    perror("value output");
                    cleanup(arr, x, y, x, y);
                    return EXIT_FAILURE;
                }
    cleanup(arr, x, y, x, y);
    return EXIT_SUCCESS;
}
