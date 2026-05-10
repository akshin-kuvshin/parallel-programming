// author: Danila "akshin_" Axyonov

#include <omp.h>
#include <iostream>
#include <string>
#include <cmath>

double f(double x);

int main(int argc, char const* argv[]) {
    if (argc > 2) {
        std::cerr << "[ERROR]: too many arguments." << std::endl;
        return 1;
    }
    int threads = (argc == 2 ? std::stoi(argv[1]) : 1);
    omp_set_num_threads(threads);

    double a = 0.3,
           b = 0.8,
           ans = 0.0803291; // WolframAlpha

    int N = 1'000'000'000;
    double dx = (b - a) / (double)N;
    double dx3 = dx / 3.0;

    double res = 0.0,
           res_local = 0.0;

    double t_start,
           t_finish,
           dt;
    t_start = omp_get_wtime();
    #pragma omp parallel default(shared) private(res_local)
    {
        #pragma omp for schedule(static)
        for (int k = 1; k < N; k += 2) {
            double x1 = a + (double)(k - 1) * dx,
                   x2 = a + (double)k * dx,
                   x3 = a + (double)(k + 1) * dx;
            res_local += (f(x1) + 4.0 * f(x2) + f(x3)) * dx3;
        }
        res += res_local;
    }
    t_finish = omp_get_wtime();
    dt = t_finish - t_start;

    std::cout << "ans = " << ans << std::endl
              << "res = " << res << std::endl
              << "error = " << fabs(ans - res) << std::endl
              << "time = " << dt << " s." << std::endl;

    return 0;
}

double f(double x) {
    return exp(-1.46 * x * x) / (3.5 + sin(x));
}
