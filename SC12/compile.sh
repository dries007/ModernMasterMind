#!/usr/bin/sh

# Setup env
export WATCOM=/opt/watcom
export PATH=$WATCOM/binl:$PATH
export BECKCLIB=/opt/beck
export INCLUDE=$WATCOM/h:$BECKCLIB/includes
export LIB=$WATCOM/lib286/dos:$WATCOM/lib286:$BECKCLIB/lib
#export EDPATH=$WATCOM/eddat
#export WIPFC=$WATCOM/wipfc

# Compile script
rm -r watcomTEMP
mkdir watcomTEMP
cp *.c watcomTEMP
cp *.h watcomTEMP
cd watcomTEMP

echo "All C files:"
echo "------------"
find -name '*.c' -exec basename {} \;
echo "------------"

wcl -w4 -s -zp1 -d0 -od -fpr -zu -1 -ml -za99 -bcl=dos -bt=dos -lr -l=dos -i=$INCLUDE *.c clib260h.lib -fe=main.exe

#wcc -w4 -s -zp1 -d0 -fpi -zu -1 -ml -za99 *.c
#wlink format dos libpath $WATCOM/lib286/dos:$WATCOM/lib286:$BECKCLIB/lib file *.o library clib260h.lib

if ls *.exe 1> /dev/null 2>&1; then
	echo "All EXE files:"
	echo "------------"
	find -name '*.exe' -exec basename {} \;
	echo "------------"
else
	echo "NO EXE FILES. Error"
	exit 1
fi

find -name '*.exe' -exec basename {} \; > AUTOEXEC.BAT

ftp -pvn <<EOF
open 192.168.0.10
user ftp ftp
binary
put *.exe
put AUTOEXEC.BAT
EOF

{
	echo "tel";
	echo "tel";
#	find -name '*.exe' -exec basename {} \;;
#	echo "closetelnet";
	echo "reboot";
	sleep 1;
} | telnet 192.168.0.10

cd ..