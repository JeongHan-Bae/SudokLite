# NOTICE

This product optionally integrates with **jh-toolkit** to enable enhanced POD array
support for environments that provide the `<jh/pod>` module.

During build configuration, the system attempts to detect one of the following
CMake targets:

- `jh::jh-toolkit`
- `jh::jh-toolkit-static`
- `jh::jh-toolkit-pod`

All of these variants expose the same header-only `<jh/pod>` interface.  
Even when `jh::jh-toolkit-static` is detected, **no static linking occurs** —  
only the header module is included for compilation.

---

- **jh-toolkit**  
  Copyright © 2025 JeongHan Bae [<mastropseudo@gmail.com>](mailto:mastropseudo@gmail.com)  
  Licensed under the **Apache License, Version 2.0**  
  <https://github.com/JeongHan-Bae/JH-Toolkit>

---

This integration is optional and detected dynamically at build time via CMake.  
**SudokLite** itself does not distribute, modify, or statically link `jh-toolkit`;  
it merely consumes its public header `<jh/pod>` when present.
