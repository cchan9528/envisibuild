# Object Detection with Haar-Like Features

### Description
- Haar-like features are features (kernels) that look like Haar wavelets
    - Haar wavelet is box wave
        ![Image of Haar Wavelet](https://upload.wikimedia.org/wikipedia/commons/thumb/a/a0/Haar_wavelet.svg/500px-Haar_wavelet.svg.png)
    - Haar-like features are thus boxes of boxes
        ![Image of Haar-like Features](http://fileadmin.cs.lth.se/graphics/theses/projects/facerecognition/1_all_haar_wavelets.png)


- Haar features by **themselves are weak** learners/classfiers
    - These **must be "cascaded"** to describe an object w/ sufficient accuracy
        - i.e. use a lot of them in the same ROI

### Application
- Haar-like feature detection is used for its computational efficiency
    - Essentially, each kernel supplies weighted sums for a ROI
    - This can be done **quick**ly b/c of **integral images**
        - Idea: create an intensity matrix with sums of upper-left pixels from any point
        - Drastically decreases amt of lookups to find sum under any one area
            - \#lookups = \# vertices for each kernel
                - 2-rect feature ~ 6 lookups (indicies)
                - 3-rect feature ~ 8 lookups
                - 4-rect feature ~ 9 lookups
