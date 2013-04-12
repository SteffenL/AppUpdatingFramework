@echo off
pushd src
cloc.exe --strip-comments=temp_nc --out=..\line_count.txt --quiet --exclude-dir=cloc_out --match-f=\.[hc](pp)?$ . >nul
del /q *.temp_nc
popd
