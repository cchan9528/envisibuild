# Cascade Classifier Training

- Classifier trained to determine Y/N about a region of interest (ROI)
    - (Y)es if region likely to show desired object
    - (N)o if region unlikely to

- Image is searched with a classifier
    - Move search window across image
    - Classfier itself is easily scalable for image size
        - to find an object of an unknown size, scans should occur at different scales

- "Cascade" - multiple simpler classifiers applied to same ROI
    - at some point, candidate may be rejected
    - else, ROI holds candidacy

- "Boosted" - classifiers at every stage are complex
    - "Boosting" - weighted voting
    - Simple boosting techniques in OpenCV:
        - Discrete Adaboost
        - Real Adaboost
        - Gentle Adaboost
        - Logitboost
