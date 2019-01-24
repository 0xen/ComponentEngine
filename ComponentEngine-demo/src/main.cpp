#include <ComponentEngine\Engine.hpp>
#include <ComponentEngine\Components\Mesh.hpp>
#include <ComponentEngine\Components\Renderer.hpp>
#include <EnteeZ\EnteeZ.hpp>
#include <ItemHover.hpp>
#include <iostream>

using namespace ComponentEngine;
using namespace enteez;
using namespace Renderer;
Engine* engine;

// Updates per second
const int kUPS = 30;
// UI updates per second
const int kUIUPS = 15;

// Camera
Transformation* camera;

class UIThread : public ThreadHandler
{
public:
	UIThread() : ThreadHandler(kUIUPS) {}
	virtual void Initilize()
	{

	}
	virtual void Loop()
	{
		engine->UpdateUI();
	}
	virtual void Cleanup()
	{
		std::cout << "UI Shutdown" << std::endl;
	}
};

class LogicThread : public ThreadHandler
{
public:
	LogicThread() : ThreadHandler(kUPS) {}
	virtual void Initilize()
	{
		engine->GetRendererMutex().lock();
		// Load the scene
		engine->LoadScene("../../ComponentEngine-demo/Scenes/GameInstance.xml");
		camera = engine->GetCameraTransformation();
		camera->Translate(glm::vec3(0.0f, 2.0f, 10.0f));
		engine->GetRendererMutex().unlock();
		engine->UpdateScene();
	}

	virtual void Loop()
	{
		float thread_time = engine->GetLastThreadTime();
		EntityManager& em = engine->GetEntityManager();

		for (auto e : em.GetEntitys())
		{
			e->ForEach<Logic>([&](enteez::Entity* entity, Logic& logic)
			{
				logic.Update(thread_time);
			});
		}

		engine->UpdateScene();
	}

	virtual void Cleanup()
	{
		std::cout << "Logic Shutdown" << std::endl;
	}
};


void RegisterCustomComponents()
{
	engine->RegisterComponentBase("ItemHover", ItemHover::EntityHookDefault, ItemHover::EntityHookXML);
	engine->RegisterBase<ItemHover, Logic, UI>();
}

/*
class DBConnection
{
	static DBConnection** m_instance;
	static int count;
	DBConnection()
	{
	}
public:
	static DBConnection * instance()
	{
		if (instance == nullptr)
		{
			m_instance = new DBConnection*[10];
		}
		if (count >= 10) return nullptr;
		DBConnection*& ins = m_instance[count++];
		return (ins = new DBConnection());
	}
};
DBConnection** DBConnection::m_instance = nullptr;
int DBConnection::count = 0;
*/

/*
class DBConnection
{
protected:
	DBConnection()
	{

	}
public:
	DBConnection * CreateInstance()
	{
		return new DBConnection();
	}
};

class MySQL : public DBConnection
{
protected:
	MySQL()
	{

	}
public:
	DBConnection * CreateInstance()
	{
		return new MySQL();
	}
};*/


int main(int argc, char **argv)
{
	engine = Engine::Singlton();
	engine->Start();

	RegisterCustomComponents();

	// Render a frame so you know it has not crashed xD
	engine->RenderFrame();

	engine->AddThread(new LogicThread(), "Logic");
	engine->AddThread(new UIThread(), "UI");

	// Rendering
	while (engine->Running(60))
	{
		engine->RenderFrame();
		engine->Update();
	}

	engine->Join();
	engine->Stop();
	delete engine;
    return 0;
}
