/*
 * Copyright (c) 2017-2018 Simon Schmidt
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "parser.h"

static void parser_parse_args(struct tokenizer *t,lua_State* L){
	OutputStream dest = OutputStream_new();
	int c = 0;
	sds s = 0;
	for(;;){
		s = tokenize_get(t,(OutputStream)0);
		if(!s) {
			fprintf(stderr,"unexpected EOF\n");
			abort();
		}
		switch(*s){
		case '(':
			c++;
			break;
		case ')':
			if(!c){
				sdsfree(s);
				goto end;
			}
			c--;
			break;
		case ',':
			if(!c){
				luab_pushsds(L,(sds)(dest->data) );
				sdssetlen((sds)(dest->data),0);
				*((sds)(dest->data)) = 0;
				sdsfree(s);
				continue;
			}
			break;
		}
		OutputStream_write(dest,s,sdslen(s));
		OutputStream_write(dest," ",1);
		sdsfree(s);
	}
end:
	luab_pushsds(L,(sds)(dest->data) );
	OutputStream_detroy(dest);
}

static void parser_parse_args_semicolon(struct tokenizer *t,lua_State* L){
	OutputStream dest = OutputStream_new();
	int c = 0;
	sds s = 0;
	for(;;){
		s = tokenize_get(t,(OutputStream)0);
		if(!s) {
			fprintf(stderr,"unexpected EOF\n");
			abort();
		}
		switch(*s){
		case '(':
			c++;
			break;
		case ')':
			if(!c){
				sdsfree(s);
				goto end;
			}
			c--;
			break;
		case ';':
			if(!c){
				luab_pushsds(L,(sds)(dest->data) );
				sdssetlen((sds)(dest->data),0);
				*((sds)(dest->data)) = 0;
				sdsfree(s);
				continue;
			}
			break;
		}
		OutputStream_write(dest,s,sdslen(s));
		OutputStream_write(dest," ",1);
		sdsfree(s);
	}
end:
	luab_pushsds(L,(sds)(dest->data) );
	OutputStream_detroy(dest);
}

static void parser_parse_body(struct tokenizer *t,lua_State* L){
	OutputStream dest = OutputStream_new();
	int c = 0;
	sds s = 0;
	for(;;){
		s = tokenize_get(t,(OutputStream)0);
		if(!s) {
			fprintf(stderr,"unexpected EOF\n");
			abort();
		}
		switch(*s){
		case '{':
			c++;
			break;
		case '}':
			if(!c){
				sdsfree(s);
				goto end;
			}
			c--;
			break;
		}
		OutputStream_write(dest,s,sdslen(s));
		OutputStream_write(dest," ",1);
		sdsfree(s);
	}
end:
	luab_pushsds(L,(sds)(dest->data) );
	OutputStream_detroy(dest);
}


void parser_parse(struct tokenizer *t,lua_State* L,OutputStream dest){
	sds s;
	int i,N;
	for(;;){
		s = tokenize_get(t,dest);
		
		if(!s) return;
		i = luab_getmacro(L,s);
		if(i==-1){
			OutputStream_write(dest,s,sdslen(s));
			sdsfree(s);
			continue;
		}
		sdsfree(s); s = 0;
		if(i&MT_ARGS){
			s = tokenize_get(t,dest);
			if(!s) {
				fprintf(stderr,"expeceted '(', got EOF\n"/*)*/);
				abort();
			}
			if(*s!='(' /*)*/ ) {
				fprintf(stderr,"expeceted '(', got '%s'\n"/*)*/,s);
				abort();
			}
			sdsfree(s);
			if(i&MT_SEMI) parser_parse_args_semicolon(t,L);
			else          parser_parse_args(t,L);
		}
		if(i&MT_BODY){
			s = tokenize_get(t,dest);
			if(!s) {
				fprintf(stderr,"expeceted '{', got EOF\n"/*}*/);
				abort();
			}
			if(*s!='{' /*}*/ ) {
				fprintf(stderr,"expeceted '{', got '%s'\n"/*}*/,s);
				abort();
			}
			sdsfree(s);
			parser_parse_body(t,L);
			lua_insert(L,2);
		}
		
		luab_call(L);
		lua_settop(L,1);
		s = luab_tosds(L,1);
		OutputStream_write(dest,s,sdslen(s));
		sdsfree(s);
		lua_settop(L,0);
	};
}

