// author: Danila "akshin_" Axyonov

#include <iostream>
#include <mpi.h>

#define ROOT 0
#define MEMORY_ALLOCATION_ERROR_CODE 1

int main(int argc, char** argv) {
    const int TAG = 4;
    MPI_Status status;

    MPI_Init(&argc, &argv);

    int rank,
        size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == ROOT) {
        int* arr = new int[size];
        if (not arr) {
            std::cerr << "[ERROR]: couldn't allocate memory." << std::endl;
            MPI_Abort(MPI_COMM_WORLD, MEMORY_ALLOCATION_ERROR_CODE);
        }

        arr[ROOT] = ROOT;
	for (int i = 1, tmp; i < size; ++i)
            MPI_Recv(arr + i, 1, MPI_INT, i, TAG, MPI_COMM_WORLD, &status);

        std::cout << "arr = [" << arr[0];
        for (int i = 1; i < size; ++i)
            std::cout << ' ' << arr[i];
        std::cout << ']' << std::endl;

        delete[] arr;
    } else
        MPI_Send(&rank, 1, MPI_INT, ROOT, TAG, MPI_COMM_WORLD);

    MPI_Finalize();
    return 0;
}
