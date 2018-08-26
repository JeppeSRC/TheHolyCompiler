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

#define THC_LIMIT_STRING_LITERAL			0xFFFF
#define THC_LIMIT_RESULTID_BOUND			0x3FFFFF
#define THC_LIMIT_CONTROLFLOW_DEPTH			0x3FF
#define THC_LIMIT_GLOBAL_VARIABLES			0xFFFF
#define THC_LIMIT_LOCAL_VARIABLES			0x7FFFF
#define THC_LIMIT_DECORATIONS_PER_TARGET	0x2F
#define THC_LIMIT_EXECUTIONS_PER_ENTRYPOINT	0xFF
#define THC_LIMIT_INDEXES					0xFF //Indexes for OpAccessChain, OpInBoundsAccessChain, OpPtrAccessChain, OpInBoundsPtrAccessChain, OpCompositeExtract and OpCompositeInsert
#define THC_LIMIT_FUNCTION_PARAMETERS		0xFF
#define THC_LIMIT_OPFUNCTIONCALL_ARGUMENTS	0xFF
#define THC_LIMIT_OPEXTINST_ARGUMENTS		0xFF
#define THC_LIMIT_OPSWITCH_PAIRS			0x3FFF
#define THC_LIMIT_OPTYPESTRUCT_MEMBERS		0x3FFF
#define THC_LIMIT_STRUCT_NESTING_DEPTH		0xFF