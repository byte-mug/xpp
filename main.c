/*
 * Copyright (c) 2017 Simon Schmidt
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
#include <stdio.h>
#include <string.h>
#include "patlist.h"
#include "strtest.h"
#include "tokenizer.h"
#include "luab.h"
#include "parser.h"
#include "luasrc/lauxlib.h"

int main(int argc,const char** argv){
	struct tokenizer tok;
	tok.buffer = smust(sdsempty());
	tok.source = ptrmust(fopen("test.txt","r"));
	lua_State* L = create_lua();

	luab_setmacro(L,"tail_for","local pre,con,pos,body = ...; return solve(pre..';\\ndo {\\n'..body..'\\n'..pos..';\\n} while('..con..');')",MT_ARGS|MT_BODY);
	
	OutputStream dest = OutputStream_new();
	//OutputStream dest = FILE_asStream(stdout);
	
	tokenizer_init();
	
	
	parser_parse(&tok,L,dest);
	
	printf("%s\n",(char*)dest->data);
	return 0;
}



