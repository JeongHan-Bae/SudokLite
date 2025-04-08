#ifndef SD_C_API_H
#define SD_C_API_H

#ifdef __cplusplus
#include "sudok_solver.hpp"
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

    /**
     * @brief Solve a standard 9x9 Sudoku puzzle.
     *
     * @param puzzle Pointer to an array of 81 int8_t values (row-major).
     *        - 0 indicates an empty cell
     *        - 1~9 are the given digits
     *        - The solved puzzle will be written back into the array (if solvable)
     * @param size Must be 81 (strictly validated)
     *
     * @return One of the following constant strings:
     *         - "Solved"
     *         - "Invalid puzzle"
     *         - "Invalid size" || "Size mismatch with declared size"
     *         - "No solution found"
     */
    static inline const char *sudoku_solver_c(int8_t *puzzle, uint64_t size){
        return sd::sudoku_solver(puzzle, size);
    }

#ifdef __cplusplus
}
#endif

#endif // SD_C_API_H
