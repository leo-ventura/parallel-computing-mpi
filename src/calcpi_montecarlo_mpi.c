#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#define N 1000000000
#define CHUNK_SIZE 50000

int is_inside_circle(double x, double y) {
    return (x*x + y*y) <= 1;
}

int main(int argc, char *argv[]) {
    double t_inicial, t_final;
    int cont = 0, total = 0;
    int i;
    int meu_ranque, num_procs, inicio, dest, raiz=0, tag=1, stop=0;
    long long int count = 0;
    long long int count_total = 0;
    MPI_Status estado;
    MPI_Request r_envia;
    MPI_Request r_recebe;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &meu_ranque);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    /* Registra o tempo inicial de execução do programa */
    t_inicial = MPI_Wtime();
    /* Envia pedaços com CHUNK_SIZE números para cada processo */
    estado.MPI_TAG = 1;
    if (meu_ranque == 0) { 
        for (dest=1, inicio=1; dest < num_procs && inicio < N; dest++, inicio += CHUNK_SIZE) {
            MPI_Send(&inicio, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
        }

        stop = num_procs - 1;
        /* Fica recebendo as contagens parciais de cada processo */
        while (stop) {
            MPI_Recv(&count, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &estado);

            count_total += count;
            dest = estado.MPI_SOURCE;
            if (inicio > N) {
                tag = 99;
                stop--;
            }
/* Envia um nvo pedaço com CHUNK_SIZE números para o mesmo processo*/
            MPI_Send(&inicio, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
            inicio += CHUNK_SIZE;
        }
    }
    else { 
        /* Cada processo escravo recebe o início do espaço de busca */
        while (estado.MPI_TAG != 99) {
            MPI_Recv(&inicio, 1, MPI_INT, raiz, MPI_ANY_TAG, MPI_COMM_WORLD, &estado);

            if (estado.MPI_TAG != 99) {
                for (i = inicio, count = 0; i < (inicio + CHUNK_SIZE) && i < N; i++) {
                    double x = (double) rand() / RAND_MAX;
                    double y = (double) rand() / RAND_MAX;

                    if (is_inside_circle(x, y))
                        count++;
                }

                /* Envia a contagem parcial para o processo mestre */
                MPI_Send(&count, 1, MPI_INT, raiz, tag, MPI_COMM_WORLD);
            }
        }
    }
    /* Registra o tempo final de execução */
    t_final = MPI_Wtime();
    if (meu_ranque == 0) {
        t_final = MPI_Wtime();
        printf("Valor final de pi: %f\n", (double) count_total / N * 4);
        printf("Tempo de execucao: %1.3f \n", t_final - t_inicial);
    }
/* Finaliza o programa */
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Finalize();
	return(0);
}