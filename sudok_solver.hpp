/**
* @file sudok_solver.hpp
 * @author JeongHan-Bae <mastropseudo@gmail.com>
 * @brief Efficient Backtracking Sudoku Solver Implementation
 *
 * @details
 * This file provides a high-performance Sudoku solver using iterative backtracking
 * with constraint propagation. The solver operates on a fixed-size stack and uses
 * bit-masking to efficiently manage candidate values for each cell.
 *
 * ## Historical Background
 * The name "Sudok" (수독) is derived from the Joseonjok pronunciation of the Chinese word "Shudu" (数独).
 * Though popularized in Japan as "Sudoku", its history predates the 20th century:
 *
 * - In the 18th century, Leonhard Euler worked on Latin squares.
 * - In 1612, Claude-Gaspard Bachet de Méziriac discussed 3x3 square puzzles.
 * - In China’s Song Dynasty (960–1279), the "九宫" (Nine Palace Grid) puzzle existed.
 * - A 3x3 grid with magic sum 15 was documented as early as the BeiZhou Dynasty (6th century).
 *
 *
 * @note This file is intended to be directly included or compiled as part of your build.
 *
 * -----------------------------------------------------------------------------
 * MIT License
 *
 * Copyright (c) 2025 JeongHan-Bae
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * -----------------------------------------------------------------------------
 */


#pragma once

#include <cstdint>
#include <cstddef>
#include <iostream>
#include <cstring>
#include <memory>      // NOLINT for std::unique_ptr


namespace sd {
    namespace detail {
        // =========================
        //   POD TYPE OPTIMIZATION
        // =========================

        /**
         * @brief Compile-time trait to check if a type is POD-like.
         */
        template<typename T>
        struct is_pod_like : std::integral_constant<
                    bool,
                    std::is_trivially_copyable<T>::value && // NOLINT for std::is_trivially_copyable_v
                    std::is_trivially_constructible<T>::value && // NOLINT for std::is_trivially_constructible_v
                    std::is_trivially_destructible<T>::value && // NOLINT for std::is_trivially_destructible_v
                    std::is_standard_layout<T>::value // NOLINT for std::is_standard_layout_v
                > {
        };

        // =========================
        //   BIT OPERATIONS
        // =========================

#if defined(__GNUC__) || defined(__clang__)
#define HAS_BUILTIN_CTZ 1
#else
#define HAS_BUILTIN_CTZ 0
#endif

        /**
         * @brief Return the position of the only bit set in a power-of-two mask.
         * @return Value in [0, 15], or -1 if input is not a power-of-two.
         */
        inline int8_t get_power_of_two_runtime(std::uint16_t x) {
            if (x == 0 || (x & (x - 1)) != 0)
                return -1;

#if HAS_BUILTIN_CTZ
            return static_cast<int8_t>(__builtin_ctz(x));
#else
        int8_t n = 0;
        while (x >>= 1) ++n;
        return n;
#endif
        }

        /**
         * @brief Return the number of bits set in a 16-bit integer.
         */
        inline uint8_t popcount16(const std::uint16_t x) {
#if defined(__GNUC__) || defined(__clang__)
            return static_cast<uint8_t>(__builtin_popcount(x));
#else   // MSVC support removed to avoid fragile compilation
        uint8_t count = 0;
        uint16_t val = x;
        while (val) {
            count += val & 1;
            val >>= 1;
        }
        return count;
#endif
        }
    }

    // =========================
    //   FIXED ARRAY TEMPLATE
    // =========================
    template<typename T, std::uint16_t N, bool = detail::is_pod_like<T>::value>
    struct array_impl;

    template<typename T, std::uint16_t N>
    struct alignas(alignof(T)) array_impl<T, N, true> final {
        T data[N];

        constexpr T &operator[](std::size_t i) noexcept { return data[i]; }
        constexpr const T &operator[](std::size_t i) const noexcept { return data[i]; }

        constexpr T *begin() noexcept { return data; }
        [[nodiscard]] constexpr const T *begin() const noexcept { return data; }
        constexpr T *end() noexcept { return data + N; }
        [[nodiscard]] constexpr const T *end() const noexcept { return data + N; }

        [[nodiscard]] static constexpr std::size_t size() noexcept { return N; }

        bool operator==(const array_impl &other) const {
            return std::memcmp(data, other.data, sizeof(data)) == 0;
        }

        bool operator!=(const array_impl &other) const {
            return !(*this == other);
        }
    };

    template<typename T, std::uint16_t N>
    struct alignas(alignof(T)) array_impl<T, N, false> final {
        static_assert(true, "POD-like type required");
    };

    template<typename T, std::uint16_t N>
    using array = array_impl<T, N>;

    // =========================
    //   CELL STRUCTURE
    // =========================

    /**
     * @brief A single Sudoku cell, encoded as a bitmask.
     */
    struct SudokuCell {
        std::uint16_t state; ///< Bitmask: bit 0 = confirmed, bits 1–9 = candidate digits

        [[nodiscard]] bool isConfirmed() const {
            return state & 0b1;
        }

        [[nodiscard]] std::uint16_t possibleMask() const {
            return state & 0b1111111110;
        }

        [[nodiscard]] bool isValid() const {
            const std::uint16_t mask = possibleMask();
            if (state == 0) return false;
            if (state == 1) return false;
            if (isConfirmed() && (mask & (mask - 1)) != 0) return false;
            return true;
        }

        [[nodiscard]] int8_t getConfirmedValue() const {
            if (!isConfirmed()) return 0;
            const std::uint16_t mask = possibleMask();
            if ((mask & mask - 1) != 0) return 0; // ensure single-bit
            return detail::get_power_of_two_runtime(mask);
        }
    };

    // =========================
    //   SUDOKU BOARD
    // =========================

    /**
     * @brief Main board class representing a 9x9 Sudoku puzzle.
     */
    struct Board {
        array<SudokuCell, 81> cells;

        SudokuCell &at(const uint8_t r, const uint8_t c) { return cells[r * 9 + c]; }
        [[nodiscard]] const SudokuCell &at(const uint8_t r, const uint8_t c) const { return cells[r * 9 + c]; }

        array<SudokuCell * const, 9> get_row(const uint8_t r) {
            return {
                &at(r, 0), &at(r, 1), &at(r, 2), &at(r, 3), &at(r, 4),
                &at(r, 5), &at(r, 6), &at(r, 7), &at(r, 8)
            };
        }

        array<SudokuCell * const, 9> get_col(const uint8_t c) {
            return {
                &at(0, c), &at(1, c), &at(2, c), &at(3, c), &at(4, c),
                &at(5, c), &at(6, c), &at(7, c), &at(8, c)
            };
        }

        array<SudokuCell * const, 9> get_box(const uint8_t b) {
            const uint8_t sr = b / 3 * 3;
            const uint8_t sc = b % 3 * 3;
            return {
                &at(sr, sc), &at(sr, sc + 1), &at(sr, sc + 2),
                &at(sr + 1, sc), &at(sr + 1, sc + 1), &at(sr + 1, sc + 2),
                &at(sr + 2, sc), &at(sr + 2, sc + 1), &at(sr + 2, sc + 2)
            };
        }

        static bool deduce_group(array<SudokuCell * const, 9> group) {
            uint16_t confirmedMask = 0b1;
            bool changed = false;
            for (const auto *cell: group)
                if (cell->isConfirmed()) confirmedMask |= cell->possibleMask();

            for (auto *cell: group) {
                if (!cell->isConfirmed()) {
                    const uint16_t mask = cell->possibleMask() & ~confirmedMask;
                    if (mask == 0) return false;
                    if ((mask & mask - 1) == 0) {  // ensure single-bit
                        cell->state = mask | 0b1;
                        confirmedMask |= mask;
                        changed = true;
                    } else if (mask != cell->possibleMask()) {
                        cell->state = mask;
                        changed = true;
                    }
                }
            }
            return changed;
        }

        bool deduce_once() {
            bool changed = false;
            for (uint8_t i = 0; i < 9; ++i) {
                changed |= deduce_group(get_row(i));
                changed |= deduce_group(get_col(i));
                changed |= deduce_group(get_box(i));
            }
            return changed;
        }

        bool deduce_full() {
            while (deduce_once()) {
            }
            for (auto &c: cells) if (!c.isValid()) return false; // NOLINT for range-based for
            return true;
        }

        [[nodiscard]] bool is_solved() const {
            for (const auto &c: cells) // NOLINT for range-based for
                if (!c.isConfirmed()) return false;
            return true;
        }

        void print() const {
            for (uint8_t r = 0; r < 9; ++r) {
                for (uint8_t c = 0; c < 9; ++c) {
                    const uint8_t v = at(r, c).getConfirmedValue();
                    std::cout << (v ? static_cast<char>('0' + v) : '.') << ' ';
                }
                std::cout << '\n';
            }
        }

        void load(const char *str) {
            for (uint8_t i = 0; i < 81; ++i) {
                const char ch = str[i]; // NOLINT for inplace-if
                if (ch >= '1' && ch <= '9')
                    cells[i].state = 0b1 | 1 << (ch - '0'); // NOLINT for parentheses
                else
                    cells[i].state = 0b1111111110;
            }
        }

        void load_int8_t(const int8_t *arr) {
            for (uint8_t i = 0; i < 81; ++i) {
                const int8_t ch = arr[i]; // NOLINT for inplace-if
                if (ch >= 1 && ch <= 9)
                    cells[i].state = 0b1 | 1 << ch; // NOLINT for parentheses
                else
                    cells[i].state = 0b1111111110;
            }
        }

        int8_t inner_solve() {
            if (!deduce_full()) return -2;

            if (is_solved()) return -1;

            uint8_t min_choices = 10;
            int8_t target_idx = -2;

            for (int8_t i = 0; i < 81; ++i) {
                if (!cells[i].isConfirmed()) {
                    auto count = detail::popcount16(cells[i].possibleMask()); // NOLINT for inplace-if
                    if (count < min_choices) {
                        min_choices = count;
                        target_idx = i;
                    }
                }
            }

            return target_idx;
        }

        [[nodiscard]] bool check_initial_valid() {
            for (uint8_t i = 0; i < 9; ++i) {
                if (!check_unit(get_row(i))) return false;
                if (!check_unit(get_col(i))) return false;
                if (!check_unit(get_box(i))) return false;
            }
            return true;
        }

        static bool check_unit(const array<SudokuCell * const, 9> &group) {
            uint16_t confirmed = 0b0;
            for (const auto *cell: group) {
                if (cell->isConfirmed()) {
                    const uint16_t mask = cell->possibleMask();
                    if ((confirmed & mask) != 0) return false; // duplicate
                    confirmed |= mask;
                }
            }
            return true;
        }
    };

    // =========================
    //   BACKTRACKING SOLVER
    // =========================

    /**
     * @brief Internal stack frame for iterative backtracking.
     */
    struct Frame {
        Board board{};
        uint16_t remaining_mask{};
        uint8_t target_idx{};
    };


    /**
     * @brief Solve the given Sudoku board using iterative backtracking.
     * @param root Board to solve (in-place).
     * @return true if solved, false otherwise.
     */
    static bool solve(Board &root) {
        const auto raw_stack = std::make_unique<uint8_t[]>(81 * sizeof(Frame)); // Maximum 81 frames
        auto stack = reinterpret_cast<Frame *>(raw_stack.get()); // Avoid initializing unused stack frames
        int8_t top = 0;

        const int8_t result = root.inner_solve();
        if (result == -1) return true; // already solved
        if (result == -2) return false; // invalid

        stack[0].board = root;
        stack[0].target_idx = static_cast<uint8_t>(result);
        stack[0].remaining_mask = root.cells[result].possibleMask();

        while (top >= 0) {
            Frame &frame = stack[top]; // NOLINT for object auto-unpacking
            const uint16_t mask = frame.remaining_mask;

            if (mask == 0) {
                --top; // no mask, backtrack
                continue;
            }

            // Lowest mask
            const uint16_t pick = mask & -mask;
            frame.remaining_mask ^= pick;

            Board next = frame.board;
            next.cells[frame.target_idx].state = pick | 0b1;

            const int8_t res = next.inner_solve();
            if (res == -1) {
                root = next;
                return true;
            }
            if (!next.check_initial_valid()) {
                --top; // The previous state is invalid which leads to this state
                continue;
            } // Only check if I took a guess
            if (res == -2) continue;

            ++top;
            stack[top].board = next;
            stack[top].target_idx = res;
            stack[top].remaining_mask = next.cells[res].possibleMask();
        }

        return false;
    }

    /**
     * @brief Public API for external use.
     * @param puzzle Pointer to 81-element int8_t array (0 for empty, 1–9 for digits).
     * @param size Must be 81.
     * @return String indicating result ("Solved", "Invalid puzzle", etc.)
     */
    inline const char *sudoku_solver(int8_t *puzzle, const uint64_t size) {
        if (size != 81) {
            return "Invalid size";
        }
        Board board{};
        try {
            board.load_int8_t(puzzle);
        } catch (const std::exception &) {
            return "Size mismatch with declared size";
        }
        if (!board.check_initial_valid()) {
            return "Invalid puzzle";
        }
        if (solve(board)) {
            for (uint8_t i = 0; i < 81; ++i) {
                puzzle[i] = board.cells[i].getConfirmedValue();
            }
            return "Solved";
        }
        return "No solution found";
    }
}