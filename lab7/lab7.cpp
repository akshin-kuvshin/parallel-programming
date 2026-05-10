// author: Danila "akshin_" Axyonov

#include <omp.h>
#include <iostream>
#include <string>
#include <cstdlib>

int** allocate_matrix(int n, int m);
void delete_matrix(int** matrix, int n);
int randint(int l, int r);

int main(int argc, char const* argv[]) {
    if (argc > 2) {
        std::cout << "[ERROR]: too many arguments." << std::endl;
        return 1;
    }
    int threads = (argc == 2 ? std::stoi(argv[1]) : 1);
    omp_set_num_threads(threads);

    const int N = 1'000;
    int** a = allocate_matrix(N, N);
    int** b = allocate_matrix(N, N);
    int** c = allocate_matrix(N, N);

    srand(time(nullptr));
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j) {
            a[i][j] = randint(-N, N);
            b[i][j] = randint(-N, N);
        }

    double t_start,
           t_finish,
           dt;
    t_start = omp_get_wtime();
    #pragma omp parallel default(shared)
    {
        #pragma omp for collapse(3) schedule(static)
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
                for (int k = 0; k < N; ++k)
                    c[i][j] += a[i][k] * b[k][j];
    }
    t_finish = omp_get_wtime();
    dt = t_finish - t_start;
    std::cout << "time spent for i-j-k cycle: " << dt << " s." << std::endl;

    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
            c[i][j] = 0;

    t_start = omp_get_wtime();
    #pragma omp parallel default(shared)
    {
        #pragma omp for collapse(3) schedule(static)
        for (int k = 0; k < N; ++k)
            for (int i = 0; i < N; ++i)
                for (int j = 0; j < N; ++j)
                    c[i][j] += a[i][k] * b[k][j];
    }
    t_finish = omp_get_wtime();
    dt = t_finish - t_start;
    std::cout << "time spent for k-i-j cycle: " << dt << " s." << std::endl;

    delete_matrix(a, N);
    delete_matrix(b, N);
    delete_matrix(c, N);
    return 0;
}

int** allocate_matrix(int n, int m) {
    int** matrix = new int*[n];
    for (int i = 0; i < n; ++i)
        matrix[i] = new int[m]{};
    return matrix;
}

void delete_matrix(int** matrix, int n) {
    for (int i = 0; i < n; ++i)
        delete[] matrix[i];
    delete[] matrix;
}

int randint(int l, int r) {
    return l + rand() % (r - l + 1);
}
