#    PyEW is a library that creates a python interface to the Earthworm Transport system
#    Copyright (C) 2018  Francisco J Hernandez Ramirez
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU Affero General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU Affero General Public License for more details.
#
#    You should have received a copy of the GNU Affero General Public License
#    along with this program.  If not, see <https://www.gnu.org/licenses/>

from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize

extensions = [Extension("PyEW", ["PyEW.pyx"])]
setup(
    ext_modules = cythonize(extensions)
)
