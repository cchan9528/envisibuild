# HSVFinder makefile

CXX = g++
CXXFLAGS = -g -Wall
OBJ = HSVFinder.o
LIBDIR = /usr/local/lib
LIBS = -lopencv_calib3d -lopencv_contrib -lopencv_core -lopencv_features2d\
	   -lopencv_flann -lopencv_gpu -lopencv_highgui -lopencv_imgproc\
	   -lopencv_legacy -lopencv_ml -lopencv_nonfree -lopencv_objdetect\
	   -lopencv_ocl -lopencv_photo -lopencv_stitching -lopencv_superres\
	   -lopencv_ts -lopencv_video -lopencv_videostab
OPENCVSRC = /usr/local/include/

all: HSVFinder

HSVFinder: $(OBJ)
	$(CXX) -L$(LIBDIR) $(LIBS) -o $@ $(OBJ)

HSVFinder.o: HSVFinder.cpp
	$(CXX) $(CXXFLAGS) -I$(OPENCVSRC) -o $@ -c $<

clean:
	rm -r *.o ./HSVFinder ./HSVFinder.dSYM
