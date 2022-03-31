# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)

from spack import *


class Pfd(CMakePackage):
    """FIXME: Put a proper description of your package here."""

    homepage = "https://github.com/singular-gpispace/PFD"
    url      = "https://github.com/singular-gpispace/PFD/archive/refs/tags/v0.02.tar.gz"
    git      = "https://github.com/singular-gpispace/PFD"

    maintainers = ['MHeymann', 'jankoboehm', 'lristau', 'mrahn']

    version('0.02', sha256='945e8419258f53bd29dc03aa6b2f36caa92900f4f9893747e28683d76df20653')

    depends_on('singular@snapshot_22_03')
    depends_on('gpi-space@21.09')
    depends_on('flint@2.6.3')

    def cmake_args(self):
        spec = self.spec
        print(self.spec)
        args = [ self.define("GSPC_HOME", spec['gpi-space'].prefix)
               , self.define("SINGULAR_HOME", spec['singular'].prefix)
               , self.define("FLINT_HOME", spec['flint'].prefix)
               ]
        return args
