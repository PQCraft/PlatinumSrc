make clean
git add */ Makefile README.md LICENSE *.sh
git commit -S -m "Build $(grep '#define PSRC ' src/psrc.h | sed 's/#define .* //')"
git push
