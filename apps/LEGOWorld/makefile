# LEGOWorld makefile
# Note: make more targets to stay more conservative to,
#	really, be more agressive in increasing build time

CXX = g++
CXXFLAGS = -g -Wall
OBJ = main.o legoworld.o
LIBDIR = /usr/local/lib
LIBS = -lopencv_calib3d -lopencv_contrib -lopencv_core -lopencv_features2d\
	   -lopencv_flann -lopencv_gpu -lopencv_highgui -lopencv_imgproc\
	   -lopencv_legacy -lopencv_ml -lopencv_nonfree -lopencv_objdetect\
	   -lopencv_ocl -lopencv_photo -lopencv_stitching -lopencv_superres\
	   -lopencv_ts -lopencv_video -lopencv_videostab
OPENCVSRC = /usr/local/include/

all: LEGOWorld

LEGOWorld: $(OBJ)
	$(CXX) -L$(LIBDIR) $(LIBS) -o $@ $(OBJ)

main.o: main.cpp legoworld.h	# Depends on legoworld.h, but don't compile
	$(CXX) $(CXXFLAGS) -I$(OPENCVSRC) -o $@ -c $<

legoworld.o: legoworld.cpp legoworld.h
	$(CXX) $(CXXFLAGS) -I$(OPENCVSRC) -o $@ -c $<

clean:
	rm -r *.o ./LEGOWorld
