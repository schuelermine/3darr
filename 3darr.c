#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * @file 3darr.c
 * Code comprising the `3darr` program.
 * Allocates a 3D array with dimensions specified by the arguments, populates it
 * with unique values, and prints it.
 * Unless specified otherwise, all functions have the implicit precondition of
 * all their arguments being defined. Unless specified otherwise, all functions
 * taking pointers have the implicit precondition that these pointers are valid,
 * and their pointees, if they are pointers, also satisfy the property the
 * argument itself satisfies. If an effect is not qualified by "may" or does not
 * express a mere possibility, it is guaranteed.
 */

/**
 * Element of our array.
 * The fact that this is a unsigned long is relied on.
 */
typedef unsigned long elem;

/**
 * Report number of successful allocations.
 * @param allocs number of allocations to report.
 *
 * @return if printing was successful.
 *
 * **Effects**: Prints to stdout, may print to stderr.
 */
bool print_allocs(size_t allocs) {
    if (printf("successfully allocated %zu times\n", allocs) < 0) {
        perror("value output");
        return false;
    }
    return true;
}

/**
 * Parse argument to `size_t`.
 *
 * @param arg argument string to be processed.
 * @param name argument name to be printed.
 *
 * @return parsed argument as `size_t`.
 *
 * @pre
 * - `arg` is nul-terminated.
 * - `name` is nul-terminated.
 *
 * **Effects**: may exit program, may print to stderr.
 */
size_t get_arg_size_t(char *arg, char *name) {
    char *argptrcpy = arg;
    while (isspace(*argptrcpy))
        argptrcpy++;
    if (*argptrcpy == '-') {
        fprintf(stderr, "argument %s must be positive\n", name);
        exit(EXIT_FAILURE);
    }
    char *endptr;
    errno = 0;
    uintmax_t val = strtoumax(arg, &endptr, 10);
    if (arg[0] == '\0' || *endptr != '\0') {
        fprintf(stderr, "failed to parse argument %s\n", name);
        exit(EXIT_FAILURE);
    }
    if (errno == ERANGE || val > SIZE_MAX) {
        fprintf(stderr, "argument %s is too large\n", name);
        exit(EXIT_FAILURE);
    }
    return (size_t)val;
}

/** Free subarrays up to excluding arr[i].
 *
 * @param arr array to free.
 * @param i index of latest element of `arr` for which allocation has begun,
 * or the size of `arr` if allocation is finished.
 * @param y size of elements of `arr`.
 *
 * @pre
 * - (1) for all `i_ < i`, `arr[i_]` is defined and allocated.
 * - (2) for all `i_ < i`, `j < y`, `arr[i_][j]` is defined and allocated.
 *
 * **Correctness conditions**
 * - in (1), these are the only such `i_`.
 * - in (2), these are the only such `i_`, `j`.
 *
 * **Frees**
 * - `arr[i_]` for `i_` in (1).
 * - `arr[i_][j]` for `i_`, `j` in (2).
 * - Effects: frees some pointers derived from `arr`.
 */
void free_sub_arr_up_to(elem ***arr, size_t i, size_t y) {
    for (size_t i_ = 0; i_ < i; i_++) {
        for (size_t j = 0; j < y; j++)
            free(arr[i_][j]);
        free(arr[i_]);
    }
}

/** Free completely allocated array.
 *
 * @param arr: array to free.
 * @param x: size of `arr`.
 * @param y: size of elements of `arr`.
 *
 * @pre
 * - (1) for all `i < x`, `arr[i]` is defined and allocated.
 * - (2) for all `i < x`, `j < y, arr[i][j]` is defined and allocated.
 * - `arr` is allocated.
 *
 * **Correctness conditions**
 * - in (1), these are the only such `i`.
 * - in (2), these are the only such `i`, `j`.
 *
 * **Frees**
 * - `arr[i]` for `i` in (1).
 * - `arr[i][j]` for `i`, `j` in (2).
 *
 * **Effects**: frees arr and all valid pointers derived from it.
 */
void free_complete_arr(elem ***arr, size_t x, size_t y) {
    free_sub_arr_up_to(arr, x, y);
    free(arr);
}

/** Free incomplete array.
 *
 * @param arr: array to free.
 * @param y: size of elements of arr.
 * @param i: index of latest element of arr for which allocation has begun.
 * @param j: index of latest element of `arr[i]` for which allocation has
 * begun, or y if allocation of that subarray is finished.
 *
 * @pre
 * - (1) for all `i_ < i`, `arr[i_]` is defined and allocated.
 * - (2) for all `i_ < i`, `j < y`, `arr[i_][j]` is defined and allocated.
 * - (3) for all `j_ < j`, `arr[i][j_]` is defined and allocated.
 * - arr is allocated.
 *
 * **Correctness conditions**
 * - in (1), these are the only such `i_`.
 * - in (2), these are the only such `i_`, `j`.
 * - in (3), these are the only such `j_`.
 *
 * **Frees**
 * - `arr`
 * - `arr[i_]` for `i_` in (1).
 * - `arr[i_][j]` for `i_`, `j_` in (2).
 * - `arr[i][j_]` for `j_` in (3).
 *
 * **Effects**: frees `arr` and all valid pointers derived from it.
 */
void free_incomplete_arr(elem ***arr, size_t y, size_t i, size_t j) {
    free_sub_arr_up_to(arr, i, y);
    for (size_t j_ = 0; j_ < j; j_++)
        free(arr[i][j_]);
    free(arr[i]);
    free(arr);
}

/** Calculate `x` to the `y`-th power using exponentiation by squaring.
 *
 * @param x base of exponentiation.
 * @param y exponent.
 */
elem elem_pow(elem x, size_t y) {
    elem result = 1;
    while (true) {
        if ((y & 1) == 1)
            result *= x;
        y >>= 1;
        if (y == 0)
            break;
        x *= x;
    }
    return result;
}

/** Allocate and initialize 3D array.
 *
 * @param x desired size of first layer of array.
 * @param y desired size of each second layer of array.
 * @param z desired size of each third layer of array.
 * @param[out] allocs: pointer to store allocation count.
 *
 * **Effects**: allocates, writes *allocs, may print to stderr, may exit
 * program.
 *
 * @post
 * for all `i < x`, `j < y`, `k < z`, `x[i][j][k]` is defined, where x is the
 * return value.
 */
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
            free_incomplete_arr(arr, y, i, 0);
            exit(EXIT_FAILURE);
        }
        ++*allocs;
        for (size_t j = 0; j < y; j++) {
            arr[i][j] = malloc(z * sizeof(elem));
            if (arr[i][j] == NULL) {
                perror("array allocation");
                print_allocs(*allocs);
                free_incomplete_arr(arr, y, i, j);
                exit(EXIT_FAILURE);
            }
            ++*allocs;
            for (size_t k = 0; k < z; k++)
                // First three prime numbers
                arr[i][j][k] = elem_pow(2, i) * elem_pow(3, j) * elem_pow(5, k);
        }
    }
    return arr;
}

/** Check argument count, correct user, and exit.
 *
 * @param argc argument count.
 * @param argv_0 first argument string (usually the program name).
 *
 * @pre
 * - only if `argc == 0`, `argv_0` may be undefined.
 * - `argv_0`, if defined, is nul-terminated.
 *
 * **Effects**: may print to stderr, may exit program.
 *
 * @post
 * for all `i < 4`, `argv[i]` is defined.
 */
void ensure_usage(int argc, char *argv_0) {
    if (argc != 4) {
        char *pname = argc == 0 || argv_0[0] == '\0' ? "<program>" : argv_0;
        fprintf(stderr,
                "wrong usage!\n"
                "usage: %s <x> <y> <z>\n",
                pname);
        exit(EXIT_FAILURE);
    }
}

/** Print elements of array.
 *
 * @param arr: array to print.
 * @param x: size of `arr`.
 * @param y: size of elements of `arr`.
 * @param z: size of elements of elements of `arr`.
 *
 * @pre
 * for all `i < x`, `j < y`, `k < z`, `arr[i][j][k]` is defined.
 *
 * **Owns**
 * - `arr`.
 * - `arr[i]` for all `i < x`.
 * - `arr[i][j]` for all `i < x`, `j < y`.
 *
 * **Effects**: prints to stdout, may print to stderr, may exit program.
 */
void print_arr(elem ***arr, size_t x, size_t y, size_t z) {
    for (size_t i = 0; i < x; i++)
        for (size_t j = 0; j < y; j++)
            for (size_t k = 0; k < z; k++)
                if (printf("arr[%zu][%zu][%zu] = %lu\n", i, j, k,
                           arr[i][j][k]) < 0) {
                    perror("value output");
                    free_complete_arr(arr, x, y);
                    exit(EXIT_FAILURE);
                }
}

/** Main function of the `3darr` program.
 *
 * Allocates a 3D array with dimensions specified by the arguments, populates it
 * with unique values, and prints it.
 */
int main(int argc, char **argv) {
    ensure_usage(argc, argv[0]);
    size_t x = get_arg_size_t(argv[1], "x");
    size_t y = get_arg_size_t(argv[2], "y");
    size_t z = get_arg_size_t(argv[3], "z");
    size_t allocs;
    elem ***arr = mk_arr(x, y, z, &allocs);
    if (!print_allocs(allocs)) {
        perror("value output");
        free_complete_arr(arr, x, y);
        return EXIT_FAILURE;
    }
    print_arr(arr, x, y, z);
    free_complete_arr(arr, x, y);
    return EXIT_SUCCESS;
}
