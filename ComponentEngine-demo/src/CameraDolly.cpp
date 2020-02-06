#include <CameraDolly.hpp>

#include <ComponentEngine\Engine.hpp>
#include <ComponentEngine\Components\Transformation.hpp>
#include <ComponentEngine\Components\Camera.hpp>

#include <EnteeZ\EnteeZ.hpp>
#include <EnteeZ\Entity.hpp>

#include <imgui.h>
#include <sstream>

ComponentEngine::CameraDolly::CameraDolly(enteez::Entity * entity)
{
	m_entity = entity;
	deltaTime = 0.0f;
	currentIndex = -1;
}

void ComponentEngine::CameraDolly::Update(float frame_time)
{
	if (snapshots.size() <= 1)return;

	deltaTime += frame_time;

	Engine* engine = Engine::Singlton();
	Camera* c = engine->GetMainCamera();

	CameraDollySnapshot start = snapshots[currentIndex];
	CameraDollySnapshot end = snapshots[(currentIndex + 1) % snapshots.size()];

	c->SetFOV(lerp(start.fov, end.fov, deltaTime / end.transitionTime));
	c->UpdateProjection();

	if (deltaTime > end.transitionTime)
	{
		deltaTime = 0.0f;
		currentIndex = (currentIndex + 1) % snapshots.size();
	}

}

void ComponentEngine::CameraDolly::EditorUpdate(float frame_time)
{

}

void ComponentEngine::CameraDolly::Display()
{
	
	if (ImGui::Button("Capture Position"))
	{
		Camera* c = Engine::Singlton()->GetMainCamera();

		CameraDollySnapshot snapshot;
		snapshot.transitionTime = 3.0f;


		snapshot.fov = c->GetFOV();


		snapshots.push_back(snapshot);
		currentIndex = 0;
	}

	ImGui::Separator();
	ImGui::Text("%i", snapshots.size());

	for (CameraDollySnapshot& snapshot : snapshots)
	{
		ImGui::Separator();

		ImGui::PushID(&snapshot);

		ImGui::Text("TransitionTime");
		ImGui::DragFloat("##TransitionTime", &snapshot.transitionTime, 0.1f, 0.1f, 180.0f);

		ImGui::Text("FOV");
		ImGui::DragFloat("##FOV", &snapshot.fov, 0.1f, 1.0f, 180.0f);


		ImGui::PopID();
	}
}

void ComponentEngine::CameraDolly::Load(std::ifstream & in)
{
	//ReadBinary(in, reinterpret_cast<char*>(this) + offsetof(CameraDolly, m_rotateSpeed), PayloadSize());
}

void ComponentEngine::CameraDolly::Save(std::ofstream & out)
{
	//WriteBinary(out, reinterpret_cast<char*>(this) + offsetof(CameraDolly, m_rotateSpeed), PayloadSize());
}

unsigned int ComponentEngine::CameraDolly::PayloadSize()
{
	return 0;//SizeOfOffsetRange(CameraDolly, m_rotateSpeed, m_lock_y);
}

bool ComponentEngine::CameraDolly::DynamiclySized()
{
	return false;
}

enteez::BaseComponentWrapper * ComponentEngine::CameraDolly::EntityHookDefault(enteez::Entity & entity)
{
	enteez::ComponentWrapper<CameraDolly>* wrapper = entity.AddComponent<CameraDolly>(&entity);
	wrapper->SetName("Camera Dolly");
	return wrapper;
}

float ComponentEngine::CameraDolly::lerp(float a, float b, float f)
{
	return a + f * (b - a);
}
