#include <errno.h>
#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

typedef unsigned long elem;

bool print_allocs(size_t allocs) {
    if (printf("successfully allocated %zu times\n", allocs) < 0) {
        perror("value output");
        return false;
    }
    return true;
}

size_t get_arg_size_t(int argno, char *argid, char **argv) {
    char *endptr;
    errno = 0;
    uintmax_t val = strtoumax(argv[argno], &endptr, 10);
    if (argv[1][0] == '\0' || *endptr != '\0') {
        fprintf(stderr, "failed to parse argument %s\n", argid);
        exit(EXIT_FAILURE);
    }
    if (errno == ERANGE || val > SIZE_MAX) {
        fprintf(stderr, "argument %s is too large\n", argid);
        exit(EXIT_FAILURE);
    }
    return (size_t)val;
}

void free_and_exit(elem ***arr, size_t x, size_t y, size_t i, size_t j,
                   int ecode) {
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
    exit(ecode);
}

elem ***mk_arr(size_t x, size_t y, size_t z, size_t *allocs) {
    *allocs = 0;
    elem ***arr = malloc(x * sizeof(elem **));
    if (arr == NULL) {
        perror("array allocation");
        print_allocs(*allocs);
        exit(EXIT_FAILURE);
    }
    ++*allocs;
    for (size_t i = 0; i < x; i++) {
        arr[i] = malloc(y * sizeof(elem *));
        if (arr[i] == NULL) {
            perror("array allocation");
            print_allocs(*allocs);
            free_and_exit(arr, x, y, i, 0, EXIT_FAILURE);
        }
        ++*allocs;
        for (size_t j = 0; j < y; j++) {
            arr[i][j] = malloc(z * sizeof(elem));
            if (arr[i][j] == NULL) {
                perror("array allocation");
                print_allocs(*allocs);
                free_and_exit(arr, x, y, i, j, EXIT_FAILURE);
            }
            ++*allocs;
            for (size_t k = 0; k < z; k++)
                arr[i][j][k] = pow(2, i) + pow(3, j) + pow(5, k);
        }
    }
    return arr;
}

void ensure_usage(int argc, char **argv) {
    if (argc != 4) {
        char *pname = argc == 0 ? "<program>" : argv[0];
        fprintf(stderr,
                "wrong usage!\n"
                "usage: %s <x> <y> <z>\n",
                pname);
        exit(EXIT_FAILURE);
    }
}

void print_arr(elem ***arr, size_t x, size_t y, size_t z) {
    for (size_t i = 0; i < x; i++)
        for (size_t j = 0; j < y; j++)
            for (size_t k = 0; k < z; k++)
                if (printf("arr[%zu][%zu][%zu] = %lu\n", i, j, k,
                           arr[i][j][k]) < 0) {
                    perror("value output");
                    free_and_exit(arr, x, y, x, y, EXIT_FAILURE);
                }
}

int main(int argc, char **argv) {
    ensure_usage(argc, argv);
    size_t x = get_arg_size_t(1, "x", argv);
    size_t y = get_arg_size_t(2, "y", argv);
    size_t z = get_arg_size_t(3, "z", argv);
    size_t allocs;
    elem ***arr = mk_arr(x, y, z, &allocs);
    if (!print_allocs(allocs)) {
        perror("value output");
        free_and_exit(arr, x, y, x, y, EXIT_FAILURE);
    }
    print_arr(arr, x, y, z);
    free_and_exit(arr, x, y, x, y, EXIT_SUCCESS);
}
