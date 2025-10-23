from setuptools import setup, Extension
from Cython.Build import cythonize
import numpy
import subprocess
import tempfile
import os
import sys

# -----------------------------------------------------------------------------
# Base configuration
# -----------------------------------------------------------------------------
base_dir = os.path.abspath(os.path.dirname(__file__))
c_api_path = os.path.abspath(os.path.join(base_dir, "..", ".."))

is_msvc = sys.platform == "win32"

extra_compile_args = []
extra_link_args = []
include_dirs = [base_dir, c_api_path, numpy.get_include()]

# -----------------------------------------------------------------------------
# Step 1: Try to discover jh::jh-toolkit* via CMake introspection
# -----------------------------------------------------------------------------
def cmake_find_jh_toolkit():
    """Try to find jh::jh-toolkit target via temporary CMake project."""
    with tempfile.TemporaryDirectory() as tmp:
        cmakelists = os.path.join(tmp, "CMakeLists.txt")
        with open(cmakelists, "w") as f:
            f.write(r"""
cmake_minimum_required(VERSION 3.14)
project(tmp_probe LANGUAGES CXX)

find_package(jh-toolkit QUIET)

if (TARGET jh::jh-toolkit)
  set(target jh::jh-toolkit)
elseif (TARGET jh::jh-toolkit-static)
  set(target jh::jh-toolkit-static)
elseif (TARGET jh::jh-toolkit-pod)
  set(target jh::jh-toolkit-pod)
else()
  message(FATAL_ERROR "No jh-toolkit target found")
endif()

get_target_property(INC ${target} INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(LIBS ${target} INTERFACE_LINK_LIBRARIES)

# Prevent NOTFOUND pollution
if (LIBS STREQUAL "LIBS-NOTFOUND")
  unset(LIBS)
endif()

message(STATUS "JH_TARGET=${target}")
message(STATUS "JH_INCLUDE_DIRS=${INC}")
message(STATUS "JH_LINK_LIBS=${LIBS}")
""")

        try:
            result = subprocess.run(
                [
                    "cmake", "-S", tmp, "-B", os.path.join(tmp, "build"),
                    "-DCMAKE_PREFIX_PATH=/usr/local;/usr;/lib;/usr/lib;/usr/local/lib"
                ],
                capture_output=True, text=True, check=True,
            )
        except subprocess.CalledProcessError:
            return None

        includes, libs, target = [], [], None
        for line in result.stdout.splitlines():
            line = line.strip()
            if line.startswith("-- JH_TARGET="):
                target = line.split("=", 1)[1].strip()
            elif line.startswith("-- JH_INCLUDE_DIRS="):
                incs = line.split("=", 1)[1].strip().split(";")
                includes.extend([x for x in incs if x])
            elif line.startswith("-- JH_LINK_LIBS="):
                libnames = line.split("=", 1)[1].strip().split(";")
                libs.extend([x for x in libnames if x and "NOTFOUND" not in x])
        if target:
            return {"target": target, "include_dirs": includes, "link_libs": libs}
        return None


# -----------------------------------------------------------------------------
# Step 2: Platform-specific logic
# -----------------------------------------------------------------------------
if is_msvc:
    # Force UTF-8 encoding for stdout/stderr to avoid UnicodeEncodeError on Windows
    sys.stdout.reconfigure(encoding="utf-8")
    sys.stderr.reconfigure(encoding="utf-8")
    # Windows uses MSVC — skip jh-toolkit detection
    print("\u2139\ufe0f  MSVC detected \u2014 skipping jh-toolkit detection (use internal POD)")
    extra_compile_args = ["/std:c++20", "/DNPY_NO_DEPRECATED_API=NPY_1_7_API_VERSION"]
else:
    # Try to detect jh-toolkit on UNIX-like systems
    jh = cmake_find_jh_toolkit()
    if jh:
        print(f"\u2705 Found {jh['target']}")
        for inc in jh["include_dirs"]:
            if inc and os.path.exists(inc):
                include_dirs.append(inc)
        for lib in jh["link_libs"]:
            if lib and "NOTFOUND" not in lib:
                extra_link_args.append(lib)
        extra_compile_args += ["-std=c++20", "-DSD_USE_JH_POD=1"]
    else:
        print("\u26a0\ufe0f  jh-toolkit not found \u2014 using internal POD fallback")
        extra_compile_args += ["-std=c++17"]
    extra_compile_args.append("-DNPY_NO_DEPRECATED_API=NPY_1_7_API_VERSION")

# -----------------------------------------------------------------------------
# Step 3: Define Cython extension
# -----------------------------------------------------------------------------
ext_modules = [
    Extension(
        "sd_solver",
        sources=["sd_solver.pyx"],
        language="c++",
        include_dirs=include_dirs,
        extra_compile_args=extra_compile_args,
        extra_link_args=extra_link_args,
    )
]

# -----------------------------------------------------------------------------
# Step 4: Build
# -----------------------------------------------------------------------------
setup(
    name="sd_solver",
    version="1.1.0",
    ext_modules=cythonize(ext_modules, language_level="3"),
    zip_safe=False,
)
