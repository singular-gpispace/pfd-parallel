# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)

from spack import *


class Singular(AutotoolsPackage):
    """
    Singular is a computer algebra system for polynomial computations, with
    special emphasis on commutative and non-commutative algebra, algebraic
    geometry, and singularity theory. It is free and open-source under the GNU
    General Public Licence.
    """

    homepage = "https://www.singular.uni-kl.de"
    url      = "https://github.com/Singular/Singular/archive/refs/tags/Release-4-3-0.tar.gz"
    git      = "https://github.com/Singular/Singular.git"

    maintainers = ['hannes14', 'MHeymann']

    version('snapshot_22_03', commit='e12a8af3d7330a34cb43811e28ecde25afe83a4f')
    version('4-3-0', sha256='9189ef41e91317f4de2b4e5d83ae257a26a0bd009a2a4a133b4477212a1f6fd2')

    depends_on('autoconf', type='build')
    depends_on('automake', type='build')
    depends_on('libtool',  type='build')
    depends_on('m4',       type='build')

    depends_on('readline@8.1')
    depends_on('ntl@11.5.1 +shared')
    depends_on('flint@2.6.3')
    depends_on('cddlib@0.94m')
    depends_on('4ti2@1.6')

    def autoreconf(self, spec, prefix):
        autogen_script = Executable("./autogen.sh")
        autogen_script()

    def configure_args(self):
        spec = self.spec
        args = [ '--with-flint={0}'.format(spec['flint'].prefix)
               , '--with-ntl={0}'.format(spec['ntl'].prefix)
               , '--enable-gfanlib'
               ]
        return args
