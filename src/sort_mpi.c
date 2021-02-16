
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mpi.h"
#define RANGE_RAND 1000

void print_array(int* arr, int size) {
    for (int i = 0; i < size; i++) 
        printf(" %d", arr[i]);
    printf("\n");
}

int* merge(int* left_array, int* right_array, int left_size, int right_size) {
    int left_index = 0;
    int right_index = 0;
    int i = 0;
    int *merged_array = malloc(sizeof(int) * (left_size + right_size));

    // puts("mergindo left_array");
    // print_array(left_array, left_size);
    // puts("mergindo right_array");
    // print_array(right_array, right_size);
    for (; left_index < left_size && right_index < right_size;) {
        int left_value = left_array[left_index];
        int right_value = right_array[right_index];

        if (left_value < right_value) {
            merged_array[i++] = left_value;
            left_index++;
        } else {
            merged_array[i++] = right_value;
            right_index++;
        }
    }

    while(left_index < left_size) {
        merged_array[i++] = left_array[left_index++];
    }

    while(right_index < right_size) {
        merged_array[i++] = right_array[right_index++];
    }

    return merged_array;
}

int* merge_sort(int *local_array, int size) {
    if (size == 1) {
        int *new_arr = malloc(sizeof(int));
        new_arr[0] = local_array[0];
        return new_arr;
    }
    int splitting_point = size/2;
    int *right_pointer = local_array + splitting_point;

    int right_array_size = size - splitting_point;
    int *left_array = merge_sort(local_array, splitting_point);
    int *right_array = merge_sort(right_pointer, right_array_size);

    return merge(left_array, right_array, splitting_point, right_array_size);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        puts("Passe o valor de N como argumento");
        printf("%s N\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);

    int rank, n_procs, root = 0;
    int unsorted_array[N];
    int partially_sorted_array[N];
    int *sorted_array;
    int *local_array;

    srand(time(NULL));

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &n_procs);

    int partition = N/n_procs;

    double start = MPI_Wtime();
    if (rank == root) {
        for(int i = 0; i < N; i++) {
            unsorted_array[i] = rand() % RANGE_RAND;
            printf(" %d", unsorted_array[i]);
        }
        puts("");
    }
    local_array = malloc(sizeof(int)*partition);
    /* O vetor é distribuído em partes iguais entre os processos, incluindo o processo raiz */
    MPI_Scatter(unsorted_array, partition, MPI_INT, local_array, partition, MPI_INT, root, MPI_COMM_WORLD);

    // printf("Processo =  %d, recebeu", rank);
    // print_array(local_array, partition);

    local_array = merge_sort(local_array, partition);
    // print_array(local_array, partition);

    MPI_Gather(local_array, partition, MPI_INT, partially_sorted_array, partition, MPI_INT, root, MPI_COMM_WORLD);

    if (rank == root) {
        sorted_array = partially_sorted_array;
        for (int proc = 1; proc < n_procs; proc++) {
            sorted_array = merge(sorted_array, partially_sorted_array + proc*partition, proc*partition, partition);
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
