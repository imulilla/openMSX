#!/bin/sh

# $Id$

# Generate the Makefiles and configure files
if ( aclocal --version ) </dev/null > /dev/null 2>&1; then
	echo "Building macros."
	aclocal -I m4
else
	echo "aclocal not found -- aborting"
	exit
fi

if ( autoheader --version ) </dev/null > /dev/null 2>&1; then
	echo "Building config header template"
	autoheader
else
	echo "autoheader not found -- aborting"
	exit
fi

if ( autoconf --version ) </dev/null > /dev/null 2>&1; then
	echo "Building configure"
	autoconf
	echo 'run "./configure ; make"'
else
	echo "autoconf not found -- aborting"
	exit
fi

if ( libtoolize --version ) </dev/null > /dev/null 2>&1; then
	echo "Building ltmain.sh"
	libtoolize -f
else
	if ( glibtoolize --version ) </dev/null > /dev/null 2>&1; then
		echo "Building ltmain.sh"
		glibtoolize -f
	else
		echo "(g)libtoolize not found -- aborting"
		exit
	fi
fi

if ( automake --version ) </dev/null > /dev/null 2>&1; then
	echo "Building Makefiles"
	automake -a
else
	echo "automake not found -- aborting"
	exit
fi

rm -f config.cache
