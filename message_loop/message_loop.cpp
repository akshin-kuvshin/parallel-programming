// author: Danila "akshin_" Axyonov

#include <iostream>
#include <mpi.h>

int main(int argc, char** argv) {
    const int TAG = 4;
    MPI_Status status;

    MPI_Init(&argc, &argv);

    int rank,
        size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    int prev = (rank - 1 + size) % size,
        next = (rank + 1) % size;

    int a = 0;

    if (rank != 0) {
        MPI_Recv(&a, 1, MPI_INT, prev, TAG, MPI_COMM_WORLD, &status);
        std::cout << "received " << prev << " -> " << rank << std::endl;
    }

    a += rank;
    MPI_Send(&a, 1, MPI_INT, next, TAG, MPI_COMM_WORLD);
    std::cout << "sent     " << rank << " -> " << next << std::endl;

    if (rank == 0) {
        MPI_Recv(&a, 1, MPI_INT, prev, TAG, MPI_COMM_WORLD, &status);
        std::cout << "received " << prev << " -> " << rank << std::endl;
    }

    // waiting for all processes to reach this point (to output the value of (a) at the very end)
    MPI_Barrier(MPI_COMM_WORLD);

    if (rank == 0)
        std::cout << "--------------------" << std::endl
                  << "a = " << a << std::endl;

    MPI_Finalize();
    return 0;
}
