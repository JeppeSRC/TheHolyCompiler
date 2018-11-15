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
#include <util/utils.h>
#include <stdio.h>

namespace thc {
namespace core {
namespace preprocessor {

using namespace utils;

String PreProcessor::FindFile(const String& fileName, String parentDir) {
	parentDir.Append(fileName);

	if (includedFiles.Find(parentDir) != ~0) {
		return "AlreadyIncluded";
	}

	FILE* f = fopen(parentDir.str, "rb");

	if (f != nullptr) {
		fclose(f);
		return parentDir;
	}

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

		f = fopen(path.str, "rb");

		if (f != nullptr) {
			continue;
		}

		fclose(f);

		includedFiles.Add(path);

		return path;
	}

	return "NotFound";
}

bool PreProcessor::IsDefined(const String& name, const String& value) {
	uint64 index = defines.Find<String>(name, [](const Define& current, const String& name) -> bool {
		if (current.name == name) return true;
		return false;
	});

	if (index != ~0) {
		defines[index].value = value;
		return true;
	}

	return false;
}

}
}
}