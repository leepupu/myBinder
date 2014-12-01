g++ main.cpp src\MyFile.cpp -Iinclude -lgdi32 -o myBinder.exe

g++ -Wl,-subsystem,windows -Iinclude myPutter.cpp src/MyFile.cpp -o myPutter.exe

g++ appender.cpp -o appender.exe

appender.exe myPutter.exe myBinder.exe
