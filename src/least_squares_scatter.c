#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include "mpi.h"

int main(int argc, char **argv) {

    double *x, *y, *x_partial, *y_partial, time_start, time_end;
    double mySUMx, mySUMy, mySUMxy, mySUMxx, SUMx, SUMy, SUMxy,
            SUMxx, SUMres, res, slope, y_intercept, y_estimate;
    int i,j,n,myid,numprocs,naverage,nremain,mypoints;
    int *sendcounts, *displs;
    int root = 0;
    /*int new_sleep (int seconds);*/
    MPI_Status istatus;
    FILE *infile;

    infile = fopen("xydata", "r");
    if (infile == NULL) printf("error opening file\n");

    MPI_Init(&argc, &argv);
    MPI_Comm_rank (MPI_COMM_WORLD, &myid);
    MPI_Comm_size (MPI_COMM_WORLD, &numprocs);

    /* ----------------------------------------------------------
    * Step 1: Process 0 reads data and sends the value of n
    * ---------------------------------------------------------- */
    if (myid == 0) {
        // printf ("Number of processes used: %d\n", numprocs);
        // printf ("-------------------------------------\n");
        // printf ("The x coordinates on worker processes:\n");
        /* this call is used to achieve a consistent output format */
        /* new_sleep (3);*/
        fscanf (infile, "%d", &n);
        x = (double *) malloc (n*sizeof(double));
        y = (double *) malloc (n*sizeof(double));
        for (i=0; i<n; i++)
            fscanf (infile, "%lf %lf", &x[i], &y[i]);

        time_start = MPI_Wtime();
    }

    MPI_Bcast(&n, 1, MPI_INT, root, MPI_COMM_WORLD);

    /* ---------------------------------------------------------- */

    naverage = n/numprocs;
    nremain = n % numprocs;

    /* ----------------------------------------------------------
    * Step 2: Process 0 sends subsets of x and y
    !  * ---------------------------------------------------------- */
    if (myid == 0) {
        sendcounts = (int *) malloc (numprocs*sizeof(int));
        displs = (int *) malloc (numprocs*sizeof(int));
        for (i = 0; i < numprocs; i++) {
            sendcounts[i] = (i < numprocs -1) ? naverage : naverage + nremain;
            displs[i] = i*naverage;
        }
    }
    
    MPI_Scatter(sendcounts, 1, MPI_INT, &mypoints, 1, MPI_INT, root, MPI_COMM_WORLD);

    x_partial = (double *) malloc ((mypoints)*sizeof(double));
    y_partial = (double *) malloc ((mypoints)*sizeof(double));

    MPI_Scatterv(x, sendcounts, displs, MPI_DOUBLE, x_partial, mypoints, MPI_DOUBLE, root, MPI_COMM_WORLD);
    MPI_Scatterv(y, sendcounts, displs, MPI_DOUBLE, y_partial, mypoints, MPI_DOUBLE, root, MPI_COMM_WORLD);

    /* ----------------------------------------------------------
    * Step 3: Each process calculates its partial sum
    * ---------------------------------------------------------- */
    mySUMx = 0; mySUMy = 0; mySUMxy = 0; mySUMxx = 0;
    for (j=0; j<mypoints; j++) {
        mySUMx = mySUMx + x_partial[j];
        mySUMy = mySUMy + y_partial[j];
        mySUMxy = mySUMxy + x_partial[j]*y_partial[j];
        mySUMxx = mySUMxx + x_partial[j]*x_partial[j];
    }

    /* ----------------------------------------------------------
    * Step 4: Process 0 receives partial sums from the others
    * ---------------------------------------------------------- */
    MPI_Reduce(&mySUMx, &SUMx, 1, MPI_DOUBLE, MPI_SUM, root, MPI_COMM_WORLD);
    MPI_Reduce(&mySUMy, &SUMy, 1, MPI_DOUBLE, MPI_SUM, root, MPI_COMM_WORLD);
    MPI_Reduce(&mySUMxx, &SUMxx, 1, MPI_DOUBLE, MPI_SUM, root, MPI_COMM_WORLD);
    MPI_Reduce(&mySUMxy, &SUMxy, 1, MPI_DOUBLE, MPI_SUM, root, MPI_COMM_WORLD);
    

    /* ----------------------------------------------------------
    * Step 5: Process 0 does the final steps
    * ---------------------------------------------------------- */
    if (myid == 0) {
        slope = ( SUMx*SUMy - n*SUMxy ) / ( SUMx*SUMx - n*SUMxx );
        y_intercept = ( SUMy - slope*SUMx ) / n;

        SUMres = 0;
        for (i=0; i<n; i++) {
            y_estimate = slope*x[i] + y_intercept;
            res = y[i] - y_estimate;
            SUMres = SUMres + res*res;
        }

        time_end = MPI_Wtime();
        printf("Equation: y = %6.2lfx + %6.2lf\t", slope, y_intercept);
        printf("Residual sum = %6.2lf\n", SUMres);
        printf("Scatter version. %d proccesses. n = %d. Time elapsed proccesses: %1.3f\n", numprocs, n, time_end-time_start);
    }

    /* ----------------------------------------------------------	*/
    MPI_Finalize();
}
