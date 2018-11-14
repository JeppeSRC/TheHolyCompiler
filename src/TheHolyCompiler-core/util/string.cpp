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

#include "string.h"
#include "thc_assert.h"
#include "utils.h"

namespace thc {
namespace utils {

String::String() : str(nullptr), length(0) { }

String::String(const char* const string) {
	THC_ASSERT(string != nullptr);
	length = strlen(string);
	str = new char[length+1];

	memcpy(str, string, length+1);
}

String::String(const char* const string, size_t len) : length(len) {
	THC_ASSERT(string != nullptr && len != 0);
	str = new char[length+1];
	str[length] = 0;
	memcpy(str, string, length);
}

String::String(const String& string) : length(string.length) {
	THC_ASSERT(string.str != nullptr);
	str = new char[length+1];
	memcpy(str, string.str, length+1);
}

String::String(const String* string) : length(string->length) {
	THC_ASSERT(string->str != nullptr);
	str = new char[length+1];
	memcpy(str, string->str, length+1);
}

String::String(String&& string)  {
	str = string.str;
	length = string.length;
	string.str = nullptr;
	string.length = 0;
}

String::~String() {
	delete[] str;
}

String& String::operator=(const String& string) {
	if (this != &string) {
		delete[] str;
		str = new char[string.length+1];
		length = string.length;
		memcpy(str, string.str, length+1);
	}

	return *this;
}

String& String::operator=(String&& string) {
	if (this != &string) {
		delete[] str;
		str = string.str;
		length = string.length;

		string.str = nullptr;
		string.length = 0;
	}

	return *this;
}

String& String::Append(const String& string) {
	return Append(string.str);
}

String& String::Append(const char* const string) {
	THC_ASSERT(string != nullptr);
	size_t len = strlen(string);
	char* newStr = new char[length + len + 1];

	memcpy(newStr, str, length);
	memcpy(newStr+length, string, len+1);

	delete[] str;

	str = newStr;
	length = len;

	return *this;
}

String& String::Remove(const String& start, const String& end) {
	return Remove(Find(start), Find(end)+end.length-1);
}

String& String::Remove(const char* const start, const char* const end) {
	return Remove(Find(start), Find(end)+strlen(end)-1);
}

String& String::Remove(size_t start, size_t end) {
	THC_ASSERT(start <= end);
	THC_ASSERT(end < length);

	char* tmp = str;

	size_t remLen = end - start + 1;
	size_t newLen = length - remLen;

	str = new char[newLen+1];

	memcpy(str, tmp, start);
	memcpy(str+start, tmp+end+1, length - (start + remLen) + 1);

	delete[] tmp;
	length = newLen;

	return *this;
}

size_t String::Count(const String& string) const {
	return Count(string.str);
}

size_t String::Count(const char* const string) const {
	THC_ASSERT(string != nullptr);
	
	size_t index = 0;
	size_t count = 0;

	while ((index = Find(string, index+1)) != ~0) {
		count++;
	}
	
	return count;
}

size_t String::Find(const String& string, size_t offset) const {
	return Find(string.str, offset);
}

size_t String::Find(const char* const string, size_t offset) const {
	THC_ASSERT(string != nullptr);
	size_t len = strlen(string);

	for (size_t i = offset; i < length - len; i++) {
		bool match = true;
		for (size_t j = 0; j < len; j++) {
			if (str[i + j] != string[j]) {
				match = false;
				break;
			}
		}

		if (match) {
			return i;
		}
	}
	
	return ~0;
}

size_t String::FindReversed(const String& string, size_t offset) const {
	return FindReversed(string.str, offset);
}

size_t String::FindReversed(const char* const string, size_t offset) const {
	THC_ASSERT(string != nullptr);
	size_t len = strlen(string);

	offset = CLAMP(offset, len-1, length - len);

	if (offset == 0) {
		offset = length-len;
	}

	for (size_t i = offset; i >= 0; i--) {
		bool match = true;
		for (size_t j = 0; j < len; j++) {
			if (str[i + j] != string[j]) {
				match = false;
				break;
			}
		}

		if (match) {
			return i;
		}
	}

	return ~0;
}

bool String::StartsWith(const String& string) const {
	return StartsWith(string.str);
}

bool String::StartsWith(const char* const string) const {
	THC_ASSERT(string != nullptr);
	size_t len = strlen(string);

	if (len > length) return false;

	for (size_t i = 0; i < len; i++) {
		if (str[i] != string[i]) return false;
	}

	return true;
}

bool String::EndsWith(const String& string) const {
	return EndsWith(string.str);
}

bool String::EndsWith(const char* const string) const {
	THC_ASSERT(string != nullptr);
	size_t len = strlen(string);

	if (len > length) return false;

	size_t offset = length - len;

	for (size_t i = 0; i < len; i++) {
		if (str[offset + i] != string[i]) return false;
	}

	return true;
}

String String::SubString(size_t start, size_t end) const {
	THC_ASSERT(start != ~0 && end != ~0);
	THC_ASSERT(end >= start);

	size_t len = end - start + 1;

	THC_ASSERT(start + len < length);

	return String(str+start, len);
}

String String::SubString(const String& start, const String& end) const {
	return SubString(start.str, end.str);
}

String String::SubString(const char* const start, const char* const end) const {
	THC_ASSERT(start != nullptr && end != nullptr);
	return SubString(Find(start), Find(end));
}

List<String> String::Split(const String& delimiters) const {
	return Split(delimiters.str);
}

List<String> String::Split(const char* const delimiters) const {
	List<String> list;

	size_t numDelimiters = strlen(delimiters);

	size_t lastIndex = 0;

	for (size_t i = 0; i < length; i++) {
		for (size_t j = 0; j < numDelimiters; j++) {
			if (str[i] == delimiters[j]) {
				if (lastIndex == i-1 || lastIndex == i) {
					lastIndex++;
				} else {
					list.Add(SubString(lastIndex, i-1));
					lastIndex = i+1;
				}
				break;
			}
		}
	}

	if (lastIndex < length) {
		list.Add(SubString(lastIndex, length-1));
	}

	return list;
}

char& String::operator[](size_t index) {
	THC_ASSERT(index < length);
	return str[index];
}

char String::operator[](size_t index) const {
	THC_ASSERT(index < length);
	return str[index];
}

String String::operator+(const String& string) const {
	return operator+(string.str);
}

String String::operator+(const char* const string) const {
	String tmp(this);

	tmp.Append(string);

	return tmp;
}

bool String::operator==(const String& string) const {
	return operator==(string.str);
}

bool String::operator==(const char* const string) const {
	THC_ASSERT(string != nullptr);
	size_t len = strlen(string);

	if (len != length) return false;

	for (size_t i = 0; i < len; i++) {
		if (str[i] != string[i]) return false;
	}

	return true;
}

}
}