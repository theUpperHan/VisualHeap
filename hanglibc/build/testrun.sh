#!/bin/bash
builddir=`dirname "$0"`
GCONV_PATH="${builddir}/iconvdata"

usage () {
cat << EOF
Usage: $0 [OPTIONS] <program> [ARGUMENTS...]

  --tool=TOOL  Run with the specified TOOL. It can be strace, rpctrace,
               valgrind or container. The container will run within
               support/test-container.
EOF

  exit 1
}

toolname=default
while test $# -gt 0 ; do
  case "$1" in
    --tool=*)
      toolname="${1:7}"
      shift
      ;;
    --*)
      usage
      ;;
    *)
      break
      ;;
  esac
done

if test $# -eq 0 ; then
  usage
fi

case "$toolname" in
  default)
    exec   env GCONV_PATH="${builddir}"/iconvdata LOCPATH="${builddir}"/localedata LC_ALL=C  "${builddir}"/elf/ld-linux-x86-64.so.2 --library-path "${builddir}":"${builddir}"/math:"${builddir}"/elf:"${builddir}"/dlfcn:"${builddir}"/nss:"${builddir}"/nis:"${builddir}"/rt:"${builddir}"/resolv:"${builddir}"/mathvec:"${builddir}"/support:"${builddir}"/crypt:"${builddir}"/nptl ${1+"$@"}
    ;;
  strace)
    exec strace  -EGCONV_PATH=/home/uprhan/hanglibc/build/iconvdata  -ELOCPATH=/home/uprhan/hanglibc/build/localedata  -ELC_ALL=C  /home/uprhan/hanglibc/build/elf/ld-linux-x86-64.so.2 --library-path /home/uprhan/hanglibc/build:/home/uprhan/hanglibc/build/math:/home/uprhan/hanglibc/build/elf:/home/uprhan/hanglibc/build/dlfcn:/home/uprhan/hanglibc/build/nss:/home/uprhan/hanglibc/build/nis:/home/uprhan/hanglibc/build/rt:/home/uprhan/hanglibc/build/resolv:/home/uprhan/hanglibc/build/mathvec:/home/uprhan/hanglibc/build/support:/home/uprhan/hanglibc/build/crypt:/home/uprhan/hanglibc/build/nptl ${1+"$@"}
    ;;
  rpctrace)
    exec rpctrace  -EGCONV_PATH=/home/uprhan/hanglibc/build/iconvdata  -ELOCPATH=/home/uprhan/hanglibc/build/localedata  -ELC_ALL=C  /home/uprhan/hanglibc/build/elf/ld-linux-x86-64.so.2 --library-path /home/uprhan/hanglibc/build:/home/uprhan/hanglibc/build/math:/home/uprhan/hanglibc/build/elf:/home/uprhan/hanglibc/build/dlfcn:/home/uprhan/hanglibc/build/nss:/home/uprhan/hanglibc/build/nis:/home/uprhan/hanglibc/build/rt:/home/uprhan/hanglibc/build/resolv:/home/uprhan/hanglibc/build/mathvec:/home/uprhan/hanglibc/build/support:/home/uprhan/hanglibc/build/crypt:/home/uprhan/hanglibc/build/nptl ${1+"$@"}
    ;;
  valgrind)
    exec env GCONV_PATH=/home/uprhan/hanglibc/build/iconvdata LOCPATH=/home/uprhan/hanglibc/build/localedata LC_ALL=C valgrind  /home/uprhan/hanglibc/build/elf/ld-linux-x86-64.so.2 --library-path /home/uprhan/hanglibc/build:/home/uprhan/hanglibc/build/math:/home/uprhan/hanglibc/build/elf:/home/uprhan/hanglibc/build/dlfcn:/home/uprhan/hanglibc/build/nss:/home/uprhan/hanglibc/build/nis:/home/uprhan/hanglibc/build/rt:/home/uprhan/hanglibc/build/resolv:/home/uprhan/hanglibc/build/mathvec:/home/uprhan/hanglibc/build/support:/home/uprhan/hanglibc/build/crypt:/home/uprhan/hanglibc/build/nptl ${1+"$@"}
    ;;
  container)
    exec env GCONV_PATH=/home/uprhan/hanglibc/build/iconvdata LOCPATH=/home/uprhan/hanglibc/build/localedata LC_ALL=C  /home/uprhan/hanglibc/build/elf/ld-linux-x86-64.so.2 --library-path /home/uprhan/hanglibc/build:/home/uprhan/hanglibc/build/math:/home/uprhan/hanglibc/build/elf:/home/uprhan/hanglibc/build/dlfcn:/home/uprhan/hanglibc/build/nss:/home/uprhan/hanglibc/build/nis:/home/uprhan/hanglibc/build/rt:/home/uprhan/hanglibc/build/resolv:/home/uprhan/hanglibc/build/mathvec:/home/uprhan/hanglibc/build/support:/home/uprhan/hanglibc/build/crypt:/home/uprhan/hanglibc/build/nptl /home/uprhan/hanglibc/build/support/test-container env GCONV_PATH=/home/uprhan/hanglibc/build/iconvdata LOCPATH=/home/uprhan/hanglibc/build/localedata LC_ALL=C  /home/uprhan/hanglibc/build/elf/ld-linux-x86-64.so.2 --library-path /home/uprhan/hanglibc/build:/home/uprhan/hanglibc/build/math:/home/uprhan/hanglibc/build/elf:/home/uprhan/hanglibc/build/dlfcn:/home/uprhan/hanglibc/build/nss:/home/uprhan/hanglibc/build/nis:/home/uprhan/hanglibc/build/rt:/home/uprhan/hanglibc/build/resolv:/home/uprhan/hanglibc/build/mathvec:/home/uprhan/hanglibc/build/support:/home/uprhan/hanglibc/build/crypt:/home/uprhan/hanglibc/build/nptl ${1+"$@"}
    ;;
  *)
    usage
    ;;
esac
