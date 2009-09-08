%{
/*
 * $Id: lex-parse.l 233 2007-04-04 09:52:54Z ludovic.rousseau $
 *
 * Copyright (C) 2002
 *  Antti Tapaninen <aet@cc.hut.fi>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include "nfcconf.h"
#include "internal.h"

static nfcconf_parser *parser;

%}

%option noyywrap
%option nounput

%%

"#"[^\r\n]*	nfcconf_parse_token(parser, TOKEN_TYPE_COMMENT, yytext);

\n		nfcconf_parse_token(parser, TOKEN_TYPE_NEWLINE, NULL);

[ \t\r]+	/* eat up whitespace */

[,{}=;]		nfcconf_parse_token(parser, TOKEN_TYPE_PUNCT, yytext);

\"[^\"\n\r]*\r*[\"\n] nfcconf_parse_token(parser, TOKEN_TYPE_STRING, yytext);

[^;, \t\r\n]+  	nfcconf_parse_token(parser, TOKEN_TYPE_STRING, yytext);

%%

#ifndef YY_CURRENT_BUFFER_LVALUE
# define YY_CURRENT_BUFFER_LVALUE	yy_current_buffer
#endif

static void do_lex(nfcconf_parser *p)
{
	parser = p;

	yylex();

#if 1
	/* For non-reentrant C scanner only. */
	if (YY_CURRENT_BUFFER) {
		yy_delete_buffer(YY_CURRENT_BUFFER);
		YY_CURRENT_BUFFER_LVALUE = NULL;
		yy_init = 1;
		yy_start = 0;
	}
#endif
}

int nfcconf_lex_parse(nfcconf_parser *p, const char *filename)
{
	yyin = fopen(filename, "r");
	if (yyin == NULL)
		return 0;

	do_lex(p);

	fclose(yyin);
	yyin = NULL;
	return 1;
}

int nfcconf_lex_parse_string(nfcconf_parser *p, const char *conf_string)
{
	yy_scan_string(conf_string);
	do_lex(p);
	return 1;
}
