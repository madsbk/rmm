#!/bin/bash
# Copyright (c) 2023, NVIDIA CORPORATION.
##############################
# RMM doxygen warnings check #
##############################

# skip if doxygen is not installed
if ! [ -x "$(command -v doxygen)" ]; then
  echo -e "warning: doxygen is not installed"
  exit 0
fi

# Utility to return version as number for comparison
function version { echo "$@" | awk -F. '{ printf("%d%03d%03d%03d\n", $1,$2,$3,$4); }'; }

# doxygen supported version 1.9.1
DOXYGEN_VERSION=`doxygen --version`
if [ ! $(version "$DOXYGEN_VERSION") -eq $(version "1.9.1") ] ; then
  echo -e "warning: Unsupported doxygen version $DOXYGEN_VERSION"
  echo -e "Expecting doxygen version 1.9.1"
  exit 0
fi

export RAPIDS_VERSION="$(sed -E -e 's/^([0-9]{2})\.([0-9]{2})\.([0-9]{2}).*$/\1.\2.\3/' VERSION)"
export RAPIDS_VERSION_MAJOR_MINOR="$(sed -E -e 's/^([0-9]{2})\.([0-9]{2})\.([0-9]{2}).*$/\1.\2/' VERSION)"

# Run doxygen, ignore missing tag files error
TAG_ERROR1="error: Tag file '.*.tag' does not exist or is not a file. Skipping it..."
TAG_ERROR2="error: cannot open tag file .*.tag for writing"
DOXYGEN_STDERR=`cd doxygen && { cat Doxyfile ; echo QUIET = YES; echo GENERATE_HTML = NO; }  | doxygen - 2>&1 | sed "/\($TAG_ERROR1\|$TAG_ERROR2\)/d"`
RETVAL=$?

if [ "$RETVAL" != "0" ] || [ ! -z "$DOXYGEN_STDERR" ]; then
  echo -e "$DOXYGEN_STDERR"
  RETVAL=1 #because return value is not generated by doxygen 1.8.20
fi

exit $RETVAL
