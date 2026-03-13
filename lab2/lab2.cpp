// author: Danila "akshin_" Axyonov

#include <iostream>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <new>
#include <mpi.h>

#define ROOT1 1
#define ROOT2 0
#define MEMORY_ALLOCATION_ERROR_CODE 1

int randint(int l, int r);

int main(int argc, char** argv) {
    srand(time(nullptr));
    MPI_Init(&argc, &argv);

    int rank,
        size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    const int m = 5;
    int* x_global;
    int x_global_size = m * size;

    if (rank == ROOT1) {
        try {
            x_global = new int[x_global_size];
        } catch (const std::bad_alloc& e) {
            std::cerr << "[MEMORY ALLOCATION ERROR]: " << e.what() << std::endl;
            MPI_Abort(MPI_COMM_WORLD, MEMORY_ALLOCATION_ERROR_CODE);
        }

        for (int i = 0; i < x_global_size; ++i)
            x_global[i] = randint(0, 9);

        std::cout << "X_GLOBAL = [";
        for (int i = 0; i < x_global_size; ++i) {
            if (i % m == 0)
                std::cout << std::endl << "    "; // 4 spaces
            std::cout << x_global[i] << ' ';
        }
        std::cout << std::endl << ']' << std::endl;
    }

    int x_local[m];
    MPI_Scatter(x_global, m, MPI_INT,
                x_local,  m, MPI_INT,
                ROOT1, MPI_COMM_WORLD);

    int sum_local = 0;
    for (int i = 0; i < m; ++i)
        sum_local += x_local[i];

    std::stringstream s;
    s << "rank = " << rank << " | x_local = [" << x_local[0];
    for (int i = 1; i < m; ++i)
        s << ' ' << x_local[i];
    s << "] | sum_local = " << sum_local << std::endl;
    std::cout << s.str();

    int sum_global = 0;
    MPI_Reduce(&sum_local, &sum_global, 1, MPI_INT, MPI_SUM, ROOT2, MPI_COMM_WORLD);

    if (rank == ROOT1)
        delete[] x_global;
    else if (rank == ROOT2)
        std::cout << "SUM_GLOBAL = " << sum_global << std::endl;

    MPI_Finalize();
    return 0;
}

int randint(int l, int r) {
    return l + rand() % (r - l + 1);
}
