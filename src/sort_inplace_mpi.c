
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "mpi.h"
#define N 20
#define RANGE_RAND 100

void print_array(int* arr, int size) {
    for (int i = 0; i < size; i++) 
        printf(" %d", arr[i]);
    printf("\n");
}

void merge(int* array, int left_index, int middle, int right_index, int debug) {
    int i = left_index;
    int left_limit = middle - left_index + 1;
    int right_limit = right_index - middle;

    int left_arr[left_limit];
    int right_arr[right_limit];

    for(int j = 0; j < left_limit; j++)
        left_arr[j] = array[left_index + j];

    for(int j = 0; j < right_limit; j++)
        right_arr[j] = array[middle + 1 + j];

    if (debug) {
        puts("mergindo left_array");
        print_array(left_arr, left_limit);
        puts("mergindo right_array");
        print_array(right_arr, right_limit);
    }

    int l = 0;
    int r = 0;

    for (; l < left_limit && r < right_limit;) {
        int left_value = left_arr[l];
        int right_value = right_arr[r];

        if (left_value < right_value) {
            array[i++] = left_value;
            l++;
        } else {
            array[i++] = right_value;
            r++;
        }
    }

    while(l < left_limit) {
        array[i++] = left_arr[l++];
    }

    while(r < right_limit) {
        array[i++] = right_arr[r++];
    }
}

void merge_sort(int *local_array, int left_index, int right_index) {
    if (left_index >= right_index) return;

    int splitting_point = left_index + (right_index - left_index)/2;
    merge_sort(local_array, left_index, splitting_point);
    merge_sort(local_array, splitting_point + 1, right_index);
    merge(local_array, left_index, splitting_point, right_index, 0);
}

int main(int argc, char *argv[]) {
    int rank, n_procs, root = 0;
    int unsorted_array[N];
    int *sorted_array;
    int *local_array;

    srand(time(NULL));

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &n_procs);

    int partition_size = N/n_procs;

    double start = MPI_Wtime();
    if (rank == root) {
        for(int i = 0; i < N; i++) {
            unsorted_array[i] = rand() % RANGE_RAND;
            printf(" %d", unsorted_array[i]);
        }
        puts("");
    }
    local_array = malloc(sizeof(int)*partition_size);
    /* O vetor é distribuído em partes iguais entre os processos, incluindo o processo raiz */
    MPI_Scatter(unsorted_array, partition_size, MPI_INT, local_array, partition_size, MPI_INT, root, MPI_COMM_WORLD);

    printf("Processo =  %d, recebeu", rank);
    print_array(local_array, partition_size);

    merge_sort(local_array, 0, partition_size - 1);
    printf("Locally sorted array: ");
    print_array(local_array, partition_size);

    sorted_array = malloc(sizeof(int) * N);
    MPI_Gather(local_array, partition_size, MPI_INT, sorted_array, partition_size, MPI_INT, root, MPI_COMM_WORLD);

    if (rank == root) {
        int limit_tree = log2(n_procs);
        for (int j = 0, grouping_size = 2; j < limit_tree; j++, grouping_size <<= 1) {
            int merge_size = grouping_size * partition_size;
            for (int begin_merge = 0, end_merge = merge_size - 1; begin_merge < N; begin_merge = end_merge + 1, end_merge += merge_size) {
                int middle = (begin_merge + end_merge)/2;
                merge(sorted_array, begin_merge, middle, end_merge, 1);
            }
        }

        double end = MPI_Wtime();
        puts("Sorted array:");
        print_array(sorted_array, N);
        printf("Demorou %.4f segundos para ordenar %d valores\n", end - start, N);
    }

    /* Termina a execução */
    MPI_Finalize();
    return(0);
}
