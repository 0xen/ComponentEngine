SRCPATH=.
prefix=/usr/local
exec_prefix=${prefix}
bindir=${exec_prefix}/bin
libdir=${exec_prefix}/lib
includedir=${prefix}/include
SYS_ARCH=X86_64
SYS=WINDOWS
CC=cl
CFLAGS=-DNDEBUG -O2 -fp:fast -I. -I$(SRCPATH) -nologo -GS- -DHAVE_STRING_H -I$(SRCPATH)/extras 
CFLAGSSO=-DX264_API_EXPORTS 
CFLAGSCLI=
COMPILER=CL
COMPILER_STYLE=MS
DEPMM=-MM
DEPMT=-MT
LD="/c/Program Files (x86)/Microsoft Visual Studio/2019/Enterprise/VC/Tools/MSVC/14.22.27905/bin/HostX64/x64/link.exe" -out:
LDFLAGS=-nologo -incremental:no -Wl,--output-def=libx264-120.def 
LDFLAGSCLI=shell32.lib 
LIBX264=libx264.lib
CLI_LIBX264=$(LIBX264)
AR=lib.exe -nologo -out:
RANLIB=
STRIP=
INSTALL=install
AS=nasm
ASFLAGS= -I. -I$(SRCPATH) -DARCH_X86_64=1 -I$(SRCPATH)/common/x86/ -f win64 -DSTACK_ALIGNMENT=16 -DPIC
RC=rc.exe
RCFLAGS= -nologo -I. -I$(SRCPATH)/extras -fo
EXE=.exe
HAVE_GETOPT_LONG=0
DEVNULL=NUL
PROF_GEN_CC=-GL
PROF_GEN_LD=-LTCG:PGINSTRUMENT
PROF_USE_CC=-GL
PROF_USE_LD=-LTCG:PGOPTIMIZE
HAVE_OPENCL=yes
CC_O=-Fo$@
SONAME=libx264-159.dll
IMPLIBNAME=libx264.dll.lib
SOFLAGS=-dll -implib:$(IMPLIBNAME) 
default: lib-shared
install: install-lib-shared
