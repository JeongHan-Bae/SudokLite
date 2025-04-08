# sd_solver_c.pxd
cdef extern from "sd_c_api.h":
    const char *sudoku_solver_c(signed char *puzzle, unsigned long long size)
