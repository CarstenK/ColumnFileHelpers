ColumnFileHelpers
============

A set of simple commandline tools to make life easier when dealing with data that is organized in columns

colsum - A program to calculate the column sum or average of a simple textfile or pipe

rowmerge - A program to merge rows of different files into a single file according to a given column

Requirements
------------

boost (1.52 or higher)  
* lexical_cast
* program_options
* regex

cmake

Compilation
------------

In case the Boost library is not installed in the system path please set the
environment variable BOOST_ROOT to the correct directory. 

For example:
export BOOST_ROOT=/usr/soft/boost 

Then simply type the following commands:

mkdir build  
cd build  
cmake ..  
make

