#include <core/compiler/preprocessor/preprocessor.h>
#include <core/compiler/compiler.h>
#include <core/compiler/parsing/line.h>
#include <util/log.h>
#include <Windows.h>

using namespace thc;
using namespace core;
using namespace utils;
using namespace parsing;
using namespace preprocessor;
using namespace compiler;


int main() {
	Log::SetOutputHandle(GetStdHandle(STD_OUTPUT_HANDLE));

	char buf[1024];

	GetCurrentDirectoryA(1024, buf);

	String path(buf);

	List<String> includes;
	List<String> defines;

	includes.Add(path);

	Compiler::Run(path + "/test.thsl", defines, includes);

	/*String s = Line::ToString(PreProcessor::Run(path+"/test.thsl", defines, includes));

	printf("%s\n", s.str);*/

	return 0;
}