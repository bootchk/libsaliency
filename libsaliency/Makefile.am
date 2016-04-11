#######################################
# The list of libraries we are building seperated by spaces.
# The 'lib_' indicates that these build products will be installed
# in the $(libdir) directory. For example /usr/lib
lib_LTLIBRARIES = libsaliency.la

#######################################
# Build information for each library

# Sources for libsaliency
libsaliency_la_SOURCES = libsaliency.cpp Gradienter.cpp Smoother.cpp Sampler.cpp Gradienter.h Smoother.h Sampler.h 

# Linker options libTestProgram
libsaliency_la_LDFLAGS = 

# Compiler options. Here we are adding the include directory
# to be searched for headers included in the source code.
libsaliency_la_CPPFLAGS = -I$(top_srcdir)/include

