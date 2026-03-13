// author: Danila "akshin_" Axyonov

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <cmath>
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

        long double pi = acosl(-1.L);
        std::stringstream ss;
        ss << std::fixed << std::setprecision(size - 1) << pi;
        auto /* string */ pi_s = ss.str();
        pi_s.erase(1, 1); // удаляем десятичный разделитель (точку)

        for (int i = 0; i < size; ++i) {
            arr[i] = pi_s[i] - '0';
        }

	for (int i = 1, tmp; i < size; ++i)
            MPI_Send(arr + i, 1, MPI_INT, i, TAG, MPI_COMM_WORLD);
    } else {
        int num;
        MPI_Recv(&num, 1, MPI_INT, ROOT, TAG, MPI_COMM_WORLD, &status);
        printf("rank = %d, num = %d\n", rank, num);
    }

    MPI_Finalize();
    return 0;
}
