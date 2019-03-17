#include <ComponentEngine\Components\ParticalSystem.hpp>
#include <ComponentEngine\Engine.hpp>
#include <ComponentEngine\Common.hpp>

#include <ComponentEngine\pugixml.hpp>
#include <EnteeZ\EnteeZ.hpp>

#include <assert.h>
#include <imgui.h>

ComponentEngine::ParticleSystem::ParticleSystem(enteez::Entity * entity)
{
	m_entity = entity;

	// Initilize the system with default values
	m_config.buffer_config.memory.updateTime = 0;
	m_config.buffer_config.memory.totalTime = 0;
	m_config.buffer_config.memory.maxLife = 2.0f;
	m_config.buffer_config.memory.emissionRate = 0.001f;
	m_config.buffer_config.memory.startColor = glm::vec4(0.976f, 0.941f, 0.475f, 1.0f);
	m_config.buffer_config.memory.endColor = glm::vec4(0.965f, 0.486f, 0.004f, 1.0f);
	m_config.buffer_config.memory.scale = 0.2f;

	m_config.xVelocity = glm::vec2(1.0f, -1.0f);
	m_config.yVelocity = glm::vec2(1.0f, -1.0f);
	m_config.zVelocity = glm::vec2(1.0f, -1.0f);

	m_config.emmitter_offset = glm::vec3();

	m_config.directionalVelocity = 2.0f;

	// Set the particle count to the count based on emission rates
	m_particleCount = m_config.buffer_config.memory.maxLife / m_config.buffer_config.memory.emissionRate;

	Build();
}

ComponentEngine::ParticleSystem::~ParticleSystem()
{
	Engine::Singlton()->GetRendererMutex().lock();
	Engine::Singlton()->GetRenderer()->RemoveGraphicsPipeline(m_graphics_pipeline);
	delete m_graphics_pipeline;
	delete m_program;
	delete m_compute_pipeline;
	delete m_compute_program;

	delete m_model;
	delete m_model_pool;

	// Particle Payloads
	delete m_config_buffer;
	// Particle Payloads
	delete m_particle_payload_buffer;

	delete m_partical_vertex_pool;
	delete m_partical_vertex_set;
	delete m_vertex_buffer;
	Engine::Singlton()->GetRendererMutex().unlock();

}

void ComponentEngine::ParticleSystem::Build()
{

	ComponentEngine::Engine* engine = Engine::Singlton();
	engine->GetRendererMutex().lock();
	m_particle_lock.lock();

	Renderer::IRenderer* renderer = engine->GetRenderer();

	m_vertex_data.clear();
	m_vertex_data.resize(m_particleCount);
	// Have to store the data as a vec 4 as the GPU aligns memory to 4,8 and 16 bits, not 12 for vec3
	m_vertex_buffer = renderer->CreateVertexBuffer(m_vertex_data.data(), sizeof(ParticleInstance), m_vertex_data.size());

	m_config_buffer = renderer->CreateUniformBuffer(&m_config.buffer_config, BufferChain::Single, sizeof(ParticleSystemBufferConfig), 1, true);

	m_config_buffer->SetData(BufferSlot::Primary);


	m_particle_payloads.clear();
	m_particle_payloads.resize(m_particleCount);

	m_particle_payload_buffer = renderer->CreateUniformBuffer(m_particle_payloads.data(), BufferChain::Single,sizeof(ParticlePayload), m_particle_payloads.size(), true);

	m_compute_pipeline = renderer->CreateComputePipeline("../Shaders/ParticalEffect/comp.spv", m_particleCount, 1, 1);

	RebuildAll();

	{
		m_partical_vertex_pool = renderer->CreateDescriptorPool({
			renderer->CreateDescriptor(Renderer::DescriptorType::STORAGE_BUFFER, Renderer::ShaderStage::COMPUTE_SHADER, 0),
			renderer->CreateDescriptor(Renderer::DescriptorType::STORAGE_BUFFER, Renderer::ShaderStage::COMPUTE_SHADER, 1),
			renderer->CreateDescriptor(Renderer::DescriptorType::STORAGE_BUFFER, Renderer::ShaderStage::COMPUTE_SHADER, 2)
			});
		
		// Create descriptor set from the tempalte
		m_partical_vertex_set = m_partical_vertex_pool->CreateDescriptorSet();
		// Attach the buffer

		m_partical_vertex_set->AttachBuffer(0, m_vertex_buffer);
		m_partical_vertex_set->AttachBuffer(1, m_config_buffer);
		m_partical_vertex_set->AttachBuffer(2, m_particle_payload_buffer); 
		m_partical_vertex_set->UpdateSet();

		m_compute_pipeline->AttachDescriptorPool(m_partical_vertex_pool);
		m_compute_pipeline->AttachDescriptorSet(0, m_partical_vertex_set);
	}

	m_compute_pipeline->Build();
	
	m_compute_program = renderer->CreateComputeProgram();
	m_compute_program->AttachPipeline(m_compute_pipeline);
	m_compute_program->Build();


	m_model_pool = renderer->CreateModelPool(m_vertex_buffer);

	m_model = m_model_pool->CreateModel();

	m_model->ShouldRender(true);

	{

		// Create default pipeline
		m_graphics_pipeline = renderer->CreateGraphicsPipeline({
			{ ShaderStage::VERTEX_SHADER, "../Shaders/ParticalEffect/vert.spv" },
			{ ShaderStage::GEOMETRY_SHADER, "../Shaders/ParticalEffect/geom.spv" },
			{ ShaderStage::FRAGMENT_SHADER, "../Shaders/ParticalEffect/frag.spv" },
			});

		// Tell the pipeline what data is should expect in the forum of Vertex input
		m_graphics_pipeline->AttachVertexBinding({
			VertexInputRate::INPUT_RATE_VERTEX,
			{
				{ 0, DataFormat::R32G32B32A32_FLOAT, offsetof(ParticleInstance,position) }, // x,y,z : pos
				{ 1, DataFormat::R32G32B32A32_FLOAT, offsetof(ParticleInstance,color) }
			},
			sizeof(ParticleInstance),
			0
			});



		// Tell the pipeline what the input data will be payed out like
		m_graphics_pipeline->AttachDescriptorPool(engine->GetCameraPool());
		// Attach the camera descriptor set to the pipeline
		m_graphics_pipeline->AttachDescriptorSet(0, engine->GetCameraDescriptorSet());

		m_graphics_pipeline->UseCulling(false);

		m_graphics_pipeline->DefinePrimitiveTopology(PrimitiveTopology::PointList);

		bool sucsess = m_graphics_pipeline->Build();

		assert(sucsess && "Unable to create partcal effect graphics shader");

		m_graphics_pipeline->AttachModelPool(m_model_pool);

	}





	m_particle_lock.unlock();
	engine->GetRendererMutex().unlock();


}

void ComponentEngine::ParticleSystem::Update(float frame_time)
{
	if (!m_running)return;
	ComponentEngine::Engine* engine = Engine::Singlton();
	m_particle_lock.lock();
	engine->GetRendererMutex().lock();

	if (!m_ran_initial_build)
	{
		m_ran_initial_build = true;
		RebuildAll();
	}

	// Get the models matrix
	glm::mat4 positionMatrix = m_entity->GetComponent<Transformation>().Get();

	glm::vec3 xFacing = positionMatrix[0];
	glm::vec3 yFacing = positionMatrix[1];
	glm::vec3 zFacing = positionMatrix[2];

	xFacing = glm::normalize(xFacing) * m_config.emmitter_offset.x;
	yFacing = glm::normalize(yFacing) * m_config.emmitter_offset.y;
	zFacing = glm::normalize(zFacing) * m_config.emmitter_offset.z;

	(positionMatrix)[3][0] += xFacing.x + yFacing.x + zFacing.x;
	(positionMatrix)[3][1] += xFacing.y + yFacing.y + zFacing.y;
	(positionMatrix)[3][2] += xFacing.z + yFacing.z + zFacing.z;

	m_config.buffer_config.memory.emitter = (positionMatrix)[3];
	m_config.buffer_config.memory.totalTime += frame_time;
	m_config.buffer_config.memory.updateTime = frame_time;

	m_config_buffer->SetData(BufferSlot::Primary);

	m_compute_program->Run();

	engine->GetRendererMutex().unlock();
	m_particle_lock.unlock();
}

void ComponentEngine::ParticleSystem::Display()
{
	m_particle_lock.lock();

	if (ImGui::Checkbox("##PE_isVisible", &m_visible))
	{
		m_model->ShouldRender(m_visible);
	}
	ImGui::SameLine();
	ImGui::Text("Visible");

	ImGui::Checkbox("##PE_isRunning", &m_running);
	ImGui::SameLine();
	ImGui::Text("Running");


	ImGui::PushItemWidth(ImGui::GetWindowContentRegionWidth() - 10);
	if (ImGui::CollapsingHeader("Color"))
	{
		
		{
			if (ImGui::Checkbox("Use Color Range", &m_config.m_useColorRange))
			{
				if (!m_config.m_useColorRange)
				{
					m_config.buffer_config.memory.endColor = m_config.buffer_config.memory.startColor;
				}
			}
		}
		if (m_config.m_useColorRange)
		{
			{ // Start color
				ImGui::PushID(0);
				ImGui::Text("Start Color");
				glm::vec4 change = m_config.buffer_config.memory.startColor;
				if (ImGui::ColorEdit4("##startColor", (float*)&change))
				{
					m_config.buffer_config.memory.startColor = change;
					RebuildConfig();
				}
				ImGui::PopID();
			}
			{ // End color
				ImGui::PushID(0);
				ImGui::Text("End Color");
				glm::vec4 change = m_config.buffer_config.memory.endColor;
				if (ImGui::ColorEdit4("##endColor", (float*)&change))
				{
					m_config.buffer_config.memory.endColor = change;
					RebuildConfig();
				}

				ImGui::PopID();
			}
		}
		else
		{
			{ // Solid Color
				ImGui::PushID(0);
				ImGui::Text("Color");
				glm::vec4 change = m_config.buffer_config.memory.endColor;
				if (ImGui::ColorEdit4("##solidColor", (float*)&change))
				{
					m_config.buffer_config.memory.startColor = change;
					m_config.buffer_config.memory.endColor = change;
					RebuildConfig();
				}

				ImGui::PopID();
			}
		}


	} // End of color config

	if (ImGui::CollapsingHeader("Particle"))
	{

		{ // Emitter Offset
			ImGui::PushID(0);
			ImGui::Text("Emitter Offset");
			glm::vec3 change = m_config.emmitter_offset;
			if (ImGui::InputFloat3("##emitterOffset", (float*)&change), "%.3f", ImGuiInputTextFlags_EnterReturnsTrue)
			{
				m_config.emmitter_offset = change;
				RebuildConfig();
			}

			ImGui::PopID();
		}
		 
		{
			if (ImGui::Checkbox("Dynamic Particle Count", &m_config.m_dynamicParticleCount))
			{
				// if we enabled a dynamic partical count, we now need to set the particle count
				if (m_config.m_dynamicParticleCount)
				{
					m_particleCount = m_config.buffer_config.memory.maxLife / m_config.buffer_config.memory.emissionRate;
					RebuildAll();
				}
			}
		}



		if (!m_config.m_dynamicParticleCount)
		{
			{ // Particle Count
				ImGui::PushID(0);
				ImGui::Text("Particle Count");
				int change = m_particleCount;
				if (ImGui::InputInt("##particleCount", (int*)&change, 0.0f, 0.0f, ImGuiInputTextFlags_EnterReturnsTrue))
				{
					if (change < 0)
					{
						change = 1;
					}
					m_particleCount = change;
					RebuildAll();
				}
				ImGui::PopID();
			}
		}



		{ // Lifespan
			ImGui::PushID(0);
			ImGui::Text("Lifespan");
			float change = m_config.buffer_config.memory.maxLife;
			if (ImGui::InputFloat("##lifespan", (float*)&change, 0.0f, 0.0f, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue))
			{
				if (change < 0.1f)
				{
					change = 0.1f;
				}
				m_config.buffer_config.memory.maxLife = change;
				if (m_config.m_dynamicParticleCount)
				{
					m_particleCount = m_config.buffer_config.memory.maxLife / m_config.buffer_config.memory.emissionRate;
				}
				RebuildAll();
			}
			ImGui::PopID();
		}

		{ // Emission Rate
			ImGui::PushID(0);
			ImGui::Text("Emission Rate");
			float change = m_config.buffer_config.memory.emissionRate;
			if (ImGui::InputFloat("##emissionRate", (float*)&change, 0.0f, 0.0f, "%.5f", ImGuiInputTextFlags_EnterReturnsTrue))
			{
				if (change < 0.0f)
				{
					change = 0.00001f;
				}
				m_config.buffer_config.memory.emissionRate = change;
				if (m_config.m_dynamicParticleCount)
				{
					m_particleCount = m_config.buffer_config.memory.maxLife / m_config.buffer_config.memory.emissionRate;
				}
				RebuildAll();
			}
			ImGui::PopID();
		}


		{ // Scale
			ImGui::PushID(0);
			ImGui::Text("Scale");
			float change = m_config.buffer_config.memory.scale;
			if (ImGui::InputFloat("##scale", (float*)&change, 0.0f, 0.0f, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue))
			{
				m_config.buffer_config.memory.scale = change;
				RebuildConfig();
			}
			ImGui::PopID();
		}

	}// End of Particle dropdown



	if (ImGui::CollapsingHeader("Velocity and Drag"))
	{



		{ //  Velocity
			ImGui::PushID(0);
			ImGui::Text("Directional Velocity");
			// Temporaty storage for the velocity
			float change = m_config.directionalVelocity;

			if (ImGui::InputFloat("##Velocity", (float*)&change, 0.0f, 0.0f, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue))
			{
				m_config.directionalVelocity = change;
				RebuildAll();
			}

			ImGui::PopID();
		}// End of  Velocity


		{ // X Velocity
			ImGui::PushID(0);
			ImGui::Text("X Velocity");
			// Temporaty storage for the velocity
			glm::vec2 change = m_config.xVelocity;
			
			// Check to see if the checkbox has been changed
			if (ImGui::Checkbox("Static Velocity##XVelocity", &m_config.xVelocityStatic))
			{
				// If we are in fixed velocity, update both and submit
				if (m_config.xVelocityStatic)
				{
					m_config.xVelocity.x = m_config.xVelocity.x;
					m_config.xVelocity.y = m_config.xVelocity.x;
					RebuildAll();
				}
			}
			// If in static velocity render single input box
			if (m_config.xVelocityStatic)
			{
				// Render single float input box for x velocity
				if (ImGui::InputFloat("##xVelocity", (float*)&change, 0.0f, 0.0f, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue))
				{
					m_config.xVelocity.x = change.x;
					m_config.xVelocity.y = change.x;
					RebuildAll();
				}
			}
			else
			{
				// Render double float input box for x velocity that will generate a random number between the two
				if (ImGui::InputFloat2("##xVelocityRange", (float*)&change, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue))
				{
					m_config.xVelocity = change;
					RebuildAll();
				}
			}
			ImGui::PopID();
		}// End of X Velocity

		{ // Y Velocity
			ImGui::PushID(0);
			ImGui::Text("Y Velocity");
			// Temporaty storage for the velocity
			glm::vec2 change = m_config.yVelocity;

			// Check to see if the checkbox has been changed
			if (ImGui::Checkbox("Static Velocity##YVelocity", &m_config.yVelocityStatic))
			{
				// If we are in fixed velocity, update both and submit
				if (m_config.yVelocityStatic)
				{
					m_config.yVelocity.x = m_config.yVelocity.x;
					m_config.yVelocity.y = m_config.yVelocity.x;
					RebuildAll();
				}
			}
			// If in static velocity render single input box
			if (m_config.yVelocityStatic)
			{
				// Render single float input box for x velocity
				if (ImGui::InputFloat("##yVelocity", (float*)&change, 0.0f, 0.0f, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue))
				{
					m_config.yVelocity.x = change.x;
					m_config.yVelocity.y = change.x;
					RebuildAll();
				}
			}
			else
			{
				// Render double float input box for x velocity that will generate a random number between the two
				if (ImGui::InputFloat2("##yVelocityRange", (float*)&change, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue))
				{
					m_config.yVelocity = change;
					RebuildAll();
				}
			}
			ImGui::PopID();
		}// End of Y Velocity

		{ // Z Velocity
			ImGui::PushID(0);
			ImGui::Text("Z Velocity");
			// Temporaty storage for the velocity
			glm::vec2 change = m_config.zVelocity;

			// Check to see if the checkbox has been changed
			if (ImGui::Checkbox("Static Velocity##ZVelocity", &m_config.zVelocityStatic))
			{
				// If we are in fixed velocity, update both and submit
				if (m_config.zVelocityStatic)
				{
					m_config.zVelocity.x = m_config.zVelocity.x;
					m_config.zVelocity.y = m_config.zVelocity.x;
					RebuildAll();
				}
			}
			// If in static velocity render single input box
			if (m_config.zVelocityStatic)
			{
				// Render single float input box for x velocity
				if (ImGui::InputFloat("##zVelocity", (float*)&change, 0.0f, 0.0f, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue))
				{
					m_config.zVelocity.x = change.x;
					m_config.zVelocity.y = change.x;
					RebuildAll();
				}
			}
			else
			{
				// Render double float input box for x velocity that will generate a random number between the two
				if (ImGui::InputFloat2("##zVelocityRange", (float*)&change, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue))
				{
					m_config.zVelocity = change;
					RebuildAll();
				}
			}
			ImGui::PopID();
		}// End of Z Velocity


	} // End of Velocity and Drag dropdown

	m_particle_lock.unlock();
}

void ComponentEngine::ParticleSystem::EntityHookDefault(enteez::Entity & entity)
{
	enteez::ComponentWrapper<ParticleSystem>* wrapper = entity.AddComponent<ParticleSystem>(&entity);
	wrapper->SetName("Particle System");
	ParticleSystem& particel_system = wrapper->Get();
}

void ComponentEngine::ParticleSystem::EntityHookXML(enteez::Entity & entity, pugi::xml_node & component_data)
{
	enteez::ComponentWrapper<ParticleSystem>* wrapper = entity.AddComponent<ParticleSystem>(&entity);
	wrapper->SetName("Particle System");
	ParticleSystem& particel_system = wrapper->Get();

	particel_system.m_visible = component_data.attribute("visible").as_bool(true);
	particel_system.m_running = component_data.attribute("running").as_bool(true);
	particel_system.m_model->ShouldRender(particel_system.m_visible);

	pugi::xml_node color_node = component_data.child("Color");
	if (color_node)
	{
		pugi::xml_node start_color_node = color_node.child("StartColor");
		glm::vec4 start_color
		(
			start_color_node.attribute("R").as_float(0.0f),
			start_color_node.attribute("G").as_float(1.0f),
			start_color_node.attribute("B").as_float(1.0f),
			start_color_node.attribute("A").as_float(0.0f)
		);
		particel_system.m_config.buffer_config.memory.startColor = start_color;

		if (color_node.attribute("useColorRange").as_bool())
		{
			pugi::xml_node end_color_node = color_node.child("EndColor");
			glm::vec4 end_color
			(
				end_color_node.attribute("R").as_float(1.0f),
				end_color_node.attribute("G").as_float(0.0f),
				end_color_node.attribute("B").as_float(0.0f),
				end_color_node.attribute("A").as_float(1.0f)
			);
			particel_system.m_config.buffer_config.memory.endColor = end_color;
		}
		else
		{
			particel_system.m_config.buffer_config.memory.endColor = start_color;
		}
	}


	pugi::xml_node particle_node = component_data.child("Particle");
	if (particle_node)
	{
		particel_system.m_config.buffer_config.memory.emissionRate = particle_node.child("EmissionRate").attribute("value").as_float(0.01f);
		particel_system.m_config.buffer_config.memory.maxLife = particle_node.child("MaxLife").attribute("value").as_float(1.0f);
		if (particle_node.attribute("dynamicParticleCount").as_bool())
		{
			particel_system.m_particleCount = particel_system.m_config.buffer_config.memory.maxLife / particel_system.m_config.buffer_config.memory.emissionRate;
		}
		else
		{
			particel_system.m_particleCount = particle_node.child("ParticleCount").attribute("value").as_int(100);
		}
		particel_system.m_config.buffer_config.memory.scale = particle_node.child("Scale").attribute("value").as_float(0.1f);
		pugi::xml_node emitter_offset_node = particle_node.child("EmitterOffset");
		glm::vec3 emitter_offset
		(
			emitter_offset_node.attribute("x").as_float(),
			emitter_offset_node.attribute("y").as_float(),
			emitter_offset_node.attribute("z").as_float()
		);

		particel_system.m_config.emmitter_offset = emitter_offset;
	}


	pugi::xml_node velocity_node = component_data.child("VelocityAndDrag");
	if (velocity_node)
	{
		particel_system.m_config.directionalVelocity = velocity_node.child("DirectionalVelocity").attribute("value").as_float(1.0f);

		particel_system.m_config.xVelocityStatic = velocity_node.child("VelocityX").attribute("static").as_bool(false);
		particel_system.m_config.yVelocityStatic = velocity_node.child("VelocityY").attribute("static").as_bool(false);
		particel_system.m_config.zVelocityStatic = velocity_node.child("VelocityZ").attribute("static").as_bool(false);

		particel_system.m_config.xVelocity = glm::vec2(
			velocity_node.child("VelocityX").attribute("min").as_float(-1.0f),
			velocity_node.child("VelocityX").attribute(particel_system.m_config.xVelocityStatic ? "min" : "max").as_float(1.0f)
		);

		particel_system.m_config.yVelocity = glm::vec2(
			velocity_node.child("VelocityY").attribute("min").as_float(-1.0f),
			velocity_node.child("VelocityY").attribute(particel_system.m_config.yVelocityStatic ? "min" : "max").as_float(1.0f)
		);

		particel_system.m_config.zVelocity = glm::vec2(
			velocity_node.child("VelocityZ").attribute("min").as_float(-1.0f),
			velocity_node.child("VelocityZ").attribute(particel_system.m_config.zVelocityStatic ? "min" : "max").as_float(1.0f)
		);

	}



}

void ComponentEngine::ParticleSystem::ReciveMessage(enteez::Entity * sender, ParticleSystemVisibility & message)
{
	SetVisible(message.visible);
	SetRunning(message.visible);
	if (message.visible)
	{
		glm::mat4 positionMatrix = m_entity->GetComponent<Transformation>().Get();

		m_config.buffer_config.memory.emitter = (positionMatrix)[3];
		ResetTimer();
	}
}

void ComponentEngine::ParticleSystem::ResetTimer()
{

	ComponentEngine::Engine* engine = Engine::Singlton();
	engine->GetRendererMutex().lock();
	m_particle_lock.lock();


	m_config.buffer_config.memory.totalTime = 0;
	RebuildAll();


	m_particle_lock.unlock();
	engine->GetRendererMutex().unlock();
}

bool ComponentEngine::ParticleSystem::IsRunning()
{
	return m_running;
}

void ComponentEngine::ParticleSystem::SetRunning(bool running)
{
	m_particle_lock.lock();
	m_running = running;
	m_particle_lock.unlock();
}

bool ComponentEngine::ParticleSystem::IsVisible()
{
	return m_visible;
}

void ComponentEngine::ParticleSystem::SetVisible(bool visible)
{

	m_particle_lock.lock();

	m_visible = visible;
	m_model->ShouldRender(m_visible);

	m_particle_lock.unlock();
}

void ComponentEngine::ParticleSystem::RebuildConfig()
{
	ComponentEngine::Engine* engine = Engine::Singlton();
	engine->GetRendererMutex().lock();
	m_particle_lock.lock();

	// Pass the configuration to the GPU
	m_config_buffer->SetData(BufferSlot::Primary);

	m_particle_lock.unlock();
	engine->GetRendererMutex().unlock();
}

void ComponentEngine::ParticleSystem::RebuildAll()
{

	ComponentEngine::Engine* engine = Engine::Singlton();
	Renderer::IRenderer* renderer = engine->GetRenderer();

	engine->GetRendererMutex().lock();
	m_particle_lock.lock();


	RebuildConfig();


	if (m_vertex_buffer->GetElementCount(BufferSlot::Primary) != m_particleCount)
	{
		{

			delete m_vertex_buffer;

			m_vertex_data.clear();
			m_vertex_data.resize(m_particleCount);

			m_vertex_buffer = renderer->CreateVertexBuffer(m_vertex_data.data(), sizeof(ParticleInstance), m_vertex_data.size());


			m_partical_vertex_set->AttachBuffer(0, m_vertex_buffer);
		}
		{
			delete m_particle_payload_buffer;

			m_particle_payloads.clear();
			m_particle_payloads.resize(m_particleCount);

			//m_particle_payload_buffer->Resize(BufferSlot::Primary, m_particle_payloads.data(), m_particleCount);

			m_particle_payload_buffer = renderer->CreateUniformBuffer(m_particle_payloads.data(), BufferChain::Single, sizeof(ParticlePayload), m_particle_payloads.size(), true);


			m_partical_vertex_set->AttachBuffer(2, m_particle_payload_buffer);
		}
		m_partical_vertex_set->UpdateSet();


		m_model_pool->SetVertexBuffer(m_vertex_buffer);

		m_model_pool->SetVertexDrawCount(m_particleCount);

		m_compute_pipeline->SetX(m_particleCount);

		//m_compute_pipeline->Build();

		m_compute_program->Build();


		engine->Rebuild();
	}

	for (int i = 0; i < m_vertex_data.size(); i++)
	{
		m_vertex_data[i].position = glm::vec4(m_config.emmitter_offset + m_config.buffer_config.memory.emitter, 0.0f);
		m_vertex_data[i].color = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
	}

	m_vertex_buffer->SetData(BufferSlot::Primary);
	for (int i = 0; i < m_particle_payloads.size(); i++)
	{
		m_particle_payloads[i].memory.life = m_config.buffer_config.memory.emissionRate * i;

		float x = Common::RandomNumber(m_config.xVelocity.x, m_config.xVelocity.y);
		float y = Common::RandomNumber(m_config.yVelocity.x, m_config.yVelocity.y);
		float z = Common::RandomNumber(m_config.zVelocity.x, m_config.zVelocity.y);

		m_particle_payloads[i].memory.velocity = glm::normalize(glm::vec4(x, y, z, 0)) * m_config.directionalVelocity;
		m_particle_payloads[i].memory.origin = glm::vec4(m_config.emmitter_offset + m_config.buffer_config.memory.emitter, 0.0f);
	}

	//m_particle_payload_buffer->Resize(BufferSlot::Primary, m_particle_payloads.data(), m_particleCount);
	m_particle_payload_buffer->SetData(BufferSlot::Primary);


	m_particle_lock.unlock();
	engine->GetRendererMutex().unlock();
}
