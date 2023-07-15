#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

// Unless specified otherwise, all functions have the implicit precondition of
// all their arguments being defined. Unless specified otherwise, all functions
// taking pointers have the implicit precondition that these pointers are valid,
// and their pointees, if they are pointers, also satisfy the property the
// argument itself satisfies. If an effect is not qualified by "may" or does not
// express a mere possibility, it is guaranteed.

// Element of our array
typedef unsigned long elem;

// Report number of successful allocations
// - Argument allocs: number of allocations to report
// - Returns: if printing was successful
// - Effects: Prints to stdout, may print to stderr
bool print_allocs(size_t allocs) {
    if (printf("successfully allocated %zu times\n", allocs) < 0) {
        perror("value output");
        return false;
    }
    return true;
}

// Parse argument to size_t
// - Argument argno: argument index in argv
// - Argument argid: argument name to be printed
// - Argument argv: argument array
// - Returns: parsed argument as size_t
// - Preconditions:
// + argv[argno] is defined
// + argid is nul-terminated
// - Effects: May exit program, may print to stderr
size_t get_arg_size_t(int argno, char *argid, char **argv) {
    char *arg = argv[argno];
    char *argptrcpy = arg;
    while (isspace(*argptrcpy))
        argptrcpy++;
    if (*argptrcpy == '-') {
        fprintf(stderr, "argument %s must be positive\n", argid);
        exit(EXIT_FAILURE);
    }
    char *endptr;
    errno = 0;
    uintmax_t val = strtoumax(arg, &endptr, 10);
    if (arg[0] == '\0' || *endptr != '\0') {
        fprintf(stderr, "failed to parse argument %s\n", argid);
        exit(EXIT_FAILURE);
    }
    if (errno == ERANGE || val > SIZE_MAX) {
        fprintf(stderr, "argument %s is too large\n", argid);
        exit(EXIT_FAILURE);
    }
    return (size_t)val;
}

// Free array and subarrays, and exit
// - Argument arr: array to free
// - Argument x: size of arr
// - Argument y: size of elements of arr
// - Argument i: index of latest element of arr for which allocation has begun,
// or x if allocation is finished
// - Argument j: index of latest element of arr[i] for which allocation has
// begun, or y if allocation is finished
// - Argument ecode: exit code with which to exit
// - Preconditions:
// + (1) For all i_ < i, arr[i_] is allocated
// + (2) For all i_ < i, j_ < y, arr[i_][j_] is defined
// + (3) For all j_ < j, arr[i][j_] is defined
// + arr is allocated
// - Correctness conditions:
// + In (1), these are the only such i_
// + In (2), these are the only such i_, j_
// + In (3), these are the only such j_
// - Owns:
// + arr
// + arr[i_] for i_ in (1)
// + arr[i_][j_] for i_, j_ in (2)
// + arr[i][j_] for j_ in (3)
// - Effects: frees all elem values in arr, exits program
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

// Allocate and initialize 3D array
// - Argument x: desired size of first layer of array
// - Argument y: desired size of each second layer of array
// - Argument z: desired size of each third layer of array
// - Argument allocs: pointer to store allocation count
// - Effects: allocates, writes *allocs, may print to stderr, may exit program
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

// Check argument count, correct user, and exit
// - Argument argc: argument count
// - Argument argv: argument array
// - Preconditions: for all i < argc, argv[i] is defined and nul-terminated
// - Effects: may print to stderr, may exit program
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

// Print elements of array
// - Argument arr: array to print
// - Argument x: size of arr
// - Argument y: size of elements of arr
// - Argument z: size of elements of elements of arr
// - Preconditions: for all i < x, j < y, k < z, arr[i][j][k] is defined
// - Owns:
// + arr
// + arr[i] for all i < x
// + arr[i][j] for all i < x, j < y
// - Effects: prints to stdout, may print to stderr, may exit program
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
