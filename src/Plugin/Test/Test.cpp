#include "Test.h"

Test::Test()
{
}

Test::~Test()
{
}

bool Test::Init()
{
	printf("Test plugin load success");
	return false;
}