/*
MIT License

Copyright (c) 2018 Jesper Hammarström

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#pragma once

#include <memory>
#include "list.h"

namespace thc {
namespace utils {

class String;

class String {
public:
	char* str;

	size_t length;

public:
	String();
	String(const char* const string);
	String(const char* const string, size_t len);
	String(const String& string);
	explicit String(const String* string);
	String(String&& string);

	~String();

	String& operator=(const String& string);
	String& operator=(String&& string);

	//Appends a string
	String& Append(const String& string);
	String& Append(const char* const string);

	//Finds the index of the string, if it exist
	size_t Find(const String& string) const;
	size_t Find(const char* const string) const;

	//Creates a new String from the indexes.
	//both start and end is inclusive
	String SubString(size_t start, size_t end) const;
	String SubString(const String& start, const String& end) const;
	String SubString(const char* const start, const char* const end) const;

	List<String> Split(const String& delimiters) const;
	List<String> Split(const char* const delimiters) const;

	char& operator[](size_t index);
	char operator[](size_t index) const;
};

}
}