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

#include <core/compiler/preprocessor/preprocessor.h>

namespace thc {
namespace core {
namespace preprocessor {

using namespace utils;

PreProcessor::Define::Define(const utils::String& name, const utils::String& value) : name(name), value(value) {}
PreProcessor::Define::Define(const PreProcessor::Define& other) : name(other.name), value(other.value) {}
PreProcessor::Define::Define(const PreProcessor::Define* other) : name(other->name), value(other->value) {}
PreProcessor::Define::Define(PreProcessor::Define&& other) {
	name = std::move(other.name);
	value = std::move(other.value);
}

PreProcessor::Define& PreProcessor::Define::operator=(const PreProcessor::Define& other) {
	if (this != &other) {
		name = other.name;
		value = other.value;
	}

	return *this;
}

PreProcessor::Define& PreProcessor::Define::operator=(PreProcessor::Define&& other) {
	if (this != &other) {
		name = std::move(other.name);
		value = std::move(other.value);
	}

	return *this;
}

}
}
}