#include "cap_fileio.h"

#define IS_NULL( x )            ( x == NULL )
#define IS_NOT_NULL( x )        ( x != NULL )
#define IS_ZERO( x )            ( x == 0 )
#define SAFE_DELETE( x )		if( IS_NOT_NULL( x ) ){ delete x; x = NULL; }
#define SAFE_DELETE_POINT( x )	if( IS_NOT_NULL( x ) ){ delete x; x = NULL; }
#define SAFE_DELETE_POINTS( x )	if( IS_NOT_NULL( x ) ){ delete [] x; x = NULL; }