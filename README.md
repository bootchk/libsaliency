libsaliency
===========

A library implementing an image processing algorithm that computes a saliency map.

Derived from https://github.com/kkduncan/SaliencyDetection.git   
The algorithm is the work of  "Relational Entropy-Based Saliency Detection in Images and Videos" by K. Duncan and S. Sarkar in the IEEE International Conference on Image Processing (2012).
 
I just cleaned up that repository a little, and changed it to build a library as well as an example program.
 
Building using Eclipse
----------------------

The .project is an Eclipse IDE project.  Requires JRE (Java runtime) and Eclipse C/C++ IDE.

The nature of the project is "C/C++ Autotools Shared Library".  It builds a library.


Building from command line
--------------------------

Requires these packages for developers:
- autotools 
- libtool
- automake

To install them:

    sudo apt-get install autotools-dev
    sudo apt-get install libtool
    suto apt-get install automake

Then:

    autoreconf -i
    ./configure
    make
    sudo make install

The -i means "install missing auxiliary files", in this case, create the /m4 directory used by libtool.



Dependencies
------------

Depends on openCV v2 (package opencv-dev, which includes headers and libraries.)


ChangeLog
---------

Version 1 is like the original, just cleaned of cruft and built as a library.

Subsequently:
- restructured the code for ease of reading and modification
- some fixes that might have improved the results
- added ability to accept a color image, but the resulting grayscale saliency map is not better 
than starting from the grayscale of the color image

If you want stable code, you should use the release of version 1.
If you want code you can play with, you should use the latest version.
