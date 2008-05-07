#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "alldefs.h"
#include "alloc.h"
#include <pcre.h>


object compile_pcre(object pattern){
/*	 pcre *pcre_compile(const char *pattern, int options,
            const char **errptr, int *erroffset,
            const unsigned char *tableptr);
*/
	pcre *re;
	const char *error;
	int erroffset;
	char* str;
	object ret;
	
	str = EMalloc( SEQ_PTR(pattern)->length + 1);
	MakeCString( str, pattern );
	re = pcre_compile( str, 0, &error, &erroffset, NULL );
	if( re == NULL ){
		// error, so pass the error string to caller
		return NewString( error );
	}
	EFree( str );
	
	if ((unsigned) re > (unsigned)MAXINT)
		ret = NewDouble((double)(unsigned long)re);
	else
		ret = (unsigned long)re;
	
	return re;
}

object exec_pcre(object x ){
//        int pcre_exec(const pcre *code, const pcre_extra *extra,
//             const char *subject, int length, int startoffset,
//             int options, int *ovector, int ovecsize);
	int rc;
	int ovector[30];
	pcre* re;
	char* str;
	s1_ptr s;
	s1_ptr sub;
	int i, j;
	object pcre_ptr;
	
	// x[1] = pcre ptr
	// x[2] = string to search
	pcre_ptr = SEQ_PTR(x)->base[1];
	if( IS_ATOM_INT( pcre_ptr ) )
		re = (pcre*) pcre_ptr;
	else
		re = (pcre*) (int) DBL_PTR( pcre_ptr )->dbl;
	
	sub = SEQ_PTR(SEQ_PTR(x)->base[2]);
	str = EMalloc(sub->length+1);
	MakeCString( str, SEQ_PTR(x)->base[2] );
	
	rc = pcre_exec( re, NULL, str, ((s1_ptr)SEQ_PTR(SEQ_PTR(x)->base[2]))->length,
		0, 0, ovector, 30 );
	EFree( str );
	if( rc <= 0 ) return rc;
	
	// put the substrings into sequences
	s = NewS1( rc );
	
	for( i = 1, j=0; i <= rc; i++ ){
		sub = NewS1( 2 );
		sub->base[1] = ovector[j++] + 1;
		sub->base[2] = ovector[j] > 0 ? ovector[j] - 1 : 0;
		j++;
		s->base[i] = MAKE_SEQ( sub );
	}
	return MAKE_SEQ( s );
}
