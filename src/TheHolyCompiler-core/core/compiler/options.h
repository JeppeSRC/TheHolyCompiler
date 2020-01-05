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
#include <util/string.h>

namespace thc {
namespace core {
namespace compiler {

class CompilerOptions {
private:
	static bool warningsMessages;
	static bool debugMessages;
	static bool debugInformation;
	static bool ppOnly;
	static bool stopOnError;
	static bool fpPrecision64;
	static bool implicitConversions;
	static bool vertexShader;
	static bool fragmentShader;

	static utils::List<utils::String> includeDirectories;
	static utils::List<utils::String> defines;
	static utils::String inputFile;
	static utils::String outputFile;

public:
	static bool ParseOptions(uint32 argc, char** argv);
	static bool ParseOptions(const utils::List<utils::String>& args);

	inline static bool WarningMessages() { return warningsMessages; }
	inline static bool DebugMessages() { return debugMessages; }
	inline static bool DebugInformation() { return debugInformation; }
	inline static bool PPOnly() { return ppOnly; }
	inline static bool StopOnError() { return stopOnError; }
	inline static bool FPPrecision64() { return fpPrecision64; }
	inline static bool FPPrecision32() { return !fpPrecision64; }
	inline static bool ImplicitConversions() { return implicitConversions; }
	inline static bool VertexShader() { return vertexShader; }
	inline static bool FragmentShader() { return fragmentShader; }

	inline static const utils::List<utils::String>& IncludeDirectories() { return includeDirectories; }
	inline static const utils::List<utils::String>& PredefinedDefines() { return defines; }
	inline static const utils::String& InputFile() { return inputFile; }
	inline static const utils::String& OutputFile() { return outputFile; }
};

}
}
}