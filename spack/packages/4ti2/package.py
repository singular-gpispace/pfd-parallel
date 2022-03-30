# Copyright 2013-2021 Lawrence Livermore National Security, LLC and other
# Spack Project Developers. See the top-level COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)

# ----------------------------------------------------------------------------
# If you submit this package back to Spack as a pull request,
# please first remove this boilerplate and all FIXME comments.
#
# This is a template package file for Spack.  We've put "FIXME"
# next to all the things you'll want to change. Once you've handled
# them, you can save this file and test your package like this:
#
#     spack install 4ti2
#
# You can edit this file again by typing:
#
#     spack edit 4ti2
#
# See the Spack documentation for more information on packaging.
# ----------------------------------------------------------------------------

from spack import *


class _4ti2(AutotoolsPackage):
    """FIXME: Put a proper description of your package here."""

    # FIXME: Add a proper url for your package's homepage here.
    homepage = "https://www.4ti2.de"
    url      = "http://www.4ti2.de/version_1.6/4ti2-1.6.tar.gz"

    # maintainers = ['github_user1', 'github_user2']

    version('1.6', sha256='5edd106a3584408d89c58cb432f8b08eec472aa3277a35928523cdb4e43c769e')

