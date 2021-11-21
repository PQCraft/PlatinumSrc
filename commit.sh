ver="$(grep '#define PSRC ' src/psrc.h | sed 's/#define .* //')"
verstr="Build $ver"
make clean
git add */ Makefile README.md LICENSE *.sh
git commit -S -m "$verstr"
git push
git tag -s "$ver"
gh release create "$ver" --title "$verstr"
