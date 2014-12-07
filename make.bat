g++ main.cpp src\MyFile.cpp -Iinclude -ImyZlib/include -LmyZlib/lib -lz -lgdi32 -o myBinder.exe

windres my.rc -O coff -o my.res

g++ -c -Iinclude -ImyZlib/include -LmyZlib/lib myPutter.cpp -lz -o myPutter.o

g++ -c -Iinclude src/MyFile.cpp -o myFile.o

g++ -Wl,-subsystem,windows -Iinclude -o myPutter.exe myPutter.o myFile.o -LmyZlib/lib -lz my.res

g++ appender.cpp -o appender.exe

appender.exe myPutter.exe myBinder.exe
