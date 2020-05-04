#include <CameraDolly.hpp>

#include <ComponentEngine\Engine.hpp>
#include <ComponentEngine\Common.hpp>
#include <ComponentEngine\Components\Transformation.hpp>
#include <ComponentEngine\Components\Camera.hpp>

#include <EnteeZ\EnteeZ.hpp>
#include <EnteeZ\Entity.hpp>

#include <imgui.h>
#include <sstream>

void ComponentEngine::CameraDolly::GenerateSnapshot(CameraDollySnapshot & snapshot)
{
	Camera* c = Engine::Singlton()->GetMainCamera();

	snapshot.transitionTime = 3.0f;
	snapshot.pauseTime = 1.0f;

	snapshot.transformation = c->GetTransformation()->GetLocalMat4();
	snapshot.fov = c->GetFOV();
	snapshot.aperture = c->GetAperture();
	snapshot.focusDistance = c->GetFocusDistance();

}

ComponentEngine::CameraDolly::CameraDolly(enteez::Entity * entity)
{
	m_entity = entity;
	deltaTime = 0.0f;
	currentIndex = -1;
}

void ComponentEngine::CameraDolly::Update(float frame_time)
{
	if (!running || snapshots.size() <= 1)return;

	deltaTime += frame_time;

	Engine* engine = Engine::Singlton();
	Camera* c = engine->GetMainCamera();

	CameraDollySnapshot start = snapshots[currentIndex];
	CameraDollySnapshot end = snapshots[(currentIndex + 1) % snapshots.size()];

	float lerpAmount = deltaTime / end.transitionTime;
	if (lerpAmount > 1.0f)lerpAmount = 1.0f;

	c->SetFOV(lerp(start.fov, end.fov, lerpAmount));
	c->SetAperture(lerp(start.aperture, end.aperture, lerpAmount));
	c->SetFocusDistance(lerp(start.focusDistance, end.focusDistance, lerpAmount));


	{
		glm::vec3 startScale;
		glm::quat startRotation;
		glm::vec3 startTranslation;
		glm::vec3 startSkew;
		glm::vec4 startPerspective;

		glm::vec3 endScale;
		glm::quat endRotation;
		glm::vec3 endTranslation;
		glm::vec3 endSkew;
		glm::vec4 endPerspective;
		glm::decompose(start.transformation, startScale, startRotation, startTranslation, startSkew, startPerspective);
		glm::decompose(end.transformation, endScale, endRotation, endTranslation, endSkew, endPerspective);


		glm::quat quat_interp = glm::slerp(startRotation, endRotation, lerpAmount);
		glm::vec3 pos_interp = glm::mix(startTranslation, endTranslation, lerpAmount);

		glm::mat4  view_matrix = glm::mat4_cast(quat_interp); // Setup rotation
		view_matrix[3] = glm::vec4(pos_interp, 1.0);  // Introduce translation

		c->GetTransformation()->SetLocalMat4(view_matrix);
	}



	/*
	// First interpolate the rotation
	glm::quat  quat_interp = glm::slerp(quat_start, quat_end, interp_factor);

	// Then interpolate the translation
	glm::vec3  pos_interp = glm::mix(eye_start, eye_end, interp_factor);

	glm::mat4  view_matrix = glm::mat4_cast(quat_interp); // Setup rotation
	view_matrix[3] = glm::vec4(pos_interp, 1.0);  // Introduce translation



	c->GetTransformation()->SetLocalMat4()*/


	c->UpdateProjection();

	if (deltaTime > end.pauseTime + end.transitionTime)
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
	ImGui::Text("Running");
	ImGui::SameLine();
	ImGui::Checkbox("##CameraDollyRunning", &running);

	if (ImGui::Button("Capture Position"))
	{
		CameraDollySnapshot snapshot;
		GenerateSnapshot(snapshot);
		snapshots.push_back(snapshot);
		currentIndex = 0;
	}

	ImGui::Separator();
	ImGui::Text("%i", snapshots.size());

	for (int i = 0; i < snapshots.size(); i++)
	{
		CameraDollySnapshot& snapshot = snapshots[i];
		ImGui::Separator();

		ImGui::PushID(&snapshot);

		if (ImGui::Button("X"))
		{
			snapshots.erase(snapshots.begin() + i);
			ImGui::PopID();
			if(snapshots.size()>0)
				currentIndex = 0;
			else
				currentIndex = -1;
			break;
		}
		ImGui::SameLine();
		if (ImGui::Button("ReCapture Position"))
		{
			GenerateSnapshot(snapshot);
		}

		Transformation::DisplayTransform(snapshot.transformation);

		ImGui::Text("Transition Time");
		ImGui::DragFloat("##TransitionTime", &snapshot.transitionTime, 0.1f, 0.1f, 180.0f);

		ImGui::Text("Pause Time");
		ImGui::DragFloat("##PauseTime", &snapshot.pauseTime, 0.1f, 0.1f, 180.0f);

		ImGui::Text("Aperture");
		ImGui::DragFloat("##Aperture", &snapshot.aperture, 0.1f, 0.1f, 1.0f);

		ImGui::Text("Focus Distance");
		ImGui::DragFloat("##FocusDistance", &snapshot.focusDistance, 0.1f, 0.1f, 100.0f);




		ImGui::Text("FOV");
		ImGui::DragFloat("##FOV", &snapshot.fov, 0.1f, 1.0f, 180.0f);


		ImGui::PopID();
	}
}

/*void ComponentEngine::CameraDolly::Load(std::ifstream & in)
{
	//ReadBinary(in, reinterpret_cast<char*>(this) + offsetof(CameraDolly, m_rotateSpeed), PayloadSize());
	int size = 0;
	Common::Read(in, &size, sizeof(int));
	snapshots.resize(size);
	for (auto& snapshot : snapshots)
	{
		ReadBinary(in, &snapshot, sizeof(CameraDollySnapshot));
		currentIndex = 0;
	}
}

void ComponentEngine::CameraDolly::Save(std::ofstream & out)
{
	int size = snapshots.size();
	Common::Write(out, &size, sizeof(int));
	for (auto& snapshot : snapshots)
	{
		WriteBinary(out, &snapshot, sizeof(CameraDollySnapshot));
	}
}*/


void ComponentEngine::CameraDolly::Load(pugi::xml_node& node)
{
	snapshots.clear();
	for (pugi::xml_node snapshotNode : node.children("Snapshot"))
	{
		CameraDollySnapshot snapshot;
		snapshot.transitionTime = snapshotNode.attribute("TransitionTime").as_float(snapshot.transitionTime);
		snapshot.pauseTime = snapshotNode.attribute("PauseTime").as_float(snapshot.pauseTime);

		pugi::xml_node Transformation = snapshotNode.child("Transformation");
		for (int row = 0; row < 4; row++)
		{
			pugi::xml_node rowNode = Transformation.child("Row");
			snapshot.transformation[row][0] = rowNode.attribute("C0").as_float(snapshot.transformation[row][0]);
			snapshot.transformation[row][1] = rowNode.attribute("C1").as_float(snapshot.transformation[row][1]);
			snapshot.transformation[row][2] = rowNode.attribute("C2").as_float(snapshot.transformation[row][2]);
			snapshot.transformation[row][3] = rowNode.attribute("C3").as_float(snapshot.transformation[row][3]);
		}
		snapshot.fov = snapshotNode.attribute("Fov").as_float(snapshot.fov);
		snapshot.aperture = snapshotNode.attribute("Aperture").as_float(snapshot.aperture);
		snapshot.focusDistance = snapshotNode.attribute("FocusDistance").as_float(snapshot.focusDistance);
		snapshots.push_back(snapshot);
	}
}

void ComponentEngine::CameraDolly::Save(pugi::xml_node& node)
{
	for (auto& snapshot : snapshots)
	{
		pugi::xml_node snapshotNode = node.append_child("Snapshot");
		snapshotNode.append_attribute("TransitionTime").set_value(snapshot.transitionTime);
		snapshotNode.append_attribute("PauseTime").set_value(snapshot.pauseTime);

		pugi::xml_node Transformation = snapshotNode.append_child("Transformation");
		for (int row = 0; row < 4; row++)
		{
			pugi::xml_node rowNode = Transformation.append_child("Row");
			rowNode.append_attribute("C0").set_value(snapshot.transformation[row][0]);
			rowNode.append_attribute("C1").set_value(snapshot.transformation[row][1]);
			rowNode.append_attribute("C2").set_value(snapshot.transformation[row][2]);
			rowNode.append_attribute("C3").set_value(snapshot.transformation[row][3]);
		}
		snapshotNode.append_attribute("Fov").set_value(snapshot.fov);
		snapshotNode.append_attribute("Aperture").set_value(snapshot.aperture);
		snapshotNode.append_attribute("FocusDistance").set_value(snapshot.focusDistance);
	}
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
