/**
 * @file consumer.cpp
 * @author igor
 * @date 30 окт. 2015 г.
 */

#include <iostream>
#include <exception>
#include <cstdlib>


int
main(int argc, char *argv[])
try
{
	std::cout << "consumer" << std::endl;
	return EXIT_SUCCESS;
}
catch (const std::exception& e)
{
	std::cerr << "Exception: " << e.what() << std::endl;
	return EXIT_FAILURE;
}
catch(...)
{
	std::cerr << "Unknown exceprion" << std::endl;
    return EXIT_FAILURE;
}
