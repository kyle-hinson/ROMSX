# AMReX
COMP = gnu
PRECISION = DOUBLE

# Profiling
PROFILE       = FALSE
TINY_PROFILE  = TRUE
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

TEST = TRUE
USE_ASSERTION = TRUE

# Debugging
DEBUG = FALSE

USE_NETCDF = TRUE

# GNU Make
Bpack := ./Make.package
Blocs := .
ROMSX_HOME := ../..
ROMSX_PROBLEM_DIR = $(ROMSX_HOME)/Exec/OCCAMS
DEFINES += -DROMSX_USE_HISTORYFILE
include $(ROMSX_HOME)/Exec/Make.ROMSX
