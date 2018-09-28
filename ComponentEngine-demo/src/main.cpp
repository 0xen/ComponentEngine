#include <ComponentEngine\Engine.hpp>
#include <iostream>
#include <vector>


ComponentEngine::Engine* engine;


int main(int argc, char **argv)
{
	engine = new ComponentEngine::Engine();

	bool running = true;
	while (running)
	{

	}

	delete engine;
    return 0;
}
