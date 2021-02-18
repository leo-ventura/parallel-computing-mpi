/* Sequential Mandlebrot program */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "mpi.h"

#define X_RESN 1000 /* x resolution */
#define Y_RESN 1000 /* y resolution */
#define MAX_ITER (2000)

typedef struct complextype {
    double real, imag;
} Compl;

int main(int argc, char *argv[])
{
    int rank, num_procs;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    double start, finish;
    if (rank == 0)
        start = MPI_Wtime();

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
    int *ks = NULL;
    ks_loc = (int *)malloc(my_count * sizeof(int));
    if (rank == 0)
        ks = (int *)malloc((X_RESN * Y_RESN) * sizeof(int));

    double *ds_loc;
    double *ds = NULL;
    ds_loc = (double *)malloc(my_count * sizeof(double));
    if (rank == 0)
        ds = (double *)malloc((X_RESN * Y_RESN) * sizeof(double));

    /* Calculate and draw points */
    for (int it = 0; it < my_count; it++)
    {
        int pixel_idx = rank + it*num_procs;
        int i = pixel_idx / Y_RESN;
        int j = pixel_idx % Y_RESN;

        // mandelbrot set is defined in the region of x = [-2, +2] and y = [-2, +2]
        double u = ((double)i - (X_RESN / 2.0)) / (X_RESN / 4.0);
        double v = ((double)j - (Y_RESN / 2.0)) / (Y_RESN / 4.0);

        Compl z, c, t;

        z.real = z.imag = 0.0;
        c.real = v;
        c.imag = u;

        int k = 0;
        double d = 0.0;

        double lengthsq, temp;
        do
        { /* iterate for pixel color */
            t = z;
            z.imag = 2.0 * t.real * t.imag + c.imag;
            z.real = t.real * t.real - t.imag * t.imag + c.real;
            lengthsq = z.real * z.real + z.imag * z.imag;
            d += pow(pow(z.imag - t.imag, 2.0) + pow(z.real - t.real, 2.0), 0.5);
            k++;
        } while (lengthsq < 4.0 && k < MAX_ITER);

        ks_loc[it] = k;
        ds_loc[it] = d;
    }

    // concatenating all ks_loc arrays from each process into the ks array at process 0
    MPI_Gatherv(ks_loc, counts[rank], MPI_INT, ks, counts, starts, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Gatherv(ds_loc, counts[rank], MPI_DOUBLE, ds, counts, starts, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    free(ks_loc);
    free(ds_loc);
    free(counts);
    if (rank != 0)
        free(starts);

    if (rank == 0)
    {
        free(starts);

        free(ks);
        free(ds);
    
        finish = MPI_Wtime();
        double elapsed = finish - start;
        printf("%f\n", elapsed);
    }

    MPI_Finalize();

    /* Program Finished */
    return 0;
}
