#include <lfilesystem/lfilesystem.h>
#include <iostream>

int main (int, char**)
{
	const auto cwd = limes::files::dirs::cwd();

	std::cout << cwd.getAbsolutePath();

	return 0;
}
