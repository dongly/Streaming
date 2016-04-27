/*
Streaming.h - Arduino library for supporting the << streaming operator
Copyright (c) 2010-2012 Mikal Hart.  All rights reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef ARDUINO_STREAMING
#define ARDUINO_STREAMING

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#define STREAMING_LIBRARY_VERSION 5

#define cout Serial
#define tabl '\t'

// Generic template
template<class T> 
inline Print &operator <<(Print &stream, T arg) 
{ stream.print(arg); return stream; }

struct _BASED 
{ 
  long val; 
  int base;
  _BASED(long v, int b): val(v), base(b) 
  {}
};

#if ARDUINO >= 100

struct _BYTE_CODE
{
	byte val;
	_BYTE_CODE(byte v) : val(v)
	{}
};
#define _BYTE(a)    _BYTE_CODE(a)

inline Print &operator <<(Print &obj, const _BYTE_CODE &arg)
{ obj.write(arg.val); return obj; } 

#else

#define _BYTE(a)    _BASED(a, BYTE)

#endif

#define _HEX(a)     _BASED(a, HEX)
#define _DEC(a)     _BASED(a, DEC)
#define _OCT(a)     _BASED(a, OCT)
#define _BIN(a)     _BASED(a, BIN)

// Specialization for class _BASED
// Thanks to Arduino forum user Ben Combee who suggested this 
// clever technique to allow for expressions like
//   Serial << _HEX(a);

inline Print &operator <<(Print &obj, const _BASED &arg)
{ obj.print(arg.val, arg.base); return obj; } 

#if ARDUINO >= 18
// Specialization for class _FLOAT
// Thanks to Michael Margolis for suggesting a way
// to accommodate Arduino 0018's floating point precision
// feature like this:
//   Serial << _FLOAT(gps_latitude, 6); // 6 digits of precision

struct _FLOAT
{
  float val;
  int digits;
  _FLOAT(double v, int d): val(v), digits(d)
  {}
};

inline Print &operator <<(Print &obj, const _FLOAT &arg)
{ obj.print(arg.val, arg.digits); return obj; }
#endif

#if ARDUINO >= 18
// Specialization for class _FIXED
// a fixed point number
// output val/(10^d)
//  n=1234
//   Serial << _FIXED(n, 2); // output 12.34

struct _FIXED
{
  long val;
  int digits;
  _FIXED(long v, int d): val(v), digits(d)
  {}
};

Print &operator <<(Print &obj, const _FIXED &arg)
{ 

  if(arg.digits<=0){
    obj.print(arg.val);
  }
  else{
    char buf[12]; 
    char *str = &buf[sizeof(buf) - 1];
    unsigned long n;
    int i = arg.digits; 
    if(arg.val<0){
      n = -arg.val;
    }
    else{
      n = arg.val;
    }
    *str = '\0';

    do {
      unsigned long m = n;
      n /= 10;
      char c = m - 10 * n;
      *--str =  c + '0';
      i--;
      if(i==0){*--str='.';}
    } while(n);
    while(i>=0){
      if(i==0){
        *--str='.';
        *--str='0';
      }
      else{
        *--str='0';     
      }
      i--;
    } 
    if(arg.val<0){*--str='-';}
    obj.print(str);
  }
  return obj; 
} 
#endif

#if ARDUINO >= 18
// Specialization for class _DYNAMIC(v,d,s)
// 输出一个定位小数(10进制),但有长度s限制,输出尽量多的有效位(舍去的小数部分四舍五入)
// output v/(10^d)
// 
//   Serial << _DYNAMIC(12345, 2,4); // output 123.5

struct _DYNAMIC
{
  long val;
  int digits;
  _DYNAMIC(long v, int d,size_t s)
  {
    long b=1;
    val=v;
    if (d<0){
      digits=0;
    }
    else{
      digits=d;
    }
    if(s<=0){
      return;
    }
    if (v<0){
      s--;
    }
    while(s>0){
      b *=  10;
      s--;
    }

    while(digits>0 && abs(val)>=b){
        val =(val+5)/10;
        digits--;
    }
  }
};

Print &operator <<(Print &obj, const _DYNAMIC &arg)
{ 

  if(arg.digits<=0){
    obj.print(arg.val);
  }
  else{
    obj <<_FIXED(arg.val,arg.digits);
  }
  return obj; 
} 
#endif

#if ARDUINO >= 18
// Specialization for class _LEADING0(v,d,s)
// 输出一个定位小数(10进制),显示前度零 ,长度s
// output v/(10^d)
//  n=1234
//   Serial << _DYNAMIC(n, 2,5); // output 012.34

struct _LEADING0
{
  long val;
  int digits;
  size_t size;
  _LEADING0(long v, int d,size_t s):val(v),size(s)
  {if (d<0 || d >=s){
    digits=0;
  }else{
    digits=d;
  }}
};

Print &operator <<(Print &obj, const _LEADING0 &arg)
{ 
  int32_t v,b=1L;
  uint8_t s=0;


  while(s<arg.digits){
    b *=  10;
    s++;
  }
  v=arg.val;
  if(v<0){ //负数
    v=-v;
    s++;
    obj <<"-";
  }
  if(v<b){s++;b *=10;} //纯小数
  while(s<arg.size){
    if(v<b){
      obj <<"0";
    }
    s++;
    b *=10;
  }
  if(arg.digits<=0){
    obj.print(v);
  }
  else{
    obj <<_FIXED(v,arg.digits);
  }
  return obj; 
} 
#endif


// Specialization for enum _EndLineCode
// Thanks to Arduino forum user Paul V. who suggested this
// clever technique to allow for expressions like
//   Serial << "Hello!" << endl;

enum _EndLineCode { endl };

inline Print &operator <<(Print &obj, _EndLineCode arg) 
{ obj.println(); return obj; }

#endif
