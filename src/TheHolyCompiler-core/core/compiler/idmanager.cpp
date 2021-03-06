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

namespace thc {
namespace core {
namespace compiler {

using namespace utils;

List<ID*> IDManager::free;
List<ID*> IDManager::ids;

ID* IDManager::GetNewId() {
	ID* newId = nullptr;

	if (free.GetCount() != 0) {
		newId = free.RemoveAt(0);
	} else {
		newId = new ID(GetCount()+1);
		ids.Add(newId);
	}

	return newId;
}

void IDManager::RemoveId(ID* id) {
	free.Add(id);
	ids.Remove(id);
}

uint32 IDManager::GetCount() {
	return (uint32)ids.GetCount();
}

}
}
}