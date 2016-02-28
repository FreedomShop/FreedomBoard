/*
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 */

/*  * * * * * * * * * * * * * * * * * * * * * * * * * * *
 Code by Wellington Rats
 http://www.wtisoftware.com.br
* * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// For Arduino 1.0 and earlier
#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "StringParser.h"

StringParser::StringParser(unsigned int cap) : max_word_length(0)
{
  capacity = cap;
  working.reserve(capacity);
}

StringParser::StringParser(unsigned int cap, const char* init) : max_word_length(0)
{
  capacity = cap;
  working.reserve(capacity);
  while(char c=*init++) addChar(c);
}

void StringParser::addChar(char c)

{
 
  if(working.length() < capacity)
    working += c;
  newline = (c == '\n');
}

void StringParser::addChars(char* p)
{
  char c;
  while(c=*p++) addChar(c);
}

const char* StringParser::c_str() {
  return working.c_str();
}
String& StringParser::get() {
  return working;
}

void StringParser::clear() {
  working="";
  newline=false;
}

String StringParser::nextWord() {

  return nextWord(' ', working);
}

String StringParser::nextWord(char sep) {

  return nextWord(sep, working);
}
/*
String StringParser::nextWord(char sep) {
 
  working.trim();
  int pos = working.indexOf(sep);
  if(pos == -1) pos = working.length();
  String out = working.substring(0, pos);
  working = working.substring(pos+1);

  return out;

}
*/

String StringParser::nextWord(char sep, String &str) {

  char delims[] = "\"'";
  str.trim();
  int len = str.length();
  int pos = 0;
  int subs= 0;
  bool hasDelims;

  while(pos<len) {
    char c = str.charAt(pos);
    if(c==sep) break;
    hasDelims = false;
    for(byte i=0;i<2;i++) {

      if(c==delims[i]) {
         hasDelims = true;
         while(++pos<len && str.charAt(pos) != delims[i]);
         pos++; subs++;
         break;
      }

    } 

    if(!hasDelims) pos++;

  }

  String out = str.substring(0, pos);
  str = str.substring(pos+1);

  out.trim();
  if(subs==1) {
    len=out.length();
    for(byte i=0;i<2;i++)
      if(out.charAt(0) == delims[i] && out.charAt(len-1) == delims[i]) {
        out = out.substring(1,len-1);
        break;
      }
  }
  if((max_word_length) && (out.length() > max_word_length)) {
    out = out.substring(0, max_word_length);
  }
  return out;
}
/*
String StringParser::nextWord(char sep) {
 
  static char c='"';
  int len=-1;
  int asp=-1;
  working.trim();
  int len = working.length();
  while(1) {
    pos = working.indexOf(sep, asp+1);
    asp = working.indexOf(c,   asp+1);
    if(pos == -1) pos = len;
    if(asp == -1) asp = len;

    if(asp < pos) {
      asp = working.indexOf(c, asp+1);
      if(asp != -1) {
        pos=working.indexOf(sep,asp+1);
        if(pos==-1) { 
           pos=len;
           break;
        }  
    } else break;
  }

  String out = working.substring(0, pos);
  working = working.substring(pos+1);

  if(out.length() && out.charAt(0)==c) {
    out = out.substring(1,out.length()-1);
  }
  out.trim();

  return out;

}
*/

void StringParser::setString(String str) {
   if(str.length() > capacity)
     working=str.substring(0,capacity);
   else
     working=str;
}


