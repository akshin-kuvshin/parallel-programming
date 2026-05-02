// author: Danila "akshin_" Axyonov

#include <iostream>
#include <iomanip>
#include <sstream>
#include <cmath>
#include <mpi.h>

#define ROOT 0

double f(double x);

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    std::ostringstream s;

    int rank,
        size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    double t_start,
           t_finish,
           dt;
    t_start = MPI_Wtime();

    const int N0 = 1'000'000,
              PRECISION = 10,
              MAX_ITERATIONS = 10;
    const double A_GLOBAL = 1.,
                 B_GLOBAL = 1.2,
                 ANS = 0.1376279, // https://mathdf.com/int/ru/
                 EPS = 1e-8;

    double a_local = A_GLOBAL + (B_GLOBAL - A_GLOBAL) / (double)size * (double)rank,
           b_local = A_GLOBAL + (B_GLOBAL - A_GLOBAL) / (double)size * ((double)rank + 1.),
           res_local,
           res;
    int n_local = N0 / size,
        k = 0;
    double h;
    do {
        MPI_Barrier(MPI_COMM_WORLD);
        if (rank == ROOT)
            std::cout << "\t[n_local = " << n_local << ']' << std::endl;
        MPI_Barrier(MPI_COMM_WORLD);

        res_local = res = 0.;
        h = (b_local - a_local) / (double)n_local;

        double x = a_local;
        for (int i = 0; i < n_local; x += h, ++i)
            res_local += f(x + h * .5);
        res_local *= h;

        s << "rank = " << rank << " | a_local = " << std::fixed
          << std::setprecision(PRECISION) << a_local << " | b_local = "
          << std::fixed << std::setprecision(PRECISION) << b_local
          << " | res_local = " << std::fixed << std::setprecision(PRECISION)
          << res_local << std::endl;
        std::cout << s.str();
        s.str("");

        MPI_Allreduce(&res_local, &res, 1,
                      MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

        MPI_Barrier(MPI_COMM_WORLD);
        if (rank == ROOT)
            std::cout << "\t[res = " << std::fixed << std::setprecision(PRECISION)
                      << res << " | error = " << std::fixed
                      << std::setprecision(PRECISION) << fabs(res - ANS)
                      << ']' << std::endl
                      << "--------------------------------------------------"
                      << std::endl;
        MPI_Barrier(MPI_COMM_WORLD);

        n_local *= 2;
        ++k;
    } while (fabs(res - ANS) > EPS and k < MAX_ITERATIONS);

    t_finish = MPI_Wtime();
    dt = t_finish - t_start;

    MPI_Barrier(MPI_COMM_WORLD);
    if (rank == ROOT)
        std::cout << "\t[execution time = " << std::fixed
                  << std::setprecision(PRECISION) << dt << " s.]" << std::endl;

    MPI_Finalize();
    return 0;
}

double f(double x) {
    return sqrt(x * (3. - x)) / (x + 1.);
}
