#######################################
# The list of libraries we are building seperated by spaces.
# The 'lib_' indicates that these build products will be installed
# in the $(libdir) directory. For example /usr/lib
lib_LTLIBRARIES = libsaliency.la

#######################################
# Build information for each library

# Sources for libsaliency
libsaliency_la_SOURCES = \
	libsaliency.cpp libsaliency.hpp \
	KernelDensityInfo.cpp kernelDensityInfo.hpp \
	Gradienter.cpp Gradienter.h  \
	Smoother.cpp Smoother.h \
	Sampler.cpp Sampler.h \
	SamplePool.cpp SamplePool.h \
	Bounder.cpp Bounder.h \
	Quantizer.cpp Quantizer.h \
	constants.h \
    Samples.h 

# Linker options libTestProgram
libsaliency_la_LDFLAGS = 

# Compiler options. Here we are adding the include directory
# to be searched for headers included in the source code.
libsaliency_la_CPPFLAGS = -I$(top_srcdir)/include -std=c++11

