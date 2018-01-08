#ifndef MACRO_H_
#define MACRO_H_
#pragma once

namespace client {
	namespace binding {

#define GETR(Type, MemberName, FaceName) \
  const Type &get##FaceName() const { \
  return MemberName; \
}

#define GETARR(Type, MemberName, FaceName) \
  const Type &get##FaceName(int index) const { \
  return MemberName[index]; \
}

#ifndef IS_NOT_NULL
#define IS_NOT_NULL( x )        ( NULL != (x) )
#endif

#define SAFE_DELETE( x )		if( IS_NOT_NULL( x ) ){ delete x; x = NULL; }
#define SAFE_DELETE_POINT( x )	if( IS_NOT_NULL( x ) ){ delete x; x = NULL; }
#define SAFE_DELETE_POINTS( x )	if( IS_NOT_NULL( x ) ){ delete [] x; x = NULL; }

	}  // namespace binding
}  // namespace client

#endif  // MACRO_H_