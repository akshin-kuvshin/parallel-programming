// author: Danila "akshin_" Axyonov

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <new>
#include <mpi.h>

#define ROOT 0
#define MEMORY_ALLOCATION_ERROR_CODE 1

int randint(int l, int r);

int main(int argc, char** argv) {
    const int TAG = 4;
    MPI_Status status;

    srand(time(nullptr));
    MPI_Init(&argc, &argv);

    int rank,
        size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int* arr;

    if (rank == ROOT) {
        int arr_size = (size - 1) * size / 2;
        try {
            arr = new int[arr_size];
        } catch (const std::bad_alloc& e) {
            fprintf(stderr, "[MEMORY ALLOCATION ERROR]: %s\n", e.what());
            MPI_Abort(MPI_COMM_WORLD, MEMORY_ALLOCATION_ERROR_CODE);
        }
        for (int i = 0; i < arr_size; ++i)
            arr[i] = randint(-9, 9);

        int sent_size = 0;
        for (int i = 0; i < size; ++i) {
            if (i == ROOT)
                continue;

            MPI_Send(arr + sent_size, i, MPI_INT, i, TAG, MPI_COMM_WORLD);
            sent_size += i;
        }
    } else { // rank != ROOT
        try {
            arr = new int[rank];
        } catch (const std::bad_alloc& e) {
            fprintf(stderr, "[MEMORY ALLOCATION ERROR]: %s\n", e.what());
            MPI_Abort(MPI_COMM_WORLD, MEMORY_ALLOCATION_ERROR_CODE);
        }

        MPI_Recv(arr, rank, MPI_INT, ROOT, TAG, MPI_COMM_WORLD, &status);

        printf("rank = %d | received arr = [%d", rank, arr[0]);
        for (int i = 1; i < rank; ++i)
            printf(" %d", arr[i]);
        printf("]\n");
    }

    delete[] arr;
    MPI_Finalize();

    return 0;
}

int randint(int l, int r) {
    return l + rand() % (r - l + 1);
}
