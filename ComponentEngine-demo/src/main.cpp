#include <ComponentEngine\Engine.hpp>
#include <ComponentEngine\Components\Mesh.hpp>
#include <ComponentEngine\Components\Renderer.hpp>
#include <EnteeZ\EnteeZ.hpp>
#include <iostream>
using namespace ComponentEngine;
using namespace enteez;
using namespace Renderer;
Engine* engine;
void LogicThread()
{
	engine->GetRendererMutex().lock();
	EntityManager& em = engine->GetEntityManager();
	IRenderer* renderer = engine->GetRenderer();
	// Load the scene
	engine->LoadScene("../../ComponentEngine-demo/Scenes/GameInstance.xml");
	Transformation* camera = engine->GetCameraTransformation();
	camera->Translate(glm::vec3(0.0f, 0.0f, 6.0f));
	engine->GetRendererMutex().unlock();
	// Logic Updating
	while (engine->Running())
	{
		float thread_time = engine->GetThreadTime();
		em.ForEach<Transformation,Mesh>([thread_time, camera](enteez::Entity* entity, Transformation& transformation, Mesh& mesh)
		{
			if (&transformation != camera)
			{
				transformation.Rotate(glm::vec3(0.0f, 1.0f, 0.0f), 1.0f * thread_time);
			}
		}, true);
		engine->GetRendererMutex().lock();
		//std::cout << "L:" << thread_time << std::endl;
		engine->UpdateScene();
		engine->GetRendererMutex().unlock();
	}
}

#include <direct.h>
#define GetCurrentDir _getcwd

std::string GetCurrentWorkingDir(void)
{
	char buff[FILENAME_MAX];
	GetCurrentDir(buff, FILENAME_MAX);
	std::string current_working_dir(buff);
	return current_working_dir;
}


struct A
{
public:
	virtual void test1() = 0;
	int a;
};

struct B
{
public:
	virtual void test2() = 0;
};


struct C
{
public:
	virtual void test3() = 0;
};

struct D : public A, public B, public C
{
public:
	virtual void test1() { std::cout << "a" << std::endl; };
	virtual void test2() { std::cout << "b" << std::endl; };
	virtual void test3() { std::cout << "c" << std::endl; };
};


int main(int argc, char **argv)
{

	void* a = new D;

	B& d = *static_cast<D*>(a);


	d.test2();
	//d.test2();
	//d.test3();



	std::cout << GetCurrentWorkingDir() << std::endl;



	engine = Engine::Singlton();
	engine->Start(LogicThread);
	EntityManager& em = engine->GetEntityManager();



	/*
	engine->GetRendererMutex().lock();
	Mesh* mesh = new Mesh(em.CreateEntity(), "../../ComponentEngine-demo/Resources/Models/cessna.obj");
	void* ptr = mesh; // Storage in ValuePair
	UI* ui = dynamic_cast<UI*>(static_cast<Mesh*>(ptr)); // Conversion to UI from ValuePair
	UI& ui1 = *ui; // Returned to UI Manager from ValuePair
	ui1.Display(); // Called in UI Manager
	engine->GetRendererMutex().unlock();
	*/

	// Rendering
	while (engine->Running())
	{
		engine->Update();
		engine->GetRendererMutex().lock();



		engine->UpdateUI();
		//std::cout << "R:" << engine->GetFrameTime() << std::endl;
		engine->RenderFrame();
		engine->GetRendererMutex().unlock();
	}
	delete engine;
    return 0;
}
