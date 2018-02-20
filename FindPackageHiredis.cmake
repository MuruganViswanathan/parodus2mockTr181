# Find hiredis opensourse package include & library path 
#
FIND_PATH(HIREDIS_INCLUDE 
	hiredis.h 
	/usr/local/include/hiredis)
	
FIND_LIBRARY(HIREDIS_LIBRARY 
	NAMES hiredis 
	PATHS /usr/lib /usr/local/lib )

INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(HIREDIS DEFAULT_MSG 
	HIREDIS_INCLUDE 
	HIREDIS_LIBRARY)

MARK_AS_ADVANCED(HIREDIS_LIBRARY HIREDIS_INCLUDE)
