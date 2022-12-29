#include <iostream>
#include "List.hpp"

using namespace std;

int main()
{
	List a = {45, "hello world", "hi"}, b = {-32.3, 4};

	a.append(b);

	cout << a << endl;

	return 0;
}