// author: Danila "akshin_" Axyonov

#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <new>
#include <mpi.h>

#define ROOT 0
#define MEMORY_ALLOCATION_ERROR_CODE 1
#define TAG 4

int* int_alloc(int size);

int main(int argc, char** argv) {
    srand(time(nullptr));
    MPI_Init(&argc, &argv);
    std::ostringstream s;

    int rank,
        size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    const int n_global = (int)1e7;
    int* x_global;
    int* y_global;
    if (rank == ROOT) {
        x_global = int_alloc(n_global);
        y_global = int_alloc(n_global);
        for (int i = 0; i < n_global; ++i)
            x_global[i] = y_global[i] = 1;
    }

    int* n_locals = int_alloc(size);
    for (int i = 0; i < size; ++i)
        n_locals[i] = n_global / size + (i < n_global % size);
    int* displs = int_alloc(size);
    for (int i = 1; i < size; ++i)
        displs[i] = displs[i - 1] + n_locals[i - 1];

    int n_local = n_locals[rank];
    int* x_local = int_alloc(n_local);
    int* y_local = int_alloc(n_local);
    MPI_Scatterv(x_global, n_locals, displs, MPI_INT,
                 x_local,  n_local,          MPI_INT,
                 ROOT, MPI_COMM_WORLD);
    MPI_Scatterv(y_global, n_locals, displs, MPI_INT,
                 y_local,  n_local,          MPI_INT,
                 ROOT, MPI_COMM_WORLD);
    if (rank == ROOT) {
        delete[] x_global;
        delete[] y_global;
    }
    delete[] n_locals;
    delete[] displs;

    int sc_pr = 0;
    for (int i = 0; i < n_local; ++i)
        sc_pr += x_local[i] * y_local[i];
    delete[] x_local;
    delete[] y_local;

    s << "rank = " << rank << " | sc_pr = " << sc_pr << std::endl;
    std::cout << s.str();
    s.str("");

    MPI_Barrier(MPI_COMM_WORLD);
    if (rank == ROOT)
        std::cout << "----------------------------------------" << std::endl;
    MPI_Barrier(MPI_COMM_WORLD);

    int log2size;
    for (log2size = 0; (1 << log2size) < size; ++log2size);

    int groups = (size + log2size - 1) / log2size;
    int* groups_sizes = int_alloc(groups);
    for (int i = 0; i < groups; ++i)
        groups_sizes[i] = log2size;
    groups_sizes[groups - 1] = size - (groups - 1) * log2size;
    int* groups_leads = int_alloc(groups);
    groups_leads[0] = groups_sizes[0] - 1;
    for (int i = 1; i < groups; ++i)
        groups_leads[i] = groups_leads[i - 1] + groups_sizes[i];

    int group = rank / log2size;
    int group_size = groups_sizes[group],
        pos_in_group = rank % log2size;
    if (pos_in_group != 0) {
        int recv;
        MPI_Recv(&recv, 1, MPI_INT, rank - 1, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        sc_pr += recv;
    }
    if (pos_in_group != group_size - 1)
        MPI_Send(&sc_pr, 1, MPI_INT, rank + 1, TAG, MPI_COMM_WORLD);

    if (pos_in_group == group_size - 1) {
        s << "group = " << group << " (lead rank = " << rank << ") | sc_pr in group = " << sc_pr << std::endl;
        std::cout << s.str();
        s.str("");
    }

    MPI_Barrier(MPI_COMM_WORLD);
    if (rank == ROOT)
        std::cout << "----------------------------------------" << std::endl;
    MPI_Barrier(MPI_COMM_WORLD);

    for (int i0 = 0, step = 1; i0 < groups;) {
        for (int i = i0; i < groups; i += 2 * step) {
            int from = groups_leads[i],
                to = groups_leads[std::min(i + step, groups - 1)];
            if (rank == from)
                MPI_Send(&sc_pr, 1, MPI_INT, to, TAG, MPI_COMM_WORLD);
            else if (rank == to) {
                int recv;
                MPI_Recv(&recv, 1, MPI_INT, from, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                sc_pr += recv;

                s << "recv " << from << " -> " << to << " | +" << recv << " | new sc_pr = " << sc_pr << std::endl;
                std::cout << s.str();
                s.str("");
            }
        }

        i0 += step;
        step *= 2;
    }

    MPI_Barrier(MPI_COMM_WORLD);

    if (rank == size - 1)
        std::cout << "----------------------------------------" << std::endl
                  << "FINAL RESULT: (x, y) = " << sc_pr << std::endl;

    MPI_Finalize();
    return 0;
}

int* int_alloc(int size) {
    try {
        return new int[size]{};
    } catch (const std::bad_alloc& e) {
        std::cerr << "[MEMORY ALLOCATION ERROR]: " << e.what() << std::endl;
        MPI_Abort(MPI_COMM_WORLD, MEMORY_ALLOCATION_ERROR_CODE);
    }
}
