# SudokLite — A Lightning-Fast, Header-Only Sudoku Solver (`sudok_solver.hpp`)

> 🐉 **SudokLite** — "Sudoku" (수독) in Joseonjok dialect + *Lite* for speed & minimalism  
> Naturally pronounced *Sudong-nite* in speech due to nasalized linking.  
> Western readers may say "Sudok-Lite" or "Sudoku Lite" — both are valid!  
> A blazing-fast, header-only Sudoku solver with Python GUI support.  
> Built for embedding, scripting, teaching — or just for fun.  

![Version](https://img.shields.io/badge/version-1.1.0-blue)
![License](https://img.shields.io/badge/license-MIT-green)
![C++](https://img.shields.io/badge/c%2B%2B-14%2F20-orange)

---

## 🚀 Features

* ✅ **Single Header Only** (`sudok_solver.hpp`)
* ✅ **Zero Dynamic Memory** — `constexpr`-friendly, no heap allocation
* ✅ **Deterministic Solving** — iterative backtracking + constraint propagation
* ✅ **C++14 Compatible** — portable and STL-minimal
* ✅ **C++20 Enhanced** — auto-integrates with [`jh/pod` from JH-Toolkit](https://github.com/JeongHan-Bae/JH-Toolkit)
* ✅ **FFI-Safe** — clean C interface, ABI-stable
* ✅ **Python Bindings** — Cython-powered NumPy API
* ✅ **GUI Demo** — modern PySide6 frontend

---

## 🧩 Implementation Highlights

SudokLite compiles cleanly on **any C++14-capable compiler**,
but automatically upgrades to use [`jh/pod`](https://github.com/JeongHan-Bae/JH-Toolkit/blob/main/docs/pods/overview.md)
when built under **C++20** and the library is available in your system.

| Environment        | Behavior                                                                                                        |
|--------------------|-----------------------------------------------------------------------------------------------------------------|
| **C++14**          | Uses the internal lightweight `sd::detail::array_impl<T, N>` implementation                                     |
| **C++20 + jh/pod** | Transparently maps `sd::array<T, N>` → `jh::pod::array<T, N>` for constexpr, alignment, and safety improvements |

> This mapping avoids reimplementing POD logic while keeping binary and ABI compatibility stable across build targets.

---

### 🧩 Design Notes

* `sd::array<T, N>` is simply an alias for `sd::detail::array_impl<T, N>`.
* When `<jh/pod>` is found (`__has_include(<jh/pod>)` + `SD_USE_JH_POD` macro not defined),
  the solver automatically performs:

  ```cpp
  namespace sd{
      using jh::pod::array;
  }
  ```
* This rebinds only the **internal POD layer**, ensuring:

    * ✅ identical external semantics (e.g., `Board`, `Frame` types unchanged),
    * ✅ constexpr evaluation improvements under C++20,
    * ✅ guaranteed standard-layout + trivially copyable types.

---

### ⚙️ POD Array Safety

Earlier versions used:

```cpp
array<SudokuCell *const, 9>
```

However, top-level `const` on pointer types violates the POD constraint
(`std::is_const_v<T>` must be `false` for `pod_like` concepts).

It has been corrected to:

```cpp
const array<SudokuCell*, 9>
```

This ensures:

* ✅ Each element type `T = SudokuCell*` is still trivially copyable;
* ✅ The array layout remains POD and safe for FFI;
* ✅ The container itself prevents pointer reassignment while preserving memory safety;
* ✅ Compatible with both internal and `jh::pod` backends.

> This change guarantees POD compliance and stability in embedded or zero-copy scenarios.

---

### 🧱 C API Safety Enhancements

The legacy interface used:

```c
const char* sudoku_solver(int8_t* puzzle, uint64_t size);
```

which relied on the caller to provide a valid size (often leading to unsafe use).

It is now replaced by a fixed-size protocol structure:

```c
typedef struct {
    int8_t data[81];
} sudoku_puzzle_t;

const char* sudoku_solver_c(sudoku_puzzle_t* puzzle);
```

**Advantages:**

* ✅ Compile-time size enforcement (81 elements, fixed layout)
* ✅ No risk of size forgery or pointer aliasing
* ✅ Clean cross-language ABI (C / Python / FFI-safe)
* ✅ Still uses runtime verification inside C++ for resilience:

  ```cpp
  try {
      board.load_int8_t(puzzle);
  } catch (const std::exception &) {
      return "Size mismatch with declared size";
  }
  ```

> This design makes SudokLite's external API secure, predictable, and suitable for scripting or embedded use.

---

## 🧩 Header-Only C++ Usage

```c++
#include "sudok_solver.hpp"
#include <cstdint>
#include <iostream>

int main() {
    int8_t puzzle[81] = {
        5,3,0, 0,7,0, 0,0,0,
        6,0,0, 1,9,5, 0,0,0,
        0,9,8, 0,0,0, 0,6,0,
        8,0,0, 0,6,0, 0,0,3,
        4,0,0, 8,0,3, 0,0,1,
        7,0,0, 0,2,0, 0,0,6,
        0,6,0, 0,0,0, 2,8,0,
        0,0,0, 4,1,9, 0,0,5,
        0,0,0, 0,8,0, 0,7,9
    };

    const char* result = sd::sudoku_solver(puzzle, 81);

    if (std::string(result) == "Solved") {
        std::cout << "Solved Sudoku:\n";
        for (int i = 0; i < 81; ++i) {
            std::cout << static_cast<int>(puzzle[i]) << ' ';
            if ((i + 1) % 9 == 0) std::cout << '\n';
        }
    } else {
        std::cerr << "❌ Solver failed: " << result << '\n';
    }
}
```

---

## 🐍 Python Bindings (Cython)

SudokLite includes Python support via Cython, found in [`bindings/python/`](./bindings/python):

```bash
cd bindings/python
python setup.py build_ext --inplace
```

> 💡 **Note:**
>
> * Install Cython and NumPy first:
>   ```bash
>   pip install cython numpy
>   ```
> * GUI demo needs PySide6:
>
>   ```bash
>   pip install PySide6
>   ```
> * Optional: install the module system-wide:
>
>   ```bash
>   pip install ./bindings/python
>   ```

---

Then in Python:

```python
from sd_solver import solve  # compiled Cython binding
import numpy as np

puzzle = np.array([...], dtype=np.int8)
solve(puzzle) # solve the puzzle in-place also having a return value (optional)
print(puzzle.reshape(9, 9))  # puzzle is solved in-place
```

---

## 🖼 GUI Example (PySide6)

A fully working PySide6 GUI lives in [`examples/gui/sudoku_gui.py`](./examples/gui/sudoku_gui.py):

```bash
python examples/gui/sudoku_gui.py
```

Features:

* Fill grid with mouse or keyboard
* Virtual numeric keypad
* Arrow key navigation with wrapping
* Solve button
* Clear board button
* Error display if solving fails

---

## 📦 Folder Structure

```markdown
/
├── sudok_solver.hpp             # ✅ Core solver (header-only)
├── sd_c_api.h                   # C interface for FFI / Cython
├── bindings/python/             # Cython bindings
│   ├── sd_solver.pxd
│   ├── sd_solver.pyx
│   └── setup.py
├── examples/sudoku_gui.py       # GUI frontend with PySide6
└── README.md                    # You're reading it
```

---

## 📄 License

[MIT License © 2025](LICENSE) [JeongHan-Bae](https://github.com/JeongHan-Bae)

---

## 💬 Feedback / Contribution

Found a bug? Want to add features?
Open a PR or issue — contributions welcome!

If you're using **SudokLite** in your project, course, or hobby app — I'd love to hear about it! 🧠⚡
