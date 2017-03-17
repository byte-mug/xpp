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
#include "tokenizer.h"
#include "patlist.h"
#include "strtest.h"

static prefix_ptr special,tokens;

inline static size_t scan_ws(const char* p,size_t l){
	size_t i;
	for(i=0;i<l;++i){
		switch(p[i]){
		case 1 ... ' ': break;
		default:
			return i;
		}
	}
	return l;
}

inline static size_t scan_lit(const char* p,size_t l,char e){
	size_t i;
	for(i=0;i<l;++i){
		if(p[i]=='\\') {++i; continue; }
		if(p[i]==e) return i+1;
	}
	return 0;
}

inline static size_t scan_word(const char* p,size_t l){
	size_t i;
	for(i=0;i<l;++i){
		switch((unsigned char)(p[i])){
		case '_':
		case '$': /* yes... sigh! */
		case '0' ... '9':
		case 'a' ... 'z':
		case 'A' ... 'Z':
		case 0x80 ... 0xff:
			break;
		default:
			return i;
		}
	}
	return l;
}


void tokenizer_init(){
	char buf[4];
	const char* elems;
	special = prefix_ptr_new();
	tokens  = prefix_ptr_new();
	
	elems = "!#$%&()*+,-./:;<=>|[~]^@{}";
	
	prefix_insert(special,"//");
	prefix_insert(special,"/*");
	prefix_insert(special,"\"");
	prefix_insert(special,"\'");
	
	for(;*elems;++elems){
		*buf=*elems;
		prefix_insert(tokens,smust(sdsnewlen(buf,1)));
	}
	elems = "!%&*+-/<=>|^";
	buf[1]='=';
	for(;*elems;++elems){
		*buf=*elems;
		prefix_insert(tokens,smust(sdsnewlen(buf,2)));
	}
	elems = "<>&|";
	buf[2]='=';
	for(;*elems;++elems){
		buf[1]=*buf=*elems;
		prefix_insert(tokens,smust(sdsnewlen(buf,2)));
		prefix_insert(tokens,smust(sdsnewlen(buf,3)));
	}
	prefix_insert(tokens,"++");
	prefix_insert(tokens,"--");
	prefix_insert(tokens,"->");
	
	//prefix_print(tokens);
}

char linebuffer[1024];

static int tokenizer_refill(struct tokenizer *t,int force){
	char* data;
	if(sdslen(t->buffer)&&!force)return 1;
	if(!(t->source))return 0;
	if(feof(t->source))return 0;
	data = fgets(linebuffer, sizeof linebuffer, t->source);
	if(!data)return 0;
	t->buffer = smust(sdscat(t->buffer,data));
	return 2;
}
#define LINEP printf("\n%d\n",__LINE__);
sds tokenize_get(struct tokenizer *t, OutputStream targ){
	const char* eoc;
	size_t skp;
	int n;
	sds result;
restart:
	skp = scan_ws(t->buffer,sdslen(t->buffer));
	if(targ) OutputStream_write(targ,t->buffer,skp);
	sdsrange(t->buffer,(int)skp,-1);
	n = tokenizer_refill(t,0);
	if(!n) return (sds)0;
	if(n==2) goto restart;
	
	skp = prefix_check(special,t->buffer,sdslen(t->buffer));
	if(skp==2){
		switch(t->buffer[1]){
		case '/': eoc = "\n";break;
		case '*': eoc = "*/";break;
		default: __builtin_unreachable();
		}
		for(;;){
			skp = match_end(t->buffer+2,sdslen(t->buffer)-2,eoc);
			if(skp==0){
				if(!tokenizer_refill(t,1)) {
					sdssetlen(t->buffer,0);
					return (sds)0;
				}
				continue;
			}
			if(targ) OutputStream_write(targ,t->buffer,skp+2);
			sdsrange(t->buffer,(int)skp+2,-1);
			break;
		}
		goto restart;
	}
	if(skp==1){
		for(;;){
			skp = scan_lit(t->buffer+1,sdslen(t->buffer)-1,t->buffer[0]);
			if(skp==0){
				if(!tokenizer_refill(t,1)) {
					sdssetlen(t->buffer,0);
					return (sds)0;
				}
				continue;
			}
			result = smust(sdsnewlen(t->buffer,skp+1));
			sdsrange(t->buffer,(int)skp+1,-1);
			return result;
		}
	}
	skp = prefix_check(tokens,t->buffer,sdslen(t->buffer));
	if(skp){
		result = smust(sdsnewlen(t->buffer,skp));
		sdsrange(t->buffer,(int)skp,-1);
		return result;
	}
	
	skp = scan_word(t->buffer,sdslen(t->buffer));
	if(skp){
		result = smust(sdsnewlen(t->buffer,skp));
		sdsrange(t->buffer,(int)skp,-1);
		return result;
	}
	
	return (sds)0;
}

sds tokenize_copy(struct tokenizer *t, OutputStream targ){
	sds r = tokenize_get(t,targ);
	if(r&&targ) OutputStream_write(targ,r,sdslen(r));
	return r;
}


