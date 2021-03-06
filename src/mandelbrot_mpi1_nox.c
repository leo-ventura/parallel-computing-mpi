/* Sequential Mandlebrot program */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "mpi.h"

#define X_RESN 1000 /* x resolution */
#define Y_RESN 1000 /* y resolution */
#define MAX_ITER (600)

typedef struct complextype {
    double real, imag;
} Compl;

int main(int argc, char *argv[])
{
    int rank, num_procs;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    /* Mandlebrot variables */

    int *counts = (int *)malloc(num_procs * sizeof(int));
    int *starts = (int *)malloc(num_procs * sizeof(int));
    for (int itP = 0; itP < num_procs; itP++)
    {
        starts[itP] = itP == 0 ? 0 : starts[itP - 1] + counts[itP - 1];
        int line_size = (X_RESN * Y_RESN) / num_procs;
        int line_inc = (itP < ((X_RESN * Y_RESN) % num_procs)) ? 1 : 0;
        counts[itP] = line_size + line_inc;
    }

    int my_start = starts[rank];
    int my_count = counts[rank];

    int *ks_loc;
    int *ks;
    ks_loc = (int *)malloc(my_count * sizeof(int));
    if (rank == 0)
        ks = (int *)malloc((X_RESN * Y_RESN) * sizeof(int));

    /* Calculate and draw points */
    for (int it = my_start; it < my_start + my_count; it++)
    {
        int i = it / Y_RESN;
        int j = it % Y_RESN;

        // mandelbrot set is defined in the region of x = [-2, +2] and y = [-2, +2]
        double u = ((double)i - (X_RESN / 2.0)) / (X_RESN / 4.0);
        double v = ((double)j - (Y_RESN / 2.0)) / (Y_RESN / 4.0);

        Compl z, c;

        z.real = z.imag = 0.0;
        c.real = v;
        c.imag = u;

        int k = 0;
        double d = 0.0;

        double lengthsq, temp;
        do
        { /* iterate for pixel color */
            temp = z.real * z.real - z.imag * z.imag + c.real;
            z.imag = 2.0 * z.real * z.imag + c.imag;
            z.real = temp;
            lengthsq = z.real * z.real + z.imag * z.imag;
            k++;
        } while (lengthsq < 4.0 && k < MAX_ITER);

        ks_loc[it - my_start] = k;
    }

    // concatenating all ks_loc arrays from each process into the ks array at process 0
    MPI_Gatherv(ks_loc, counts[rank], MPI_INT, ks, counts, starts, MPI_INT, 0, MPI_COMM_WORLD);

    free(ks_loc);
    free(counts);
    free(starts);

    free(ks);

    MPI_Finalize();

    /* Program Finished */
    return 0;
}
