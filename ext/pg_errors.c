/*
 * pg_errors.c - Definition and lookup of error classes.
 *
 */

#include "pg.h"

VALUE rb_hErrors;
VALUE rb_ePGerror;
VALUE rb_eServerError;

static VALUE
define_error_class(const char *name, const char *baseclass_code)
{
	VALUE baseclass = rb_eServerError;
	if(baseclass_code)
	{
		baseclass = rb_hash_aref( rb_hErrors, rb_str_new2(baseclass_code) );
	}
	return rb_define_class_under( rb_mPG, name, baseclass );
}

static void
register_error_class(const char *code, VALUE klass)
{
	rb_hash_aset( rb_hErrors, rb_str_new2(code), klass );
}

/* Find a proper error class for the given SQLSTATE string
 */
VALUE
lookup_error_class(const char *sqlstate)
{
	VALUE klass;

	if(sqlstate)
	{
		/* Find the proper error class by the 5-characters SQLSTATE. */
		klass = rb_hash_aref( rb_hErrors, rb_str_new2(sqlstate) );
		if(NIL_P(klass))
		{
			/* The given SQLSTATE couldn't be found. This might happen, if
			 * the server side uses a newer version than the client.
			 * Try to find a error class by using the 2-characters SQLSTATE.
			 */
			klass = rb_hash_aref( rb_hErrors, rb_str_new(sqlstate, 2) );
			if(NIL_P(klass))
			{
				/* Also the 2-characters SQLSTATE is unknown.
				 * Use the generic server error instead.
				 */
				klass = rb_eServerError;
			}
		}
	}
	else
	{
		/* Unable to retrieve the PG_DIAG_SQLSTATE.
		 * Use the generic error instead.
		 */
		klass = rb_ePGerror;
	}

	return klass;
}

void
init_pg_errors()
{
	rb_hErrors = rb_hash_new();
	rb_define_const( rb_mPG, "Errors", rb_hErrors );

	rb_ePGerror = rb_define_class_under( rb_mPG, "Error", rb_eStandardError );

	/*************************
	 *  PG::Error
	 *************************/
	rb_define_alias( rb_ePGerror, "error", "message" );
	rb_define_attr( rb_ePGerror, "connection", 1, 0 );
	rb_define_attr( rb_ePGerror, "result", 1, 0 );

	rb_eServerError = rb_define_class_under( rb_mPG, "ServerError", rb_ePGerror );

	#include "errorcodes.def"
}