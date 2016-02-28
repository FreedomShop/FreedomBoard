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

#ifndef StringParser_h
#define StringParser_h

#include <inttypes.h>
#include <WString.h> 

class StringParser
{

public:
  StringParser(unsigned int cap);
  StringParser(unsigned int cap, const char* init);
  void  clear();
  void addChar(char c);
  void addChars(char *p);
  const char* c_str();
  String& get();
  String nextWord();
  String nextWord(char sep);
  String nextWord(char sep, String& str);
  inline bool isNewLine() { return newline; }
  inline int length() { return working.length(); }
  void setString(String str);
  void setMaxWordLength(unsigned int max) { max_word_length = max; };


protected:
  String working;
  unsigned int capacity;
  unsigned int max_word_length;
  bool newline;

};

#endif
