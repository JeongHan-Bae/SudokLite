#ifndef SD_C_API_H
#define SD_C_API_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
#include "sudok_solver.hpp"
extern "C" {
#endif

    /**
     * <h3>Structure: sudoku_puzzle_t</h3>
     *
     * <p><b>Description:</b><br/>
     * Represents a fixed-size Sudoku puzzle buffer used for safe inter-language
     * communication between C++ and C-based FFI environments. This structure defines
     * the canonical binary layout for a 9×9 Sudoku grid, preventing the caller from
     * providing incorrect buffer sizes or forged memory layouts.</p>
     *
     * <p><b>Fields:</b></p>
     * <ul>
     *   <li><b><code>data</code></b> &nbsp;&mdash;&nbsp;
     *       A contiguous array of <code>81</code> signed 8-bit integers
     *       (<code>int8_t</code>), stored in row-major order.
     *       Each value corresponds to one Sudoku cell.</li>
     * </ul>
     *
     * <p><b>Encoding:</b></p>
     * <ul>
     *   <li><b>0</b> &nbsp;&mdash;&nbsp; indicates an empty cell.</li>
     *   <li><b>1&nbsp;&ndash;&nbsp;9</b> &nbsp;&mdash;&nbsp; represent given digits.</li>
     * </ul>
     *
     * <p><b>Purpose:</b><br/>
     * This structure enforces ABI stability and type-level size constraints.
     * It eliminates the need for a separate <code>size</code> parameter, ensuring
     * that the caller cannot falsify buffer length information. All API calls
     * using this type must pass the address of a properly initialized
     * <code>sudoku_puzzle_t</code> object.</p>
     *
     * <p><b>Size guarantee:</b><br/>
     * The structure has a fixed size of exactly 81 bytes on all standard platforms
     * (subject to <code>int8_t</code> definition), ensuring binary compatibility
     * across C++, C, and foreign-function interfaces (FFI) such as Python, Rust,
     * or C# bindings.</p>
     */
    typedef struct {
        int8_t data[81];
    } sudoku_puzzle_t;

    /**
     * <h3>Function: sudoku_solver_c</h3>
     *
     * <p><b>Description:</b><br/>
     * Solve a standard 9×9 Sudoku puzzle using an optimized C++ backtracking
     * algorithm, exposed through a stable C ABI. The function is designed for
     * direct use by C-based FFI environments (for example: Python, Rust, Zig,
     * or C# interop layers). The solver operates <i>in-place</i> on the provided
     * puzzle buffer.</p>
     *
     * <p><b>Parameters:</b></p>
     * <ul>
     *   <li><b><code>puzzle</code></b> &nbsp;&mdash;&nbsp;
     *       Pointer to a valid <code>sudoku_puzzle_t</code> instance.
     *       The structure must contain exactly 81 cells representing
     *       a Sudoku grid configuration.</li>
     * </ul>
     *
     * <p><b>Behavior:</b></p>
     * <ul>
     *   <li>If <code>puzzle</code> is <code>NULL</code>, the function immediately
     *       returns the string <code>"Null pointer"</code>.</li>
     *   <li>If the puzzle data contains direct contradictions
     *       (duplicate digits in a row, column, or 3×3 box),
     *       the function returns <code>"Invalid puzzle"</code>.</li>
     *   <li>If the solver finds a valid and consistent assignment for all cells,
     *       it writes the solution back to <code>puzzle-&gt;data</code> and returns
     *       <code>"Solved"</code>.</li>
     *   <li>If the puzzle passes the initial validity check but the internal
     *       constraint propagation and backtracking search cannot produce a
     *       consistent board state, the function returns
     *       <code>"No solution found"</code>.<br/>
     *       <i>This state indicates that the initial grid is formally valid
     *       but logically contradictory — a situation where the search space
     *       collapses due to hidden conflicts rather than a true absence
     *       of mathematical solutions.</i></li>
     * </ul>
     *
     * <p><b>Return value:</b><br/>
     * A constant null-terminated string describing the solver outcome.
     * The returned pointer is owned by the library and must not be freed.</p>
     *
     * <p><b>Thread-safety:</b><br/>
     * This function is <i>thread-safe</i> as long as different threads
     * operate on distinct <code>sudoku_puzzle_t</code> instances.</p>
     *
     * <p><b>Safety and integrity:</b><br/>
     * Because the <code>sudoku_puzzle_t</code> structure has a fixed binary layout,
     * external callers cannot manipulate or falsify the buffer size.
     * This ensures safe interoperability between the C++ core and
     * C-based foreign-function interfaces (FFI) without compromising memory safety.</p>
     */
    static inline const char *sudoku_solver_c(sudoku_puzzle_t* puzzle){
        if (!puzzle) return "Null pointer";
        return sd::sudoku_solver(puzzle->data, sizeof(puzzle->data)/sizeof(puzzle->data[0]));
    }

#ifdef __cplusplus
}
#endif

#endif // SD_C_API_H
