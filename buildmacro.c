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
#include <stdarg.h>
#include "buildmacro.h"

sds bdmcr_build_macro(sds* args,int n,sds body){
	int i;
	sds dest = smust(sdsnew("local "));
	for(i=0;i<n;++i){
		if(i)dest = smust(sdscat(dest,","));
		dest = smust(sdscatsds(dest,args[i]));
	}
	dest = smust(sdscat(dest," = ... ;\n"));
	for(i=0;i<n;++i){
		dest = smust(sdscatsds(dest,args[i]));
		dest = smust(sdscat(dest," = normalize("));
		dest = smust(sdscatsds(dest,args[i]));
		dest = smust(sdscat(dest,");\n"));
	}
	dest = smust(sdscat(dest,"return solve("));
	dest = smust(sdscatsds(dest,body));
	dest = smust(sdscat(dest,");\n"));
	return dest;
}

sds bdmcr_build_macro_stub(const char* body, ...){
	const char* sp;
	static sds margs[1024];
	sds sbody,sresult;
	va_list ap;
	int i=0,l = 1024;
	
	sbody = smust(sdsnew(body));
	
	va_start(ap, body);
	for(i=0;i<1024;++i){
		sp = va_arg(ap, const char*);
		if(!sp){ l = i; break; }
		margs[i] = smust(sdsnew(sp));
	}
	va_end(ap);
	
	sresult = bdmcr_build_macro(margs,l,sbody);
	sdsclear(sbody);
	for(i=0;i<l;++i){
		sdsclear(margs[i]);
	}
	return sresult;
}

