#pragma once

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <any>

class List
{
	enum Types { I, D, C, S, L };

	struct Structure
	{
		Types type;
		void* value;
	};

	Structure* data;
	size_t len;
	size_t size;

	void setter() {}

	template <class First, class ... T>
	void setter(First arg, const T&... rest)
	{
		append(arg);
		setter(rest...);
	}

	inline void ensureSize()
	{
		if (len == size)
			data = reinterpret_cast<Structure*>(realloc(data, sizeof(Structure) * (size += 5)));
	}

public:
	List()
	{
		data = reinterpret_cast<Structure*>(malloc(sizeof(Structure) * (size = 5)));
		len = 0;
	}

	List (const List& l)
	{
		len = l.len;
		size = l.size;

		data = reinterpret_cast<Structure*>(malloc(sizeof(Structure) * size));
		
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

	template <class ... T>
	List (const T&... args)
	{
		data = reinterpret_cast<Structure*>(malloc(sizeof(Structure) * (size = 5)));
		len = 0;
		setter(args...);
	}

	void append(std::string x)
	{
		ensureSize();

		data[len].type = S;

		data[len].value = malloc(sizeof(std::string));
		new(data[len++].value) std::string(x);
	}

	void append(List x)
	{
		ensureSize();

		data[len].type = L;

		data[len].value = malloc(sizeof(List));
		new(data[len++].value) List(x);
	}	

	void append(const char* x)
	{
		ensureSize();

		data[len].type = S;

		data[len].value = malloc(sizeof(std::string));
		new(data[len++].value) std::string(x);
	}

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

	friend const size_t len(const List&);

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

const size_t len(const List& l)
{
	return l.len;
}
