
# very basic makefile for regression tests
#
# Visual C++ 6
#
#
# Add additional compiler options here:
#
CXXFLAGS=
#
# Add additional include directories here:
#
INCLUDES=
#
# add additional linker flags here:
#
XLFLAGS=
#
# sources to compile for each test:
#
SOURCES=*.cpp 

CFLAGS= $(INCLUDES) /Zm400 /O2 /GB /GF /Gy -GX -GR -I..\..\..\..\ $(CXXFLAGS) /DBOOST_LIB_DIAGNOSTIC=1 /Zc:wchar_t

LFLAGS= -link /LIBPATH:..\..\..\..\stage\lib /LIBPATH:..\..\build\vc7 $(XLFLAGS)

all :: r1-vc7.exe r2-vc7.exe r3-vc7.exe r4-vc7.exe r5-vc7.exe r6-vc7.exe r7-vc7.exe r8-vc7.exe
	r1-vc7
	r2-vc7
	r3-vc7
	r4-vc7
	r5-vc7
	r6-vc7
	-copy ..\..\build\vc7\boost_regex*.dll
	-copy ..\..\..\..\stage\lib\boost_regex*.dll
	r7-vc7
	r8-vc7

r1-vc7.exe : 
	cl /ML $(CFLAGS) /O2 -o r1-vc7.exe $(SOURCES) $(LFLAGS)

r2-vc7.exe : 
	cl /MLd $(CFLAGS) -o r2-vc7.exe $(SOURCES) $(LFLAGS)

r3-vc7.exe : 
	cl /MT $(CFLAGS) /O2 -o r3-vc7.exe $(SOURCES) $(LFLAGS)

r4-vc7.exe : 
	cl /MTd $(CFLAGS) -o r4-vc7.exe $(SOURCES) $(LFLAGS)

r5-vc7.exe : 
	cl /MD $(CFLAGS) /O2 -o r5-vc7.exe $(SOURCES) $(LFLAGS)

r6-vc7.exe : 
	cl /MDd $(CFLAGS) -o r6-vc7.exe $(SOURCES) $(LFLAGS)

r7-vc7.exe : 
	cl /MD $(CFLAGS) /O2 /DBOOST_ALL_DYN_LINK -o r7-vc7.exe $(SOURCES) $(LFLAGS)

r8-vc7.exe : 
	cl /MDd $(CFLAGS) /DBOOST_ALL_DYN_LINK -o r8-vc7.exe $(SOURCES) $(LFLAGS)


