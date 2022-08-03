CXX=clang++
CXXFLAGS=-g -Wall -Wextra -std=c++11
INCLUDES=`pkg-config --cflags gtkmm-3.0 webkit2gtk-4.0`
LDFLAGS=`pkg-config --libs gtkmm-3.0 webkit2gtk-4.0`

all:
	$(CXX) navigator.cpp $(CXXFLAGS) $(INCLUDES) -o bin/navigator $(LDFLAGS)

tidy:
	clang-tidy -checks="-*,bugprone-*,misc-*,modernize-*,performance-*,readability-*,-readability-implicit-bool-conversion,-readability-magic-numbers,-modernize-use-trailing-return-type" navigator.cpp -- $(INCLUDES)
