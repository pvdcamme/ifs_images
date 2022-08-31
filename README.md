# ifs_images
A little playground for iterated function systems. These
are fractals not unlike the more popular [Mandelbrot](https://en.wikipedia.org/wiki/Mandelbrot_set).

These iterated functions are somewhat fun to program. The results
tend to be fairly psychedlic (and of very little practical use).

![example_image](examples/example_1.jpg)

# Building and Running 

This code requires:
* libjpeg-dev for the images
* g++ for building
* an x86-64 due to platform specfic optimization.

Luckily these are (still) fairly common in 2022.

Following commands will first build and next run the program.

```
$ make 
$./ifs example.jpg
```

On newer hardware it takes about 10s for "example.jpg" to be 
saved to disk, thereafter the program keeps improving the image 
iteratively with updated versions overwritting this same file.
