# SudokLite — A Lightning-Fast, Header-Only Sudoku Solver (`sudok_solver.hpp`)

> 🐉 **SudokLite** — “Sudoku” (수독) in Joseonjok dialect + *Lite* for speed & minimalism  
> Naturally pronounced *Sudong-nite* in speech due to nasalized linking.  
> Western readers may say “Sudok-Lite” or “Sudoku Lite” — both are valid!  
> A blazing-fast, header-only Sudoku solver with Python GUI support.  
> Built for embedding, scripting, teaching — or just for fun.

![Version](https://img.shields.io/badge/version-1.0.0-blue)
![License](https://img.shields.io/badge/license-MIT-green)
![C++](https://img.shields.io/badge/c%2B%2B-14+-orange)

---

## 🚀 Features

- ✅ **Single Header Only** (`sudok_solver.hpp`)
- ✅ **Zero Dynamic Memory** — `constexpr`-friendly, no heap allocation
- ✅ **Deterministic Solving** — fast, backtracking-based core
- ✅ **C++14 Compatible** — minimal STL, portable across compilers
- ✅ **FFI-Friendly** — plain `signed char*` + C-style API
- ✅ **Python Bindings** — Cython-powered extension module
- ✅ **GUI Demo** — modern PySide6 interface included

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
> - Install Cython and NumPy first:
>   ```bash
>   pip install cython numpy
>   ```
> - GUI demo needs PySide6:
>   ```bash
>   pip install PySide6
>   ```
> - Optional: install the module system-wide:
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
- Fill grid with mouse or keyboard
- Virtual numeric keypad
- Arrow key navigation with wrapping
- Solve button
- Clear board button
- Error display if solving fails

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
