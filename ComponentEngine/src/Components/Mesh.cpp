#include <ComponentEngine\Components\Mesh.hpp>

#include <ComponentEngine\Engine.hpp>
#include <ComponentEngine\Common.hpp>

#include <ComponentEngine\ThreadHandler.hpp>
#include <ComponentEngine\DefaultMeshVertex.hpp>
#include <EnteeZ\EnteeZ.hpp>

#include <renderer\vulkan\VulkanTextureBuffer.hpp>
#include <renderer\vulkan\VulkanAcceleration.hpp>
#include <renderer\vulkan\VulkanModelPool.hpp>
#include <renderer\vulkan\VulkanModel.hpp>
#include <renderer\vulkan\VulkanRenderPass.hpp>

#include <imgui.h>
#include <glm/gtc/matrix_inverse.hpp>

#include <lodepng.h>

#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
#include <ComponentEngine\tiny_obj_loader.h>


using namespace ComponentEngine;

const unsigned int Mesh::m_buffer_size_step = 100;

// Key: File path, Value: Model instance
std::map<std::string, MeshInstance> m_mesh_instances;

ComponentEngine::Mesh::Mesh(enteez::Entity* entity) : m_entity(entity)
{
	m_materials_offsets = { 0 };
	m_loaded = false;
	m_vertex_count = 0;
	m_hit_group = 0;
	m_model = nullptr;
}

ComponentEngine::Mesh::Mesh(enteez::Entity* entity, std::string path) : /*MsgSend(entity),*/ m_entity(entity)
{
	m_materials_offsets = { 0 };
	m_loaded = false;
	m_vertex_count = 0;
	m_hit_group = 0;
	m_model = nullptr;
	ChangePath(path);
	LoadModel();

}

ComponentEngine::Mesh::~Mesh()
{
	if (m_loaded)
	{
		UnloadModel();
	}
}

void ComponentEngine::Mesh::ChangePath(std::string path)
{
	m_file_path = DropBoxInstance<FileForms>("Mesh");
	m_file_path.data.GenerateFileForm(path);
	m_file_path.SetMessage(m_file_path.data.shortForm);
	m_dir = Common::GetDir(m_file_path.data.longForm);
	LoadModel();
}

enteez::BaseComponentWrapper* ComponentEngine::Mesh::EntityHookDefault(enteez::Entity& entity)
{

	if (!entity.HasComponent<Mesh>())
	{
		enteez::ComponentWrapper<Mesh>* mesh = entity.AddComponent<Mesh>(&entity);
		mesh->SetName("Mesh");
		Send(mesh->Get().m_entity, mesh->Get().m_entity, OnComponentEnter<Mesh>(&mesh->Get()));
		return mesh;
	}
	return nullptr;///This is a bit shit and needs to be resolved. Need to dynamicly return the base component type
}

std::string ComponentEngine::Mesh::GetPath()
{
	return m_file_path.data.longForm;
}

bool ComponentEngine::Mesh::Loaded()
{
	return m_loaded;
}

void ComponentEngine::Mesh::ReciveMessage(enteez::Entity * sender, RenderStatus& message)
{
	if (!m_loaded)return;
	m_model->ShouldRender(message.should_renderer);
}

void ComponentEngine::Mesh::ReciveMessage(enteez::Entity * sender, OnComponentEnter<Transformation>& message)
{

}

void ComponentEngine::Mesh::ReciveMessage(enteez::Entity * sender, OnComponentExit<Transformation>& message)
{

}

void ComponentEngine::Mesh::Display()
{

	if (m_loaded)
	{
		ImGui::PushID(this);

		ImGui::Text("Vertex Count: %d", m_vertex_count);

		std::vector<HitShaderPipeline>& hitgroups = Engine::Singlton()->GetHitShaderPipelines();

		ImGui::Text("Hitgroup");

		Engine* engine = Engine::Singlton();
		VulkanAcceleration* as = engine->GetTopLevelAS();

		m_hit_group = as->GetModelPoolHitGroupOffset(m_model_pool);
		std::string hitgroup_name = hitgroups[m_hit_group].name;

		if (ImGui::BeginCombo("##HitgroupSelection", hitgroup_name.c_str()))
		{
			for (int i = 0 ; i < hitgroups.size(); i ++)
			{
				HitShaderPipeline& hitgroup = hitgroups[i];
				if (!hitgroup.primaryHitgroup) continue;
				bool is_selected = (hitgroup_name == hitgroup.name);
				if (ImGui::Selectable(hitgroup.name.c_str(), is_selected))
				{
					as->SetModelPoolHitGroupOffset(m_model_pool, i);
					engine->GetRenderPass()->Rebuild();
				}
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}


		for (int i = 0 ; i < m_materials.size(); i++)
		{
			MaterialFileForms& fileForms = m_materials[i];
			ImGui::PushID(&fileForms);

			bool open = ImGui::TreeNodeEx("Folder", ImGuiTreeNodeFlags_None, "%i", i);

			if (open)
			{
				{ // diffuse_texture
					ImGui::PushID(&fileForms.diffuse_texture);
					DropBoxInstance<FileForms> tempFilePath = fileForms.diffuse_texture;
					if (UIManager::DropBox("Texture", "File", tempFilePath))
					{
						if (tempFilePath.data.extension == ".png" && fileForms.diffuse_texture.data.longForm != tempFilePath.data.longForm)
						{
							Engine* engine = Engine::Singlton();
							// Load the material definition
							MaterialDefintion definition = engine->GetMaterialDefinition(m_materials_offsets[i]);
							// Set the new texture
							definition.diffuse_texture = tempFilePath.data.longForm;
							

							// Get the models global material instance
							MatrialObj global_material = Engine::Singlton()->GetGlobalMaterialArray()[m_materials_offsets[i]];
							// Load the texture
							engine->LoadTexture(tempFilePath.data.longForm);
							// Get the texture id and store it in the material
							global_material.textureID = engine->GetTextureOffset(tempFilePath.data.longForm);

							// Register the new material combination
							engine->RegisterMaterial(definition, global_material);
							m_materials_offsets[i] = engine->GetMaterialOffset(definition);

							// Set the models new materials

							Engine::Singlton()->GetModelLoadMutex().lock();
							m_model->SetData(1, m_materials_offsets);
							Engine::Singlton()->GetModelLoadMutex().unlock();

							// Tell the GPU of the update
							engine->UpdateAccelerationDependancys();

							UpdateMaterials();
						}
					}
					ImGui::PopID();
				}

				{ // metalic_texture
					ImGui::PushID(&fileForms.metalic_texture);
					DropBoxInstance<FileForms> tempFilePath = fileForms.metalic_texture;
					if (UIManager::DropBox("Metallic Texture", "File", tempFilePath))
					{
						if (tempFilePath.data.extension == ".png" && fileForms.metalic_texture.data.longForm != tempFilePath.data.longForm)
						{
							Engine* engine = Engine::Singlton();
							// Load the material definition
							MaterialDefintion definition = engine->GetMaterialDefinition(m_materials_offsets[i]);
							// Set the new texture
							definition.metalic_texture = tempFilePath.data.longForm;


							// Get the models global material instance
							MatrialObj global_material = Engine::Singlton()->GetGlobalMaterialArray()[m_materials_offsets[i]];
							// Load the texture
							engine->LoadTexture(tempFilePath.data.longForm);
							// Get the texture id and store it in the material
							global_material.metalicTextureID = engine->GetTextureOffset(tempFilePath.data.longForm);

							// Register the new material combination
							engine->RegisterMaterial(definition, global_material);
							m_materials_offsets[i] = engine->GetMaterialOffset(definition);

							// Set the models new materials

							Engine::Singlton()->GetModelLoadMutex().lock();
							m_model->SetData(1, m_materials_offsets);
							Engine::Singlton()->GetModelLoadMutex().unlock();

							// Tell the GPU of the update
							engine->UpdateAccelerationDependancys();

							UpdateMaterials();
						}
					}
					ImGui::PopID();
				}

				{ // roughness_texture
					ImGui::PushID(&fileForms.roughness_texture);
					DropBoxInstance<FileForms> tempFilePath = fileForms.roughness_texture;
					if (UIManager::DropBox("Roughness Texture", "File", tempFilePath))
					{
						if (tempFilePath.data.extension == ".png" && fileForms.roughness_texture.data.longForm != tempFilePath.data.longForm)
						{
							Engine* engine = Engine::Singlton();
							// Load the material definition
							MaterialDefintion definition = engine->GetMaterialDefinition(m_materials_offsets[i]);
							// Set the new texture
							definition.roughness_texture = tempFilePath.data.longForm;


							// Get the models global material instance
							MatrialObj global_material = Engine::Singlton()->GetGlobalMaterialArray()[m_materials_offsets[i]];
							// Load the texture
							engine->LoadTexture(tempFilePath.data.longForm);
							// Get the texture id and store it in the material
							global_material.roughnessTextureID = engine->GetTextureOffset(tempFilePath.data.longForm);

							// Register the new material combination
							engine->RegisterMaterial(definition, global_material);
							m_materials_offsets[i] = engine->GetMaterialOffset(definition);

							// Set the models new materials

							Engine::Singlton()->GetModelLoadMutex().lock();
							m_model->SetData(1, m_materials_offsets);
							Engine::Singlton()->GetModelLoadMutex().unlock();

							// Tell the GPU of the update
							engine->UpdateAccelerationDependancys();

							UpdateMaterials();
						}
					}
					ImGui::PopID();
				}

				{ // normal_texture
					ImGui::PushID(&fileForms.normal_texture);
					DropBoxInstance<FileForms> tempFilePath = fileForms.normal_texture;
					if (UIManager::DropBox("Normal Texture", "File", tempFilePath))
					{
						if (tempFilePath.data.extension == ".png" && fileForms.normal_texture.data.longForm != tempFilePath.data.longForm)
						{
							Engine* engine = Engine::Singlton();
							// Load the material definition
							MaterialDefintion definition = engine->GetMaterialDefinition(m_materials_offsets[i]);
							// Set the new texture
							definition.normal_texture = tempFilePath.data.longForm;


							// Get the models global material instance
							MatrialObj global_material = Engine::Singlton()->GetGlobalMaterialArray()[m_materials_offsets[i]];
							// Load the texture
							engine->LoadTexture(tempFilePath.data.longForm);
							// Get the texture id and store it in the material
							global_material.normalTextureID = engine->GetTextureOffset(tempFilePath.data.longForm);

							// Register the new material combination
							engine->RegisterMaterial(definition, global_material);
							m_materials_offsets[i] = engine->GetMaterialOffset(definition);

							// Set the models new materials

							Engine::Singlton()->GetModelLoadMutex().lock();
							m_model->SetData(1, m_materials_offsets);
							Engine::Singlton()->GetModelLoadMutex().unlock();

							// Tell the GPU of the update
							engine->UpdateAccelerationDependancys();

							UpdateMaterials();
						}
					}
					ImGui::PopID();
				}


				{ // normal_texture
					ImGui::PushID(&fileForms.cavity_texture);
					DropBoxInstance<FileForms> tempFilePath = fileForms.cavity_texture;
					if (UIManager::DropBox("Cavity Texture", "File", tempFilePath))
					{
						if (tempFilePath.data.extension == ".png" && fileForms.cavity_texture.data.longForm != tempFilePath.data.longForm)
						{
							Engine* engine = Engine::Singlton();
							// Load the material definition
							MaterialDefintion definition = engine->GetMaterialDefinition(m_materials_offsets[i]);
							// Set the new texture
							definition.cavity_texture = tempFilePath.data.longForm;


							// Get the models global material instance
							MatrialObj global_material = Engine::Singlton()->GetGlobalMaterialArray()[m_materials_offsets[i]];
							// Load the texture
							engine->LoadTexture(tempFilePath.data.longForm);
							// Get the texture id and store it in the material
							global_material.cavityTextureID = engine->GetTextureOffset(tempFilePath.data.longForm);

							// Register the new material combination
							engine->RegisterMaterial(definition, global_material);
							m_materials_offsets[i] = engine->GetMaterialOffset(definition);

							// Set the models new materials

							Engine::Singlton()->GetModelLoadMutex().lock();
							m_model->SetData(1, m_materials_offsets);
							Engine::Singlton()->GetModelLoadMutex().unlock();

							// Tell the GPU of the update
							engine->UpdateAccelerationDependancys();

							UpdateMaterials();
						}
					}
					ImGui::PopID();
				}


				{ // normal_texture
					ImGui::PushID(&fileForms.ao_texture);
					DropBoxInstance<FileForms> tempFilePath = fileForms.ao_texture;
					if (UIManager::DropBox("AO Texture", "File", tempFilePath))
					{
						if (tempFilePath.data.extension == ".png" && fileForms.ao_texture.data.longForm != tempFilePath.data.longForm)
						{
							Engine* engine = Engine::Singlton();
							// Load the material definition
							MaterialDefintion definition = engine->GetMaterialDefinition(m_materials_offsets[i]);
							// Set the new texture
							definition.ao_texture = tempFilePath.data.longForm;


							// Get the models global material instance
							MatrialObj global_material = Engine::Singlton()->GetGlobalMaterialArray()[m_materials_offsets[i]];
							// Load the texture
							engine->LoadTexture(tempFilePath.data.longForm);
							// Get the texture id and store it in the material
							global_material.aoTextureID = engine->GetTextureOffset(tempFilePath.data.longForm);

							// Register the new material combination
							engine->RegisterMaterial(definition, global_material);
							m_materials_offsets[i] = engine->GetMaterialOffset(definition);

							// Set the models new materials

							Engine::Singlton()->GetModelLoadMutex().lock();
							m_model->SetData(1, m_materials_offsets);
							Engine::Singlton()->GetModelLoadMutex().unlock();

							// Tell the GPU of the update
							engine->UpdateAccelerationDependancys();

							UpdateMaterials();
						}
					}
					ImGui::PopID();
				}

				{ // normal_texture
					ImGui::PushID(&fileForms.height_texture);
					DropBoxInstance<FileForms> tempFilePath = fileForms.height_texture;
					if (UIManager::DropBox("Height Texture", "File", tempFilePath))
					{
						if (tempFilePath.data.extension == ".png" && fileForms.height_texture.data.longForm != tempFilePath.data.longForm)
						{
							Engine* engine = Engine::Singlton();
							// Load the material definition
							MaterialDefintion definition = engine->GetMaterialDefinition(m_materials_offsets[i]);
							// Set the new texture
							definition.height_texture = tempFilePath.data.longForm;


							// Get the models global material instance
							MatrialObj global_material = Engine::Singlton()->GetGlobalMaterialArray()[m_materials_offsets[i]];
							// Load the texture
							engine->LoadTexture(tempFilePath.data.longForm);
							// Get the texture id and store it in the material
							global_material.heightTextureID = engine->GetTextureOffset(tempFilePath.data.longForm);

							// Register the new material combination
							engine->RegisterMaterial(definition, global_material);
							m_materials_offsets[i] = engine->GetMaterialOffset(definition);

							// Set the models new materials

							Engine::Singlton()->GetModelLoadMutex().lock();
							m_model->SetData(1, m_materials_offsets);
							Engine::Singlton()->GetModelLoadMutex().unlock();

							// Tell the GPU of the update
							engine->UpdateAccelerationDependancys();

							UpdateMaterials();
						}
					}
					ImGui::PopID();
				}

				ImGui::TreePop();
			}


			ImGui::PopID();
		}

		

		ImGui::PopID();
	}

	DropBoxInstance<FileForms> tempFilePath = m_file_path;
	if (UIManager::DropBox("Mesh File", "File", tempFilePath))
	{
		if (tempFilePath.data.extension == ".obj" && m_file_path.data.longForm != tempFilePath.data.longForm)
		{
			Engine& engine = *Engine::Singlton();
			engine.GetLogicMutex().lock();
			engine.GetRendererMutex().lock();
			tempFilePath.SetMessage(tempFilePath.data.shortForm);
			//if (m_loaded)UnloadModel();

			ChangePath(tempFilePath.data.longForm);
			//m_file_path = tempFilePath;
			//std::cout << m_file_path.data.longForm << std::endl;
			engine.GetRendererMutex().unlock();
			engine.GetLogicMutex().unlock();
		}
	}




}

void ComponentEngine::Mesh::Load(std::ifstream & in)
{
	ReadBinary(in, reinterpret_cast<char*>(this) + offsetof(Mesh, m_hit_group), SizeOfOffsetRange(Mesh, m_hit_group, m_hit_group));
	std::string path = Common::ReadString(in);

	// Material count
	int materialCount = 0;
	ComponentEngine::Common::Read(in, &materialCount, sizeof(int));

	std::vector<MaterialDefintion> definitions;
	definitions.resize(materialCount);

	for (int i = 0; i < materialCount; i++)
	{
		MaterialDefintion& definition = definitions[i];

		definition.diffuse_texture = Common::ReadString(in);
		definition.metalic_texture = Common::ReadString(in);
		definition.roughness_texture = Common::ReadString(in);
		definition.normal_texture = Common::ReadString(in);
		definition.cavity_texture = Common::ReadString(in);
		definition.ao_texture = Common::ReadString(in);
		definition.height_texture = Common::ReadString(in);
	}

	if (path.size()> 0)
	{
		loading_definitions = definitions;
		ChangePath(path);



	}
}

void ComponentEngine::Mesh::Save(std::ofstream & out)
{
	WriteBinary(out, reinterpret_cast<char*>(this) + offsetof(Mesh, m_hit_group), SizeOfOffsetRange(Mesh, m_hit_group, m_hit_group));

	Common::Write(out, m_file_path.data.longForm);


	if (m_file_path.data.longForm.size() > 0)
	{

		// Material count
		int materialCount = m_mesh_instances[m_file_path.data.longForm].materialCount;
		Common::Write(out, &materialCount, sizeof(int));

		for (int i = 0; i < materialCount; i++)
		{
			MaterialDefintion definition = Engine::Singlton()->GetMaterialDefinition(m_materials_offsets[i]);

			Common::Write(out, definition.diffuse_texture);
			Common::Write(out, definition.metalic_texture);
			Common::Write(out, definition.roughness_texture);
			Common::Write(out, definition.normal_texture);
			Common::Write(out, definition.cavity_texture);
			Common::Write(out, definition.ao_texture);
			Common::Write(out, definition.height_texture);
		}
	}




}

unsigned int ComponentEngine::Mesh::PayloadSize()
{
	return SizeOfOffsetRange(Mesh, m_hit_group, m_hit_group) + Common::StreamStringSize(m_file_path.data.longForm);
}

bool ComponentEngine::Mesh::DynamiclySized()
{
	return true;
}

glm::mat4 rotation2(1.0f);
void ComponentEngine::Mesh::Update(float frame_time)
{
	glm::mat4 rotation(1.0f);
	//rotation = glm::rotate(rotation, 180.0f, glm::vec3(0, 1, 0));
	//rotation = glm::rotate(rotation, 90.0f, glm::vec3(0, 0, 1));
	rotation2 = glm::rotate(rotation2, 0.1f, glm::vec3(1, 0, 0));
	if (m_loaded)
	{
		glm::mat4 mat = m_entity->GetComponent<Transformation>().GetMat4();

		m_model->SetData(0, mat);

		m_model->SetData(1, glm::inverseTranspose(mat));

		m_model->SetData(2, m_materials_offsets);
	}
}


void ComponentEngine::Mesh::EditorUpdate(float frame_time)
{
	if (m_loaded)
	{
		glm::mat4 mat = m_entity->GetComponent<Transformation>().GetMat4();

		m_model->SetData(0, mat);

		m_model->SetData(1, glm::inverseTranspose(mat));

		m_model->SetData(2, m_materials_offsets);
	}
}

void ComponentEngine::Mesh::LoadModel()
{

	if (m_loaded)
	{
		UnloadModel();
	}
	m_loaded = false;


	Engine::Singlton()->GetThreadManager()->AddTask([&](float delta)
	{
		Engine* engine = Engine::Singlton();




		engine->GetModelLoadMutex().lock();

		bool notLoaded = m_mesh_instances.find(m_file_path.data.longForm) == m_mesh_instances.end();
		bool loading = false;
		if (!notLoaded)
		{
			loading = m_mesh_instances[m_file_path.data.longForm].loading;
			if (loading)
			{
				Pending pendingModel;
				pendingModel.mesh = this;
				pendingModel.definitions = loading_definitions;
				m_mesh_instances[m_file_path.data.longForm].pending_models.push_back(pendingModel);
				engine->GetModelLoadMutex().unlock();
				return;
			}
		}
		else
		{
			// Tell the system we are loading the mesh
			m_mesh_instances[m_file_path.data.longForm].loading = true;
		}
		engine->GetModelLoadMutex().unlock();





		VulkanAcceleration* as = engine->GetTopLevelAS();
		
		if (notLoaded)
		{

			VulkanVertexBuffer* vertexBuffer = engine->GetGlobalVertexBufer();
			VulkanIndexBuffer* indexBuffer = engine->GetGlobalIndexBuffer();

			std::vector<uint32_t>& all_indexs = engine->GetGlobalIndexArray();
			std::vector<MeshVertex>& all_vertexs = engine->GetGlobalVertexArray();
			VulkanBufferPool* position_buffer_pool = engine->GetPositionBufferPool();
			VulkanBufferPool* position_buffer_it_pool = engine->GetPositionITBufferPool();
			VulkanBufferPool* material_mapping_pool = engine->GetMaterialMappingPool();


			{
				// Make sure the file exists
				std::ifstream file(m_file_path.data.longForm, std::ios::ate | std::ios::binary);
				if (!file.is_open())
				{
					engine->Log("Could not find model");
					engine->GetModelLoadMutex().lock();
					m_mesh_instances[m_file_path.data.longForm].loading = false;
					engine->GetModelLoadMutex().unlock();
					return;
				}
				file.close();
			}

			// Load the model
			ObjLoader<MeshVertex> loader;
			loader.loadModel(m_file_path.data.longForm);

			unsigned int vertexStart = 0;
			unsigned int indexStart = 0;

			uint32_t m_nbVertices = static_cast<uint32_t>(loader.m_vertices.size());
			uint32_t m_nbIndices = static_cast<uint32_t>(loader.m_indices.size());



			{ // Load the verticies and indicies into memory
				engine->GetModelLoadMutex().lock();

				unsigned int& used_vertex = engine->GetUsedVertex();
				unsigned int& used_index = engine->GetUsedIndex();

				vertexStart = used_vertex;
				indexStart = used_index;


				for (uint32_t& index : loader.m_indices)
				{
					all_indexs[used_index] = index;
					used_index++; 
				}

				for (MeshVertex& vertex : loader.m_vertices)
				{
					all_vertexs[used_vertex] = vertex;
					used_vertex++;
				}

				engine->GetModelLoadMutex().unlock();
			}


			std::vector<int> loadedTextures(loader.m_textures.size());
			for (int i = 0; i < loader.m_textures.size(); i++)
			{
				std::stringstream ss;
				ss << m_dir << loader.m_textures[i];

				engine->LoadTexture(ss.str());
				loadedTextures[i] = engine->GetTextureOffset(ss.str());
			}


			m_materials.clear();
			m_materials.resize(loader.m_materials.size());


			std::array<int, 512> materialDefintionMap;


			for (int i = 0; i < loader.m_materials.size(); i++)
			{
				MatrialObj material = loader.m_materials[i];
				MaterialFileForms& fileForm = m_materials[i];
				MaterialDefintion materialDefinition;


				if (material.textureID >= 0 && loadedTextures[material.textureID] >= 0)
				{
					std::stringstream ss;
					ss << m_dir << loader.m_textures[material.textureID];
					materialDefinition.diffuse_texture = ss.str();
					material.textureID = loadedTextures[material.textureID];
				}
				else
					material.textureID = 0; // Set to default white texture

				if (material.metalicTextureID >= 0 && loadedTextures[material.metalicTextureID] >= 0)
				{
					std::stringstream ss;
					ss << m_dir << loader.m_textures[material.metalicTextureID];
					materialDefinition.metalic_texture = ss.str();
					material.metalicTextureID = loadedTextures[material.metalicTextureID];
				}
				else
					material.metalicTextureID = 1; // Set to default black texture

				if (material.roughnessTextureID >= 0 && loadedTextures[material.roughnessTextureID] >= 0)
				{
					std::stringstream ss;
					ss << m_dir << loader.m_textures[material.roughnessTextureID];
					materialDefinition.roughness_texture = ss.str();
					material.roughnessTextureID = loadedTextures[material.roughnessTextureID];
				}
				else
					material.roughnessTextureID = 0; // Set to default white texture

				if (material.normalTextureID >= 0 && loadedTextures[material.normalTextureID] >= 0)
				{
					std::stringstream ss;
					ss << m_dir << loader.m_textures[material.normalTextureID];
					materialDefinition.normal_texture = ss.str();
					material.normalTextureID = loadedTextures[material.normalTextureID];
				}
				else
					material.normalTextureID = 2; // Set to default normal texture


				material.cavityTextureID = 0; // Set to default white texture
				material.aoTextureID = 0; // Set to default white texture
				material.heightTextureID = 3; // Set to default white texture


				engine->RegisterMaterial(materialDefinition, material);
				materialDefintionMap[i] = engine->GetMaterialOffset(materialDefinition);
			}

			m_mesh_instances[m_file_path.data.longForm].mesh_instance = engine->GetRenderer()->CreateModelPool(vertexBuffer, vertexStart, m_nbVertices, indexBuffer, indexStart, m_nbIndices, ModelPoolUsage::SingleMesh);

			// For normal position
			m_mesh_instances[m_file_path.data.longForm].mesh_instance->AttachBufferPool(0, position_buffer_pool);

			// For inverse position
			m_mesh_instances[m_file_path.data.longForm].mesh_instance->AttachBufferPool(1, position_buffer_it_pool);

			m_mesh_instances[m_file_path.data.longForm].mesh_instance->AttachBufferPool(2, material_mapping_pool);


			engine->GetModelLoadMutex().lock();
			as->AttachModelPool(m_mesh_instances[m_file_path.data.longForm].mesh_instance, m_hit_group);
			engine->GetModelLoadMutex().unlock();

			{ // Finish, clean up and setup pending models
				engine->GetModelLoadMutex().lock();
				m_mesh_instances[m_file_path.data.longForm].defaultMaterialMap = materialDefintionMap;
				m_mesh_instances[m_file_path.data.longForm].loading = false;
				m_mesh_instances[m_file_path.data.longForm].materialCount = loader.m_materials.size();

				for (Pending pendingMesh : m_mesh_instances[m_file_path.data.longForm].pending_models)
				{
					pendingMesh.mesh->InstanciateModel(pendingMesh.definitions);
				}

				m_mesh_instances[m_file_path.data.longForm].pending_models.clear();
				engine->GetModelLoadMutex().unlock();
			}

		}

		InstanciateModel(loading_definitions);

	},"ModelLoader");
}

void ComponentEngine::Mesh::UnloadModel()
{
	if (m_loaded)
	{

		Engine::Singlton()->GetModelLoadMutex().lock();
		m_model->Remove();
		Engine::Singlton()->GetModelLoadMutex().unlock();
		Engine::Singlton()->UpdateAccelerationDependancys();
		m_loaded = false;
	}
}

int ComponentEngine::Mesh::GetUUID()
{
	if (m_model == nullptr)return -1;
	return m_model->GetUUID();
}

void ComponentEngine::Mesh::ChangePath(DropBoxInstance<FileForms>& p, std::string path)
{
	p = DropBoxInstance<FileForms>("Texture");
	p.data.GenerateFileForm(path);
	p.SetMessage(p.data.shortForm);
}

void ComponentEngine::Mesh::InstanciateModel(std::vector<MaterialDefintion> definitions)
{
	m_model_pool = m_mesh_instances[m_file_path.data.longForm].mesh_instance;

	m_vertex_count = m_model_pool->GetVertexSize();

	// Create a instance of the model

	Engine::Singlton()->GetModelLoadMutex().lock();
	m_model = m_model_pool->CreateModel();

	m_model->SetData(0, m_entity->GetComponent<Transformation>().Get());

	m_model->SetData(1, m_mesh_instances[m_file_path.data.longForm].defaultMaterialMap);

	m_materials_offsets = m_mesh_instances[m_file_path.data.longForm].defaultMaterialMap;

	Engine::Singlton()->GetModelLoadMutex().unlock();

	//ChangePath(fileForm.normal_texture, loader.m_textures[material.normalTextureID]);

	for (int i = 0; i < definitions.size(); i++)
	{
		SetMaterial(i, definitions[i]);
	}

	UpdateMaterials();

	Engine::Singlton()->UpdateAccelerationDependancys();

	m_loaded = true;
}

void ComponentEngine::Mesh::UpdateMaterials()
{
	int materialCount = m_mesh_instances[m_file_path.data.longForm].materialCount;
	m_materials.resize(materialCount);



	for (int i = 0; i < materialCount; i++)
	{
		MaterialDefintion definition = Engine::Singlton()->GetMaterialDefinition(m_materials_offsets[i]);

		ChangePath(m_materials[i].diffuse_texture, definition.diffuse_texture);
		ChangePath(m_materials[i].metalic_texture, definition.metalic_texture);
		ChangePath(m_materials[i].roughness_texture, definition.roughness_texture);
		ChangePath(m_materials[i].normal_texture, definition.normal_texture);
		ChangePath(m_materials[i].cavity_texture, definition.cavity_texture);
		ChangePath(m_materials[i].ao_texture, definition.ao_texture);
		ChangePath(m_materials[i].height_texture, definition.height_texture);
	}

	
}

void ComponentEngine::Mesh::SetMaterial(int index, MaterialDefintion definition)
{
	Engine* engine = Engine::Singlton();

	// Get the models global material instance
	MatrialObj global_material = engine->GetGlobalMaterialArray()[m_materials_offsets[index]];
	// Load the texture
	if(definition.diffuse_texture.size()>0)
	{

		std::stringstream ss;
		ss  << definition.diffuse_texture;
		engine->LoadTexture(ss.str());
		global_material.textureID = engine->GetTextureOffset(ss.str());
	}
	if (definition.metalic_texture.size()>0)
	{

		std::stringstream ss;
		ss  << definition.metalic_texture;
		engine->LoadTexture(ss.str());
		global_material.metalicTextureID = engine->GetTextureOffset(ss.str());
	}
	if (definition.roughness_texture.size()>0)
	{

		std::stringstream ss;
		ss  << definition.roughness_texture;
		engine->LoadTexture(ss.str());
		global_material.roughnessTextureID = engine->GetTextureOffset(ss.str());
	}
	if (definition.normal_texture.size()>0)
	{

		std::stringstream ss;
		ss << definition.normal_texture;
		engine->LoadTexture(ss.str());
		global_material.normalTextureID = engine->GetTextureOffset(ss.str());
	}
	if (definition.cavity_texture.size()>0)
	{

		std::stringstream ss;
		ss << definition.cavity_texture;
		engine->LoadTexture(ss.str());
		global_material.cavityTextureID = engine->GetTextureOffset(ss.str());
	}
	if (definition.ao_texture.size()>0)
	{

		std::stringstream ss;
		ss << definition.ao_texture;
		engine->LoadTexture(ss.str());
		global_material.aoTextureID = engine->GetTextureOffset(ss.str());
	}
	if (definition.height_texture.size()>0)
	{

		std::stringstream ss;
		ss << definition.height_texture;
		engine->LoadTexture(ss.str());
		global_material.heightTextureID = engine->GetTextureOffset(ss.str());
	}


	// Register the new material combination
	engine->RegisterMaterial(definition, global_material);
	m_materials_offsets[index] = engine->GetMaterialOffset(definition);


	// Set the models new materials
	Engine::Singlton()->GetModelLoadMutex().lock();
	m_model->SetData(1, m_materials_offsets);
	Engine::Singlton()->GetModelLoadMutex().unlock();


}
