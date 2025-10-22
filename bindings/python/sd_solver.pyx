# cython: language_level=3
import numpy as np
cimport numpy as np
from sd_solver_c cimport sudoku_solver_c, sudoku_puzzle_t

def solve(np.ndarray[np.int8_t, ndim=1, mode="c"] puzzle):
    """
    Solve a 9x9 Sudoku puzzle.

    Parameters
    ----------
    puzzle : np.ndarray[int8]
        1D array of length 81, where:
          - 0 = empty cell
          - 1~9 = given digits

    Returns
    -------
    np.ndarray[int8]
        Solved 9x9 Sudoku puzzle.
    """
    if puzzle.size < 81:
        raise ValueError("puzzle must have at least 81 elements")

    # --- copy puzzle data into the struct ---
    cdef sudoku_puzzle_t sp
    for i in range(81):
        sp.data[i] = <signed char> puzzle[i]

    # --- call the C API ---
    cdef const char *result = sudoku_solver_c(&sp)

    # --- interpret return message ---
    msg = result.decode("utf-8") if hasattr(result, "decode") else result
    if msg != "Solved":
        raise RuntimeError(f"Sudoku solver failed: {msg}")

    # --- convert back to numpy array ---
    solved = np.empty(81, dtype=np.int8)
    for i in range(81):
        solved[i] = sp.data[i]

    return solved.reshape((9, 9))
