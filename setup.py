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

import os
import sys

from distutils.core import setup
from distutils.extension import Extension
from distutils.command.build import build
from Cython.Build import cythonize


EW_HOME = os.environ.get('EW_HOME', None)
EW_VERSION = os.environ.get('EW_VERSION', None)
if EW_HOME:
    EW_INCLUDES = os.path.join(EW_HOME, 'include')
    if not os.path.exists(EW_INCLUDES):
        EW_INCLUDES = os.path.join(EW_HOME, EW_VERSION, 'include')
        if not os.path.exists(EW_INCLUDES):
            EW_INCLUDES = None
else:
    EW_INCLUDES = None


class CustomBuild(build):

    def check(self):
        """
        Ensure our build environment is set up properly before beginning.
        """
        if not EW_HOME:
            error = """
The Python-EW build process requires the EW_HOME environment
variable to be properly set.  This variable is not in your
environment.
"""
            sys.exit(error)

        if not EW_INCLUDES:
            error = """
The EW_HOME environment variable is set to {0} but we could not find a "include" subfolder.  We tried: {}. We need
the .h files from the Earthworm build so that we can properly compile our C extensions.
""".format(EW_HOME, ", ".join(os.path.join(EW_HOME, 'include'), os.path.join(EW_HOME, EW_VERSION, 'include')))
            sys.exit(error)

        if not os.environ.get('CFLAGS', None):
            error = """
The CFLAGS environment variable is not set.  It should be set up properly with the same CFLAGS values
set during the build of Earthworm on this machine."
"""
            sys.exit(error)

    def run(self):
        self.check()
        include_flag = "-I{}".format(EW_INCLUDES)
        cflags = os.environ['CFLAGS']
        if include_flag not in cflags:
            os.environ['CFLAGS'] = cflags + " " + include_flag
        super().run()


with open("README.md", "r") as fh:
    long_description = fh.read()

setup(
    name='PyEarthworm',
    version='1.41',
    author='Francisco J Hernandez Ramirez',
    url='https://github.com/Boritech-Solutions/PyEarthworm',
    ext_modules=cythonize(Extension('PyEW', ['src/PyEW.pyx'])),
    description='A Python interface to the Earthworm Seismic Data System.',
    long_description=long_description,
    long_description_content_type="text/markdown",
    install_requires=['cython>=0.29', 'numpy'],
    project_urls={
        'Home Page': 'https://github.com/Boritech-Solutions/PyEarthworm',
        'Bug Tracker': 'https://github.com/Boritech-Solutions/PyEarthworm/issues',
        'Documentation': 'https://github.com/Boritech-Solutions/PyEarthworm'
    },
    cmdclass={
        'build': CustomBuild
    },
    classifiers=[
        'Development Status :: 5 - Production/Stable',
        'Intended Audience :: Developers',
        'Intended Audience :: Science/Research',
        'License :: OSI Approved :: GNU Affero General Public License v3',
        'Operating System :: POSIX',
        'Programming Language :: Python :: 3.6',
        'Programming Language :: Python :: 3.7',
        'Topic :: Scientific/Engineering',
        'Topic :: Software Development :: Libraries :: Python Modules',
        'Topic :: Software Development :: Libraries',
    ],
)
