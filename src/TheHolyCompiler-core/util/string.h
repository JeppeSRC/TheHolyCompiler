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

	uint64 length;

public:
	String();
	String(const char* const string);
	String(const char* const string, uint64 len);
	String(const String& string);
	explicit String(const String* string);
	String(String&& string);

	~String();

	String& operator=(const String& string);
	String& operator=(String&& string);

	//Appends a string
	String& Append(const String& string);
	String& Append(const char* const string);

	//Removes part of a string
	String& Remove(const String& start, const String& end);
	String& Remove(const char* const start, const char* const end);
	String& Remove(uint64 start, uint64 end);

	//Counts how many strings is in the string
	uint64 Count(const String& string) const;
	uint64 Count(const char* const string) const;

	//Finds the index of the string, if it exist
	uint64 Find(const String& string, uint64 offset = 0) const;
	uint64 Find(const char* const string, uint64 offset = 0) const;
	uint64 Find(const char character, uint64 offset = 0) const;

	//Finds the index of the string, if it exist (Starting from the end)
	uint64 FindReversed(const String& string, uint64 offset = 0) const;
	uint64 FindReversed(const char* const string, uint64 offset = 0) const;
	uint64 FindReversed(const char string, uint64 offset = 0) const;

	//Tests if the string starts with string
	bool StartsWith(const String& string) const;
	bool StartsWith(const char* const string) const;

	//Tests if the string ends with string
	bool EndsWith(const String& string) const;
	bool EndsWith(const char* const string) const;

	//Creates a new String from the indexes.
	//both start and end is inclusive
	String SubString(uint64 start, uint64 end) const;
	String SubString(const String& start, const String& end) const;
	String SubString(const char* const start, const char* const end) const;

	//Inserts string between indexes, start and end is inclusive
	void Insert(uint64 start, uint64 end, const String& string);
	void Insert(uint64 start, uint64 end, const char* const string);

	List<String> Split(const String& delimiters) const;
	List<String> Split(const char* const delimiters) const;

	char& operator[](uint64 index);
	char operator[](uint64 index) const;

	String operator+(const String& string) const;
	String operator+(const char* const string) const;

	bool operator==(const String& string) const;
	bool operator==(const char* const string) const;
};

}
}