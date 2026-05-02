// author: Danila "akshin_" Axyonov

#include <iostream>
#include <iomanip>
#include <sstream>
#include <cmath>
#include <mpi.h>

#define ROOT 0

double f(double x, double y);

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    std::ostringstream s;

    int rank,
        size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    const double INF = 1e12,
                 ANS = 8.0 / 45.0, // посчитано вручную
                 S_TOTAL = 1.0;
    double S1 = S_TOTAL / (double)size;

    double* ys = new double[size + 1];
    ys[0] = 0.0;
    for (int i = 0; i < size; ++i)
        ys[i + 1] = sqrt(S1 + ys[i] * ys[i]);
    if (rank == ROOT) {
        const int NUMBERS_IN_ROW = 4;

        s << "ys = [" << ys[0];
        for (int i = 1; i <= size; ++i) {
            if (i % NUMBERS_IN_ROW == 0)
                s << std::endl << "     ";
            s << ' ' << ys[i];
        }
        s << ']' << std::endl;
        std::cout << s.str();
        s.str("");
    }

    const int MAX_ITERATIONS = 7,
              PRECISION = 6;
    int N = 1'000,
        k = 0;
    double d,
           S0,
           res_local,
           res,
           error;
    double t_start,
           t_finish,
           dt,
           dt_min,
           dt_max,
           dt_avg;
    do {
        d = 1.0 / (double)N;
        S0 = d * d;
        res_local = res = 0.0;

        if (rank == ROOT) {
            if (k > 0) s << "------------------------------" << std::endl;
            s << "N = " << N << ':' << std::endl;
            std::cout << s.str();
            s.str("");
        }

t_start = MPI_Wtime();
        for (double y = ys[rank]; y < ys[rank + 1]; y += d) {
            double xl = -y,
                   xr = y;
            for (double x = xl; x < xr; x += d)
                res_local += f(x, y) * S0;
        }
t_finish = MPI_Wtime();
dt = t_finish - t_start;

        dt_min = INF; dt_max = -INF; dt_avg = 0.0;
        MPI_Allreduce(&dt,
                      &dt_min,
                      1,
                      MPI_DOUBLE,
                      MPI_MIN,
                      MPI_COMM_WORLD);
        MPI_Allreduce(&dt,
                      &dt_max,
                      1,
                      MPI_DOUBLE,
                      MPI_MAX,
                      MPI_COMM_WORLD);
        MPI_Allreduce(&dt,
                      &dt_avg,
                      1,
                      MPI_DOUBLE,
                      MPI_SUM,
                      MPI_COMM_WORLD);
        dt_avg /= (double)size;

        MPI_Allreduce(&res_local,
                      &res,
                      1,
                      MPI_DOUBLE,
                      MPI_SUM,
                      MPI_COMM_WORLD);
        error = fabs(res - ANS);

        if (rank == ROOT) {
            s << "res = " << std::fixed << std::setprecision(PRECISION)
              << res << " | error = " << std::fixed
              << std::setprecision(PRECISION) << error << std::endl
              << "dt_min = " << std::fixed << std::setprecision(PRECISION)
              << dt_min << " | dt_max = " << std::fixed
              << std::setprecision(PRECISION) << dt_max << std::endl
              << "dt_avg = " << std::fixed << std::setprecision(PRECISION)
              << dt_avg << std::endl;
            std::cout << s.str();
            s.str("");
        }
        MPI_Barrier(MPI_COMM_WORLD);

        N *= 2;
        ++k;
    } while (k < MAX_ITERATIONS);

    MPI_Finalize();
    return 0;
}

double f(double x, double y) {
    double half = x * x - y * y;
    return half * half;
}
