// author: Danila "akshin_" Axyonov

#include <cstdio>
#include <new>
#include <mpi.h>

#define MEMORY_ALLOCATION_ERROR_CODE 1

int main(int argc, char** argv) {
    const int TAG = 4;
    MPI_Status status;

    MPI_Init(&argc, &argv);

    int rank,
        size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int* arr;
    try {
        arr = new int[size];
    } catch (const std::bad_alloc& e) {
        fprintf(stderr, "[MEMORY ALLOCATION ERROR]: %s\n", e.what());
        MPI_Abort(MPI_COMM_WORLD, MEMORY_ALLOCATION_ERROR_CODE);
    }

    arr[rank] = rank;
    for (int i = 0; i < size; ++i) {
        if (i == rank)
            continue;
        MPI_Send(&rank, 1, MPI_INT, i, TAG, MPI_COMM_WORLD);
    }
    for (int i = 0; i < size; ++i) {
        if (i == rank)
            continue;
        MPI_Recv(arr + i, 1, MPI_INT, i, TAG, MPI_COMM_WORLD, &status);
    }

    printf("rank = %d | arr = [%d", rank, arr[0]);
    for (int i = 1; i < size; ++i)
        printf(" %d", arr[i]);
    printf("]\n");

    delete[] arr;
    MPI_Finalize();

    return 0;
}
