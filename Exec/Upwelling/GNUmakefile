# AMReX
COMP = gnu
PRECISION = DOUBLE

# Profiling
PROFILE       = FALSE
TINY_PROFILE  = FALSE
COMM_PROFILE  = FALSE
TRACE_PROFILE = FALSE
MEM_PROFILE   = FALSE
USE_GPROF     = FALSE

# Performance
USE_MPI   = TRUE
USE_OMP   = FALSE
USE_CUDA  = FALSE
USE_HIP   = FALSE
USE_SYCL  = FALSE

# Debugging
DEBUG = TRUE
DEBUG = FALSE

TEST = TRUE
USE_ASSERTION = TRUE

# GNU Make
Bpack := ./Make.package
Blocs := .
ROMSX_HOME := ../..
ROMSX_PROBLEM_DIR = $(ROMSX_HOME)/Exec/Upwelling
USE_NETCDF = FALSE
DEFINES += -DROMSX_USE_HISTORYFILE
include $(ROMSX_HOME)/Exec/Make.ROMSX
