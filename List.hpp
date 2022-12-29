#pragma once

// <cstdlib> for malloc
// <cstring> to handle c-style strings (std::type_info::name())
// <any> to use std::any as the return type for accessing elements using indices
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <any>

class List
{
	// I = int, D = double, C = char, S = string, L = List
	enum Types { I, D, C, S, L };

	// The structure for each element of the list
	struct Structure
	{
		Types type;
		void* value;
	};

	Structure* data;	// The array of Structure items used to store each element of the list
	size_t len;		// Stores length of the list
	size_t size;		// To keep track of memory required by data as it's dynamically resized. (sizeof(data) = sizeof(Structure) * size)

	// Setter function definitions to handle recursion required for variadic template parsing
	// Function exists to handle multiple paramaters from the parameterised constructor for generating the list
	void setter() {}

	template <class First, class ... T>
	void setter(First arg, const T&... rest)
	{
		append(arg);
		setter(rest...);
	}

	// Memory allocater for data (incremented by 5)
	inline void ensureSize()
	{
		if (len == size)
			data = reinterpret_cast<Structure*>(realloc(data, sizeof(Structure) * (size += 5)));
	}

public:
	// Non-parameterised constructor
	List()
	{
		data = reinterpret_cast<Structure*>(malloc(sizeof(Structure) * (size = 5)));
		len = 0;
	}

	// Copy constructor
	List (const List& l)
	{
		len = l.len;
		size = l.size;

		data = reinterpret_cast<Structure*>(malloc(sizeof(Structure) * size));
		
		// Loop to copy every element properly into new list
		// For primitive types (C types), std::memcpy is used
		// For std::string and List, placement new is used to avoid copying pointers to dynamically allocated memory
		for (size_t i = 0; i < len; i++)
		{
			data[i].type = l.data[i].type;
			switch (data[i].type)
			{
				case I:
					data[i].value = malloc(sizeof(int));
					std::memcpy(data[i].value, l.data[i].value, sizeof(int));
					break;
				case D:
					data[i].value = malloc(sizeof(double));
					std::memcpy(data[i].value, l.data[i].value, sizeof(double));
					break;
				case C:
					data[i].value = malloc(sizeof(char));
					std::memcpy(data[i].value, l.data[i].value, sizeof(char));
					break;
				case S:
					data[i].value = malloc(sizeof(std::string));
					new(data[i].value) std::string(*reinterpret_cast<std::string*>(l.data[i].value));
					break;
				case L:
					data[i].value = malloc(sizeof(List));
					new(data[i].value) List(*reinterpret_cast<List*>(l.data[i].value));
			}
		}
	}

	// Parameterised constructor
	// Designed to accept infinite list of arguments
	// Passes control to setter()
	template <class ... T>
	List (const T&... args)
	{
		data = reinterpret_cast<Structure*>(malloc(sizeof(Structure) * (size = 5)));
		len = 0;
		setter(args...);
	}

	// append() overload for std::string arguments
	// Separate overload since placement new is required for dynamic object creation
	void append(std::string x)
	{
		ensureSize();

		data[len].type = S;

		data[len].value = malloc(sizeof(std::string));
		new(data[len++].value) std::string(x);
	}

	// append() overload for List() arguments
	// Again separate overload since placement new is used
	void append(List x)
	{
		ensureSize();

		data[len].type = L;

		data[len].value = malloc(sizeof(List));
		new(data[len++].value) List(x);
	}	

	// append() overload to handle strings when generated using double-quotes ("")
	// Stores as std::string
	void append(const char* x)
	{
		ensureSize();

		data[len].type = S;

		data[len].value = malloc(sizeof(std::string));
		new(data[len++].value) std::string(x);
	}

	// General append() overload to handle primitive types
	// Dereferencing is used for storing
	template <class T>
	void append(T x)
	{
		ensureSize();

		if (!strncmp(typeid(T).name(), "i", 2))
			data[len].type = I;
		else if (!strncmp(typeid(T).name(), "d", 2))
			data[len].type = D;
		else if (!strncmp(typeid(T).name(), "c", 2))
			data[len].type = C;
		else
			return;

		data[len].value = malloc(sizeof(T));
		*reinterpret_cast<T*>(data[len++].value) = x;
	}

	// operator[] overload for accessing elements (doesn't allow setting)
	// Returns std::any object to comply with C++ style static-typing
	// Designed to return arbitrary std::any object in case type can't be handled
	const std::any operator[] (const int index) const
	{
		int i = index;

		if (i < 0)
			i += len;

		if (i >= 0 && i < len)
			switch (data[i].type)
			{
				case I:
					return *reinterpret_cast<int*>(data[i].value);
				case D:
					return *reinterpret_cast<double*>(data[i].value);
				case C:
					return *reinterpret_cast<char*>(data[i].value);
				case S:
					return *reinterpret_cast<std::string*>(data[i].value);
				case L:
					return *reinterpret_cast<List*>(data[i].value);
			}

		std::any a;
		
		return a;
	}

	// Stream insertion operator overload
	// Prints out the list python style
	friend std::ostream& operator<< (std::ostream& buff, List& l)
	{
		buff << '[';

		for (size_t i = 0; i < l.len; i++)
		{
			switch (l.data[i].type)
			{
				case I:
					buff << *reinterpret_cast<int*>(l.data[i].value); break;
				case D:
					buff << *reinterpret_cast<double*>(l.data[i].value); break;
				case C:
					buff << '\'' << *reinterpret_cast<char*>(l.data[i].value) << '\''; break;
				case S:
					buff << '"' << *reinterpret_cast<std::string*>(l.data[i].value) << '"'; break;
				case L:
					buff << *reinterpret_cast<List*>(l.data[i].value);
			}

			if (i < l.len - 1)
				buff << ", ";
		}

		buff << ']';

		return buff;
	}

	// Declaring len function as friend to use List::len
	friend const size_t len(const List&);

	// Destructor
	// Uses delete for std::string and List elements
	// Uses free() for everything else
	~List()
	{
		for (size_t i = 0; i < len; i++)
			switch (data[i].type)
			{
				case S:
					delete reinterpret_cast<std::string*>(data[i].value); break;
				case L:
					delete reinterpret_cast<List*>(data[i].value); break;
				default:
					free(data[i].value);
			}
		free(data);
	}
};

// Function to return length of the accepted list
const size_t len(const List& l)
{
	return l.len;
}
