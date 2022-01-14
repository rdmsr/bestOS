#!/bin/bash

set -e

OPTIONS="rebuild-mlibc build-sysroot"

usage()
{
  echo "Usage: $0 [options...]"
  echo ""
  echo "Options:"
  echo "    -a, --all      Build tools, install the tools and build the system"
  echo "    -c, --clean    Do a clean build"
  echo "    -b, --build    Build the system"
  echo "    -r, --run      Run the system in QEMU"
  echo "    -i, --install  Install everything to the system root"
  echo "    -t, --tools    Build tools required in a directory called sysroot"
  echo "Tools:"
  echo "    $TOOLS"
  echo ""
  exit 1
}

eval_arg()
{
  case $1 in
    "-a" | "--all")
      mkdir -p sysroot && cd sysroot && xbstrap init .. && xbstrap install -u --all && cd ..
      make -j
      mkdir -p src/root/usr/lib
      cp sysroot/system-root/usr/lib/* src/root/usr/lib
      ;;
    "-b" | "--build")
       make -j
      ;;

    "-c" | "--clean")
      make clean
      ;;

    "-h" | "--help")
      usage
      ;;

    "-r" | "--run")
      make run
      ;;

    "-t" | "--tools")
      mkdir -p sysroot && cd sysroot && xbstrap init .. && xbstrap install -u --all
      ;;

    "-i" | "--install")
      mkdir -p src/root/usr/lib
      cp sysroot/system-root/usr/lib/* src/root/usr/lib
      ;;

    *)
      echo "Unknown option '$1'!"
      echo ""
      usage
      ;;
  esac
}


if [ -z "$1" ]; then
  usage
  exit 1
fi

while [[ $1 == -* ]]; do
  eval_arg $1
  shift
done
