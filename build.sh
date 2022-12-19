if [ $# -ne 1 ]; then
    >&2 echo "usage: $0 [release|debug|install]"
    exit -1
fi

ex() {
    echo $@
    $@
}

case $1 in
  release)
    ex clang++ bytecode-decomp.cpp clazz/clazz.cpp -DFMT_HEADER_ONLY -std=c++20 -O3 -Wall -o bytecode-decomp
    ex strip bytecode-decomp
    ;;
  release-symbols)
    ex clang++ bytecode-decomp.cpp clazz/clazz.cpp -DFMT_HEADER_ONLY -std=c++20 -O3 -Wall -o bytecode-decomp
    ;;
  debug)
    ex clang++ bytecode-decomp.cpp clazz/clazz.cpp -DFMT_HEADER_ONLY -std=c++20 -fsanitize=address,undefined -ggdb -O0 -Wall -o bytecode-decomp
    ;;
  install)
    ex clang++ bytecode-decomp.cpp clazz/clazz.cpp -DFMT_HEADER_ONLY -std=c++20 -O3 -Wall -o bytecode-decomp
    ex strip bytecode-decomp
    ex install bytecode-decomp /usr/local/bin/
    ;;
  *)
    >&2 echo "usage: $0 [release|debug|install]"
    exit -1
    ;;
esac

