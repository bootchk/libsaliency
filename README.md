libsaliency
===========

A library implementing an image processing algorithm that computes a saliency map.

Derived from https://github.com/kkduncan/SaliencyDetection.git   
The algorithm is the work of  "Relational Entropy-Based Saliency Detection in Images and Videos" by K. Duncan and S. Sarkar in the IEEE International Conference on Image Processing (2012).
 
I just cleaned up that repository a little, and changed it to build a library as well as an example program.
 
Building
--------

The .project is an Eclipse IDE project.  Requires JRE (Java runtime) and Eclipse C/C++ IDE.

The nature of the project is "C/C++ Autotools Shared Library".  It builds a library.

Requires autotools package to build.

Dependencies
------------

Depends on openCV v2 (package opencv-dev, which includes headers and libraries.)
