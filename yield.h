/* Yield in C/C++ with GNU C Extensions
 *
 * (C) Copyright 2011, Mauricio Faria de Oliveira
 * http://mfoliveira.org
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/* Yield Pointer: variable name for storing a re-entry point */
#define YIELD_POINTER __yield_pointer

/* Yield Start: 
 * 	If last return is a 'yield' return,
 * 	go to re-entry point.
 */
#define yield_start()				\
	  if ( YIELD_POINTER != NULL )		\
	    goto *YIELD_POINTER

/* Yield End:
 * 	Set return as 'non-yield' return.
 */
#define yield_end()				\
	  YIELD_POINTER = NULL

/* Yield Point:
 * 	Create a label for a re-entry point.
 * 
 * 	The extra underscored macros are for
 * 	correct __LINE__ evaluation and concatenation.
 */
#define __yield_point(x, y) 			\
	  x ## y

#define _yield_point(x, y) 			\
	  __yield_point(x, y)

#define yield_point				\
	  _yield_point(YIELD_POINT_,  __LINE__)

/* Yield:
 * 	The trick.
 * 
 * 	Set return as 'yield' return,
 * 	creating a label after the return statement
 * 	and storing its address for re-entry point.
 */
#define yield(expression)			\
do {						\
	  YIELD_POINTER = && yield_point;	\
	  return expression;			\
	  yield_point: ;			\
} while (0)
