from setuptools import setup
from setuptools.extension import Extension
from Cython.Build import cythonize
import numpy
import os

base_dir = os.path.abspath(os.path.dirname(__file__))
c_api_path = os.path.abspath(os.path.join(base_dir, "..", ".."))

ext_modules = [
    Extension(
        "sd_solver",
        sources=["sd_solver.pyx"],
        language="c++",
        include_dirs=[base_dir, c_api_path, numpy.get_include()],
        extra_compile_args=["-std=c++14", "-DNPY_NO_DEPRECATED_API=NPY_1_7_API_VERSION"],
    )
]

setup(
    name="sd_solver",
    ext_modules=cythonize(ext_modules, language_level="3"),
    version="1.0.0",
    zip_safe=False,
)
