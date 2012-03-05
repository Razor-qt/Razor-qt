cmake_minimum_required(VERSION 2.6)

set(PLUGIN "fsmon")

set(HEADERS
	mon.h
)

set(SOURCES
	mon.cpp
)

set(MOCS
	mon.h
)

set(UIS
)

set(LIBRARIES
  razorqt
  razorqxt
)

#*******************************************
include ("../BuildPlugin.cmake")
BUILD_RAZOR_PLUGIN(${PLUGIN})
