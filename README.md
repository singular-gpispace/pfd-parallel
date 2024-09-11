# pfd-parallel - Massively Parallel Partial Fraction Decomposition

Note: The information below, and more details, can be found in our wiki 

<p align="center">
  <a href="https://github.com/singular-gpispace/pfd-parallel/wiki">Documentation of the pfd-parallel project</a>
</p>

This package provide a massively parallel framework for partial fraction decomposition of
rational functions based on the [Singular/GPI-Space framework](https://www.mathematik.uni-kl.de/~boehm/singulargpispace/).

The project is supported by [Project B5](https://www.computeralgebra.de/sfb/projects/singular-a-new-level-of-abstraction-and-performance/) of [SFB-TRR 195](https://www.computeralgebra.de/sfb/) and [SymbTools](https://rptu.de/forschung/forschungsinitiative-rlp/symbtools) of [Forschungsinitiative RLP](https://mwg.rlp.de/de/themen/wissenschaft/forschung-transfer-und-innovation/forschung-und-innovation/forschungsinitiative-des-landes-rlp/).

Our implementation is based on a combination of the following two algorithms:

1) the enhanced Leinartas' algorithm described in the paper

Janko Boehm, Marcel Wittmann, Zihao Wu, Yingxuan Xu, and Yang Zhang:
IBP reduction coefficients made simple, JHEP 12 (2020) 054,

which has been implemened in [Singular](https://www.singular.uni-kl.de) in the library
[pfd.lib](https://github.com/Singular/Singular/blob/spielwiese/Singular/LIB/pfd.lib).

2) the MultivariateApart algorithm as described in

Matthias Heller, Andreas von Manteuffel, Comput.Phys.Commun. 271 (2022) 108174 

Although applicable in general, the primary aim of our package is the partial fraction
decomposition of integration-by-parts coefficients in high energy physics.

Our package relies on code developed in the repository
[framework](https://github.com/singular-gpispace/framework)
implemented primarily by Lukas Ristau.

Since most useful in applications in high energy physics, the main function of our framework applies the partial fraction decoposition
function to a specified subset of entries of a two-dimensional array of rational functions.

