# sd_solver_c.pxd
cdef extern from "sd_c_api.h":
    ctypedef struct sudoku_puzzle_t:
        signed char data[81]

    const char *sudoku_solver_c(sudoku_puzzle_t *puzzle)
