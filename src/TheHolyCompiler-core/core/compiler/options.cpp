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

#include "compiler.h"
#include <util/log.h>

namespace thc {
namespace core {
namespace compiler {

using namespace utils;

bool CompilerOptions::warningsMessages = true;
bool CompilerOptions::debugMessages = false;
bool CompilerOptions::debugInformation = false;
bool CompilerOptions::ppOnly = false;
bool CompilerOptions::stopOnError = false;
bool CompilerOptions::fpDefaultPrecision64 = false;
bool CompilerOptions::implicitConversions = true;
bool CompilerOptions::vertexShader = false;
bool CompilerOptions::fragmentShader = false;

List<String> CompilerOptions::includeDirectories;
List<String> CompilerOptions::defines;
String CompilerOptions::inputFile;
String CompilerOptions::outputFile;

bool CompilerOptions::ParseOptions(uint32 argc, char** argv) {
	List<String> args;

	for (uint32 i = 0; i < argc; i++) {
		args.Add(String(argv[i]));
	}

	return ParseOptions(args);
}

bool CompilerOptions::ParseOptions(const List<String>& args) {
	for (uint64 i = 1; i < args.GetCount(); i++) {
		String arg = args[i];

		if (arg == "-noW") warningsMessages = false;
		else if (arg == "-soE") stopOnError = true;
		else if (arg == "-eD") debugMessages = true;
		else if (arg == "-eDI") debugInformation = true;
		else if (arg == "-pp") ppOnly = true;
		else if (arg == "-deffp64") fpDefaultPrecision64 = true;
		else if (arg == "-moIMP") implicitConversions = false;
		else if (arg == "-vertex") vertexShader = true;
		else if (arg == "-fragment") fragmentShader = true;
		else if (arg.StartsWith("-D=")) {
			arg.Remove(0, 2);
			defines.Add(arg.Split(","));
		} else if (arg.StartsWith("-I=")) {
			arg.Remove(0, 2);
			includeDirectories.Add(arg.Split(","));
		} else if (arg.StartsWith("-out=")) {
			arg.Remove(0, 4);
			
			if (outputFile.length != 0) {
				Log::Error("Output file already specified");
				return false;
			}

			outputFile = arg;
		} else {
			if (inputFile.length != 0) {
				Log::Error("Input file already specified");
				return false;
			}

			inputFile = arg;
		}
	}

	if (vertexShader == fragmentShader) {
		Log::Error("Only one of -fragment or -vertex must be specified");
		return false;
	}

	return true;
}

}
}
}