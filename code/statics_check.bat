@echo off

set CheckTypes=*.h *.cpp *.inl *.c

echo STATICS:
findstr -s -n -i -r "static" %CheckTypes% | findstr /v "#define"

echo ------

echo GLOBALS:
findstr -s -n -i -r "local_persist" %CheckTypes% | findstr /v "#define"
findstr -s -n -i -r "global_variable" %CheckTypes% | findstr /v "#define"
