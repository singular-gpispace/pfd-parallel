# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)

from spack import *


class Flint(Package):
    """FLINT (Fast Library for Number Theory)."""

    homepage = "https://www.flintlib.org"
    url      = "https://mirrors.mit.edu/sage/spkg/upstream/flint/flint-2.6.3.tar.gz"
    git      = "https://github.com/wbhart/flint2.git"

    version('develop', branch='trunk')
    version('2.6.3', sha256='ce1a750a01fa53715cad934856d4b2ed76f1d1520bac0527ace7d5b53e342ee3')
    version('2.5.2', sha256='cbf1fe0034533c53c5c41761017065f85207a1b770483e98b2392315f6575e87')
    version('2.4.5', sha256='e489354df00f0d84976ccdd0477028693977c87ccd14f3924a89f848bb0e01e3')

    # Overlap in functionality between gmp and mpir
    # All other dependencies must also be built with
    # one or the other
    # variant('mpir', default=False,
    #         description='Compile with the MPIR library')

    # Build dependencies
    depends_on('autoconf', type='build')

    # Other dependencies
    depends_on('gmp')   # mpir is a drop-in replacement for this
    depends_on('mpfr')  # Could also be built against mpir

    phases = ['configure', 'build', 'install']

    def configure_args(self):
        prefix = self.prefix
        spec = self.spec
        args = ["--prefix=%s" % prefix,
                   "--with-gmp=%s" % spec['gmp'].prefix,
                   "--with-mpfr=%s" % spec['mpfr'].prefix]

        # if '+mpir' in spec:
        #     args.extend([
        #         "--with-mpir=%s" % spec['mpir'].prefix
        #     ])

        return args

    def configure(self, spec, prefix):
        configure_script = Executable("./configure")
        configure(*self.configure_args())

    def build(self, spec, prefix):
        make()
        if self.run_tests:
            make("check")

    def install(self, spec, prefix):
        #options = ["--prefix=%s" % prefix,
        #           "--with-gmp=%s" % spec['gmp'].prefix,
        #           "--with-mpfr=%s" % spec['mpfr'].prefix]
        #configure(*options)
        #make()
        #if self.run_tests:
        #    make("check")
        make("install")
