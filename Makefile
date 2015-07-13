.PHONY : all doc

all : ex1 ex2

ex1 ex2 : *.cpp cppexpat.hpp
	$(CXX) -I. -std=c++11 $@.cpp -lexpat -o $@

doc : cppexpat.hpp
	doxygen
