#! /usr/bin/env bash

get_absname() {
  if [ -d "$(dirname "$1")" ]; then
    echo "$(cd "$(dirname "$1")" && pwd)/$(basename "$1")"
  fi
}

MATLAB=`which matlab`
OUTPUT="out.log"
USAGE="Usage: `basename -- $0` [-h] [-d output-library] [-n library-name] [-b library-name-in-browser] -f formulae-file"

LIBDIR="monitor_lib"

while getopts hd:n:b:f: OPT; do
  case "$OPT" in
    h)
      echo -e $USAGE
      echo ""
      exit 0
      ;;
    d)
      LIBDIR="$(get_absname $OPTARG)"
      ;;
    n)
      LIBNAME=$OPTARG
      ;;
    b)
      BROWNAME=$OPTARG
      ;;
    f)
      FORMULAFILE="$(get_absname $OPTARG)"
      ;;
    \?)
      # getopts issues an error message
      echo -e $USAGE
      echo ""
      exit 1
      ;;
  esac
done

# assign default value to the library name
if [ "$LIBNAME" == "" ]
then
  LIBNAME=$(basename -- $LIBDIR)
fi

# assign default value to the library browser name
if [ "$BROWNAME" == "" ]
then
  BROWNAME=$LIBNAME
fi

# check if the formula file was given
if [ "$FORMULAFILE" == "" ]
then
  echo 'Please insert the name of the file containing the formulae'
  exit 1
fi

# check if matlab variable is defined
if [ "$MATLAB" == "" ]
then
    echo 'Please install MATLAB or add the matlab binary to PATH'
    exit 1
fi

# set current directory to the script directory
cd -P -- "$(dirname -- "$0")/src"

# execute system
"$MATLAB" -nodesktop -nosplash -nodisplay -r "bin.libgen('$FORMULAFILE','$LIBDIR','$LIBNAME','$BROWNAME');quit;" 

