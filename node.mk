# $Id$

include build/node-start.mk

SUBDIRS:= \
	src build doc Contrib

DIST:= \
	GNUmakefile configure \
	ChangeLog README \
	share

include build/node-end.mk
