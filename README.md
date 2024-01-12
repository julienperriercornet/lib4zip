# lib4zip
4zip compression static library

Compression software is that little software brick that supports all the modern IT technology, maintained ungratefully by a guy in Nebraska in his garage for 20 years, as the well known meme says.

Compression is ubiquitous in our modern world. Everytime we load a webpage, we load dozens of compressed files to the point that a significant bottleneck for displaying webpages quickly is the decompression speed of these files. A significant portion of the energy our computers and mobile devices are using is spent on decompressing data, to the point that optimizing that little software brick can make a significant impact on the IT industry's overall CO2 emissions.

I was leading a career where jobs with little to no meaning were successing each other. There's a climate crisis and I brainstormed what was the project with which I could have the biggest impact on our planet and the climate crisis, and a pretty good answer for me was lossless compression software...

That's the motivation behind lib4zip and creating the 4z file format.



As a result lib4zip is a library where the design is sacrificed for SPEED. The usage of C language, no exceptions, using generic algorithms from the STL is strictly forbidden, systematically use aligned memory allocations, ZERO dynamic memory allocations are allowed during compression and decompression...

This leads to rockblock solid software that can't fail while compressing or decompressing. If there isn't enough memory on the system it will immediatelly fail on context allocation.

lib4zip currently uses an arithmetic encoder because all patents on that technology have already expired, unlike ANS. In the fastest compression modes the library won't use arithmetic encoding and fallback to classic bit I/O.

lib4zip is currently single threaded, but there are plans to make it multithreaded as well down the line.

After multithreading support is done, there are plans to use vectorization to speed up the software as well.

Both of these features will have a significant impact on performance (maybe x40 on a typical 16 core CPU with AVX2?) so currently any performance bench must be made with that in mind that the library is single threaded and not vectorized yet.

lib4zip is aiming at all the spectrum of lossless compression usages: very fast compression/decompression with moderate compression ration, and at the upper end of the spectrum competitive compression.
