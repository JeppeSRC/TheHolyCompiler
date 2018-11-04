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

#include "log.h"

#define THC_PREALLOC_COUNT 128

namespace thc {
namespace utils {

template<typename T>
class List {
private:
	size_t count;
	T* items;

	size_t allocated;

public:
	List() : count(0), items(nullptr), allocated(0) {}

	List(size_t reserve) : count(0), allocated(reserve) {
		items = new T[reserve];
	}

	List(const List& other) {
		count = other.count;
		allocated = other.allocated;
		items = new T[allocated];
		memcpy(items, other.items, other.GetSize());
	}

	List(const List* other) {
		count = other->count;
		allocated = other->allocated;
		items = new T[allocated];
		memcpy(items, other->items, other->GetAllocatedSize());
	}

	List(List&& other) {
		count = other.count;
		allocated = other.allocated;
		items = other.items;

		other.count = 0;
		other.allocated = 0;
		other.items = nullptr;
	}

	~List() {
		delete[] items;
	}

	inline List& operator=(const List& other) {
		if (this != &other) {
			delete[] items;
			count = other.count;
			allocated = other.allocated;
			items = new T[allocated];
			
			for (size_t i = 0; i < count; i++) {
				new (&items[i]) T(std::move(other.items[i]));
			}
		}

		return *this;
	}

	inline List& operator=(List&& other) {
		if (this != &other) {
			delete[] items;
			count = other.count;
			allocated = other.allocated;
			items = other.items;

			other.count = 0;
			other.allocated = 0;
			other.items = nullptr;
		}

		return *this;
	}

	/*Resizes the list*/
	inline void Resize(size_t count) {
		if (count > allocated) {
			Reserve(allocated + THC_PREALLOC_COUNT);
		}

		this->count = count;
	}

	/*Resizes the list and fills the new items with the specifed defaults. If the new sizes is large, all new items will have the default values. If the new size is smaller, all items will have the defaults*/
	template<typename ...Args>
	inline void Resize(size_t count, Args&&... defaults) {
		if (count > allocated) {
			Reserve(allocated + THC_PREALLOC_COUNT);
		}

		if (count > this->count) {
			for (size_t i = this->count; i < count; i++) {
				new (items+i) T(defaults...);
			}
		} else {
			for (size_t i = 0; i < count; i++) {
				new (items+i) T(defaults...);
			}
		}

		this->count = count;
	}

	/*Reserves space*/
	inline void Reserve(size_t reserve) {
		if (reserve <= allocated) return;

		T* tmp = items;

		items = new T[reserve];

		for (size_t i = 0; i < count; i++) {
			new (items+i) T(std::move(tmp[i]));
		}

		delete[] tmp;
	}

	/*Addes item at the end of the list*/
	inline void Add(const T& item) {
		if (count >= allocated) {
			Reserve(allocated + THC_PREALLOC_COUNT);
		}

		new (items+count++) T(item);
	}

	/*Addes item at the end of the list*/
	inline void Add(T&& item) {
		if (count >= allocated) {
			Reserve(allocated + THC_PREALLOC_COUNT);
		}

		new (items+count++) T(std::move(item));
	}

	/*Replaces item*/
	inline void ReplaceAt(size_t index, const T& item) {
		THC_ASSERT(index < count);

		elements[index] = item;
	}

	/*Replaces item*/
	inline void ReplaceAt(size_t index, T&& item) {
		THC_ASSERT(index < count);

		elements[index] = std::move(item);
	}

	/*Constructs an item at the end of the list*/
	template <typename ...Args>
	inline void Emplace(Args&&... args) {
		if (count >= allocated) {
			Reserve(allocated + THC_PREALLOC_COUNT);
		}

		new (items+count++) T(args...);
	}

	/*Constructs a new item in the specified location*/
	template <typename ...Args>
	inline void EmplaceAt(size_t index, Args&&... args) {
		THC_ASSERT(index < count);

		new (&items[index]) T(args...);
	}

	/*Inserts item*/
	template<typename ...Args>
	inline void EmplaceInsert(size_t index, Args&&... args) {
		THC_ASSERT(index < count);

		if (count+1 > allocated) {
			Reserve(allocated + THC_PREALLOC_COUNT);
		}

		size_t itemsToMove = count++ - index;
		
		for (size_t i = 0; i < itemsToMove; i++) {
			size_t location = count - i - 1;
			new (items+location) T(std::move(items[itemsToMove+index - i - 1]));
		}

		new (items+index) T(args...);
	}

	/*Inserts item*/
	inline void Insert(size_t index, const T& item) {
		THC_ASSERT(index < count);

		if (count+1 > allocated) {
			Reserve(allocated + THC_PREALLOC_COUNT);
		}

		size_t itemsToMove = count++ - index;


		for (size_t i = 0; i < itemsToMove; i++) {
			size_t location = count - i - 1;
			new (items+location) T(std::move(items[itemsToMove+index - i - 1]));
		}

		new (items+index) T(item);
	}

	/*Inserts item*/
	inline void Insert(size_t index, T&& item) {
		THC_ASSERT(index < count);

		if (count+1 > allocated) {
			Reserve(allocated + THC_PREALLOC_COUNT);
		}

		size_t itemsToMove = count++ - index;


		for (size_t i = 0; i < itemsToMove; i++) {
			size_t location = count - i - 1;
			new (items+location) T(std::move(items[itemsToMove+index - i - 1]));
		}

		new (items+index) T(std::move(item));
	}

	/*Inserts items from another list*/
	inline void InsertList(size_t index, const List& other) {
		THC_ASSERT(index <= count);

		size_t totalCount = count + other.count;

		if (totalCount > allocated) {
			Reserve(totalCount + THC_PREALLOC_COUNT);
		}

		size_t itemsToMove = count - index;

		for (size_t i = 0; i < itemsToMove; i++) {
			size_t location = totalCount - i - 1;
			new (items+location) T(std::move(items[i+index]));
		}

		for (size_t i = 0; i < other.count; i++) {
			new (items+index+i) T(other[i]);
		}

		count = totalCount;
	}

	/*Inserts items from another list*/
	inline void InsertList(size_t index, List&& other) {
		THC_ASSERT(index <= count);

		size_t totalCount = count + other.count;

		if (totalCount > allocated) {
			Reserve(totalCount + THC_PREALLOC_COUNT);
		}

		size_t itemsToMove = count - index;

		for (size_t i = 0; i < itemsToMove; i++) {
			size_t location = totalCount - i - 1;
			new (items+location) T(std::move(items[i+index]));
		}

		for (size_t i = 0; i < other.count; i++) {
			new (items+index+i) T(std::move(other.items[i]));
		}

		count = totalCount;

		other.count = 0;
		other.allocated = 0;
	}

	/*Removes item at the specified location*/
	inline T RemoveAt(size_t index) {
		THC_ASSERT(index < count);

		T tmp(std::move(items[index]));

		memcpy(items+index, items+index+1, (--count - index) * sizeof(T));
			
		memset(items+count, 0, sizeof(T));

		return tmp;
	}

	/*Removes the specifed item*/
	inline T Remove(const T& item) {
		return RemoveAt(Find(item));
	}

	/*Finds the item*/
	inline size_t Find(const T& item) const {
		for (size_t i = 0; i < count; i++) {
			if (items[i] == count) return i;
		}

		return ~0;
	}

	inline T& operator[](size_t index) {
		THC_ASSERT(index < count);
		return items[index];
	}

	inline const T& operator[](size_t index) const {
		THC_ASSERT(index < count);
		return items[index];
	}

	inline T* GetData() { return items; }
	inline const T* GetData() const { return items; }

	inline size_t GetCount() const { return count; }
	inline size_t GetSize() const { return count * sizeof(T); }

	inline size_t GetAllocatedCount() const { return allocated; }
	inline size_t GetAllocatedSize() const { return allocated * sizeof(T); }
};

}
}