cd 
echo chdir ..\util
chdir %2\util
cd
echo %1 -v
%1 -v
echo %1 -o
%1  -o
echo %1 -p
%1 -p
echo %1 -m
%1 -m
echo %1 -z
%1 -z
echo chdir ..\dat
chdir ..\dat
chdir
echo Generating NetHack database
echo %1 -d
%1 -d
echo Generating rumors
echo %1 -r
%1 -r
echo Generating quests
echo %1 -q
%1 -q
echo Generating oracles
echo %1 -h
%1 -h
echo Generating dungeon.pdf
echo %1 -e
%1 -e
REM NO echo chdir ..\build
REM chdir ..\build
REM '\\#@/*copy ..\win\share\tilemap.c ..\win\share\tiletxt.c*/
