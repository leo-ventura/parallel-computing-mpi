
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "mpi.h"
#define RANGE_RAND 100

void print_array(int* arr, int size) {
    for (int i = 0; i < size; i++) 
        printf(" %d", arr[i]);
    printf("\n");
}

void merge(int* array, int left_index, int middle, int right_index) {
    int i = left_index;
    int left_limit = middle - left_index + 1;
    int right_limit = right_index - middle;

    int *left_arr = malloc(sizeof(int) * left_limit);
    int *right_arr = malloc(sizeof(int) * right_limit);

    for(int j = 0; j < left_limit; j++)
        left_arr[j] = array[left_index + j];

    for(int j = 0; j < right_limit; j++)
        right_arr[j] = array[middle + 1 + j];

    // puts("mergindo left_array");
    // print_array(left_arr, left_limit);
    // puts("mergindo right_array");
    // print_array(right_arr, right_limit);

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
    merge(local_array, left_index, splitting_point, right_index);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        puts("Passe o valor de N como argumento");
        return 1;
    }

    int N = atoi(argv[1]);

    int rank, n_procs, root = 0;
    int *unsorted_array = malloc(sizeof(int) * N);
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
            // printf(" %d", unsorted_array[i]);
        }
        // puts("");
    }
    local_array = malloc(sizeof(int)*partition_size);

    int remaining_values = N % n_procs;
    int *sendcounts = calloc(n_procs, sizeof(int));
    int *displacement = calloc(n_procs, sizeof(int));
    int sum = 0;
    for (int i = 0; i < n_procs; ++i) {
        sendcounts[i] = partition_size;
        if (remaining_values > 0) {
            sendcounts[i] += partition_size;
            remaining_values -= partition_size;
        }

        displacement[i] = sum;
        sum += sendcounts[i];
    }

    // puts("Sendcounts:");
    // print_array(sendcounts, n_procs);
    // puts("Displacement:");
    // print_array(displacement, n_procs);

    int self_partition_size = sendcounts[rank];
    /* O vetor é distribuído em partes iguais entre os processos, incluindo o processo raiz */
    MPI_Scatterv(unsorted_array, sendcounts, displacement, MPI_INT, local_array,
        self_partition_size, MPI_INT, root, MPI_COMM_WORLD);

    // printf("Processo =  %d, recebeu", rank);
    // print_array(local_array, self_partition_size);

    merge_sort(local_array, 0, self_partition_size - 1);
    // printf("Locally sorted array: ");
    // print_array(local_array, self_partition_size);

    sorted_array = malloc(sizeof(int) * N);
    MPI_Gatherv(local_array, self_partition_size, MPI_INT, sorted_array,
        sendcounts, displacement, MPI_INT, root, MPI_COMM_WORLD);

    if (rank == root) {
        int limit_tree = log2(n_procs);
        for (int proc = 1; proc < n_procs; proc++) {
            int middle = displacement[proc] - 1;
            int end = middle + sendcounts[proc];
            merge(sorted_array, 0, middle, end);
        }

        double end = MPI_Wtime();
        // puts("Sorted array:");
        // print_array(sorted_array, N);
        printf("Demorou %.4f segundos para ordenar %d valores\n", end - start, N);
    }

    /* Termina a execução */
    MPI_Finalize();
    return(0);
}
