// author: Danila "akshin_" Axyonov

#include <cstdio>
#include <mpi.h>

int main(int argc, char** argv) {
    const int TAG = 4;
    MPI_Status status;

    MPI_Init(&argc, &argv);

    int rank,
        size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int next = (rank + 1) % size;
    MPI_Send(&rank, 1, MPI_INT, next, TAG, MPI_COMM_WORLD);

    int prev = (rank - 1 + size) % size,
        n;
    MPI_Recv(&n, 1, MPI_INT, prev, TAG, MPI_COMM_WORLD, &status);
    printf("rank = %d | received n = %d\n", rank, n);

    /*
    Вариант 2:
    MPI_Sendrecv(
        &rank, 1, MPI_INT, next, TAG,
        &n,    1, MPI_INT, prev, TAG,
        MPI_COMM_WORLD, &status
    );
    */

    MPI_Finalize();
    return 0;
}
