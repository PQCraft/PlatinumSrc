ver="$(grep '#define PSRC ' src/psrc.h | sed 's/#define .* //')"
verstr="Build $ver"

mkrel() {
    echo "Making $2..."
    rm -f "$2"
    $1 $4 1> /dev/null || exit 1
    zip -r "$2" $3 1> /dev/null || exit 1
    $1 $5 1> /dev/null
}

make clean
wine make clean

mkrel "make" "PlatinumSrc-Linux-x86_64.zip" "config/ resources/ psrc" "CFLAGS=-mtune=generic -j8 build" "clean"
mkrel "wine make" "PlatinumSrc-Windows-x86_64.zip" "config/ resources/ psrc.exe" "CFLAGS=-mtune=generic -j8 build" "clean"

git add */ Makefile README.md LICENSE *.sh
git commit -S -m "$verstr"
git push
git tag -s "$ver" -m "$verstr"
gh release create "$ver" --title "$verstr" --notes "$verstr" *.zip
