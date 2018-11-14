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

#include "preprocessor.h"
#include <util/log.h>
#include <stdio.h>

namespace thc {
namespace core {
namespace preprocessor {

using namespace utils;

String PreProcessor::FindFile(const String& fileName) {
	for (uint64 i = 0; i < includeDirectories.GetCount(); i++) {
		String path = includeDirectories[i];
		
		if (path.EndsWith("/")) {
			path.Append(fileName);
		} else {
			path.Append("/").Append(fileName);
		}

		if (includedFiles.Find(path) != ~0) {
			return "AlreadyIncluded";
		}

		FILE* f = fopen(path.str, "rb");

		if (!f) {
			continue;
		}

		fclose(f);

		return path;
	}

	return "NotFound";
}

bool PreProcessor::IsDefined(const String& name) {
	for (uint64 i = 0; i < defines.GetCount(); i++) {
		if (defines[i].name == name) return true;
	}

	return false;
}

}
}
}