// author: Danila "akshin_" Axyonov

#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <new>
#include <cmath>
#include <mpi.h>

#define ROOT 0
#define MEMORY_ALLOCATION_ERROR_CODE 1

void create_matrix(int** matrix_flat, int*** matrix, int n, int m);
void fill_matrix(int** matrix, int n, int m, int min_value, int max_value);
void print_matrix(std::ostream& out, int** matrix, int n, int m, const std::string& label);
void delete_matrix(int* matrix_flat, int** matrix);
int randint(int l, int r);

int main(int argc, char** argv) {
    srand(time(nullptr));
    MPI_Init(&argc, &argv);

    int rank,
        size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    const int m = 4;
    int n = m * size;
    int*  A_flat;
    int** A;
    int*  B_flat;
    int** B;

    if (rank == ROOT) {
        create_matrix(&A_flat, &A, n, m);
        fill_matrix(A, n, m, -9, 9);
        print_matrix(std::cout, A, n, m, "A");

        create_matrix(&B_flat, &B, n, m);
        fill_matrix(B, n, m, -9, 9);
        print_matrix(std::cout, B, n, m, "B");
    }

    int*  A_local_flat;
    int** A_local;
    create_matrix(&A_local_flat, &A_local, m, m);

    int*  B_local_flat;
    int** B_local;
    create_matrix(&B_local_flat, &B_local, m, m);

    MPI_Scatter(A_flat,       m * m, MPI_INT,
                A_local_flat, m * m, MPI_INT,
                ROOT, MPI_COMM_WORLD);
    MPI_Scatter(B_flat,       m * m, MPI_INT,
                B_local_flat, m * m, MPI_INT,
                ROOT, MPI_COMM_WORLD);

    int sum_local = 0;
    for (int i = 0; i < m; ++i)
        for (int j = 0; j < m; ++j)
            sum_local += (A_local[i][j] - B_local[i][j]) * (A_local[i][j] - B_local[i][j]);

    std::ostringstream s;
    s << "rank = " << rank << std::endl;
    print_matrix(s, A_local, m, m, "A_local");
    print_matrix(s, B_local, m, m, "B_local");
    s << "sum_local = " << sum_local << std::endl;
    std::cout << s.str();

    MPI_Barrier(MPI_COMM_WORLD);

    int sum_global = 0;
    MPI_Allreduce(&sum_local, &sum_global, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    double norm = sqrt(sum_global);

    s.str("");
    s << "rank = " << rank << " | sum_global = " << sum_global << " | norm = " << norm << std::endl;
    std::cout << s.str();

    if (rank == ROOT) {
        delete_matrix(A_flat, A);
        delete_matrix(B_flat, B);
    }
    delete_matrix(A_local_flat, A_local);
    delete_matrix(B_local_flat, B_local);

    MPI_Finalize();
    return 0;
}

void create_matrix(int** matrix_flat, int*** matrix, int n, int m) {
    try {
        *matrix_flat = new int[n * m];
        *matrix = new int*[n];
        for (int i = 0; i < n; ++i)
            (*matrix)[i] = *matrix_flat + i * m;
    } catch (const std::bad_alloc& e) {
        std::cerr << "[MEMORY ALLOCATION ERROR]: " << e.what() << std::endl;
        MPI_Abort(MPI_COMM_WORLD, MEMORY_ALLOCATION_ERROR_CODE);
    }
}

void fill_matrix(int** matrix, int n, int m, int min_value, int max_value) {
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < m; ++j)
            matrix[i][j] = randint(min_value, max_value);
}

void print_matrix(std::ostream& out, int** matrix, int n, int m, const std::string& label) {
    out << label << " = [";
    for (int i = 0; i < n; ++i) {
        out << std::endl << "    "; // 4 spaces
        for (int j = 0; j < m; ++j)
            out << matrix[i][j] << ' ';
    }
    out << std::endl << ']' << std::endl;
}

void delete_matrix(int* matrix_flat, int** matrix) {
    delete[] matrix_flat;
    delete[] matrix;
}

int randint(int l, int r) {
    return l + rand() % (r - l + 1);
}
