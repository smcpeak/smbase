# smbase/sm-lib.mk
# Library of useful GNU Makefile macros, etc.


# Given a file name in $1, yield a related string suitable for use as
# part of a Makefile variable name.  Specifically, this strips off any
# directory and extension, then replaces hyphens with underscores.
#
# This is typically used in a construction like:
#
#   $(OPTIONS_FOR_$(call FILENAME_TO_VARNAME,$*))
#
# to construct the name of a variable that may or may not exist, then
# retrieving its value (or the empty string if it does not exit), thus
# providing a way to optionally customize the behavior of a given
# target when operating on a particular file.
#
define FILENAME_TO_VARNAME
$(subst -,_,$(basename $(notdir $1)))
endef


# EOF
