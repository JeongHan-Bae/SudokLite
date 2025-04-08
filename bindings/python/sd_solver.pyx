# cython: language_level=3
import numpy as np
cimport numpy as np
from sd_solver_c cimport sudoku_solver_c

def solve(np.ndarray[np.int8_t, ndim=1, mode="c"] puzzle):
    if puzzle.size < 81:
        raise ValueError("puzzle must have at least 81 elements")

    cdef signed char *ptr = <signed char *> puzzle.data
    cdef const char *result = sudoku_solver_c(ptr, 81)

    msg = result.decode("utf-8") if hasattr(result, 'decode') else result
    if msg != "Solved":
        raise RuntimeError(f"Sudoku solver failed: {msg}")

    return puzzle[:81].reshape((9, 9)).copy()
