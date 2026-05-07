windres app.rc -O coff -o app.res
g++ main.cpp app.res -o ConfigEditor.exe -mwindows -static