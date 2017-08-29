/**
 * fdtd-apml.h: This file is part of the PolyBench 3.0 test suite.
 *
 *
 * Contact: Louis-Noel Pouchet <pouchet@cse.ohio-state.edu>
 * Web address: http://polybench.sourceforge.net
 */
#ifndef FDTD_APML_H
# define FDTD_AMPL_H

/* Default to STANDARD_DATASET. */
# if !defined(MINI_DATASET) && !defined(SMALL_DATASET) && !defined(LARGE_DATASET) && !defined(EXTRALARGE_DATASET)
#  define STANDARD_DATASET
# endif

/* Do not define anything if the user manually defines the size. */
# if !defined(CZ) && ! defined(CYM) && !defined(CXM)
/* Define the possible dataset sizes. */
#  ifdef MINI_DATASET
#   define CZ 32
#   define CYM 32
#   define CXM 32
#  endif

#  ifdef SMALL_DATASET
#   define CZ 64
#   define CYM 64
#   define CXM 64
#  endif

#  ifdef STANDARD_DATASET /* Default if unspecified. */
#   define CZ 256
#   define CYM 256
#   define CXM 256
/* #   define CZ 256 */
/* #   define CYM 256 */
/* #   define CXM 256 */
#  endif

#  ifdef LARGE_DATASET
#   define CZ 512
#   define CYM 512
#   define CXM 512
#  endif

#  ifdef EXTRALARGE_DATASET
#   define CZ 1000
#   define CYM 1000
#   define CXM 1000
#  endif
# endif /* !N */


# ifndef DATA_TYPE
#  define DATA_TYPE __attribute__((annotate("no_float"))) float
# endif
#  define DATA_PRINTF_MODIFIER "%0.4lf "


#endif /* !FDTD_APML */
