
#include "TerrainGenerator.hpp"

#include "../../OpenGLWrapper/include/openglwrapper/OpenGL.hpp"
#include "../../OpenGLWrapper/include/openglwrapper/VBO.hpp"
#include "../../OpenGLWrapper/include/openglwrapper/Texture.hpp"
#include "../../OpenGLWrapper/include/openglwrapper/Sync.hpp"

#include "../../include/quickgl/Engine.hpp"
#include "../../include/quickgl/MeshManager.hpp"
#include "../../include/quickgl/pipelines/PipelineStatic.hpp"
#include "../../include/quickgl/pipelines/PipelineBoneAnimated.hpp"
#include "../../include/quickgl/pipelines/PipelinePostProcessing.hpp"
#include "../../include/quickgl/cameras/FreeFlyCamera.hpp"
#include "../../include/quickgl/Gui.hpp"

#include "../../include/quickgl/util/Log.hpp"
#include "quickgl/cameras/CameraBasicMaterials.hpp"
#include "quickgl/util/EntityBufferManager.hpp"

#include <ctime>
#include <cstdio>

#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/vector_relational.hpp>
#include <glm/gtc/quaternion.hpp>
#include <GL/glext.h>

#define PRINT_PARAMETER(X) {int v=0; glGetIntegerv(X, &v); printf(" %s = %i\n", #X, v); fflush(stdout);}

#include <thread>
#include <chrono>
#include <random>
#include <algorithm>

int main() {
	srand(time(NULL));
	std::shared_ptr<qgl::Engine> engine
		= std::make_shared<qgl::Engine>();
	engine->InitGL("Simple conceptual example");
	
// 	PRINT_PARAMETER(GL_MAX_ELEMENTS_INDICES);
// 	PRINT_PARAMETER(GL_MAX_ELEMENTS_VERTICES);
// 	PRINT_PARAMETER(GL_MAX_TEXTURE_SIZE);
// 	PRINT_PARAMETER(GL_MAX_UNIFORM_BLOCK_SIZE);
// 	PRINT_PARAMETER(GL_MAX_VERTEX_ATTRIBS);
// 	PRINT_PARAMETER(GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS);
// 	PRINT_PARAMETER(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS);
// 	PRINT_PARAMETER(GL_MAX_VERTEX_UNIFORM_COMPONENTS);
// 	PRINT_PARAMETER(GL_MAX_VERTEX_OUTPUT_COMPONENTS);
// 	PRINT_PARAMETER(GL_MAX_VIEWPORT_DIMS);
// 	PRINT_PARAMETER(GL_MAX_VIEWPORTS);
// 	PRINT_PARAMETER(GL_NUM_COMPRESSED_TEXTURE_FORMATS);
// 	PRINT_PARAMETER(GL_SAMPLE_BUFFERS);
// 	PRINT_PARAMETER(GL_SHADER_STORAGE_BUFFER_SIZE);
// 	PRINT_PARAMETER(GL_MAX_ELEMENT_INDEX);
// 	PRINT_PARAMETER(GL_SAMPLE_BUFFERS);
// 	PRINT_PARAMETER(GL_MAX_ARRAY_TEXTURE_LAYERS);
	
		
	// create animated pipeline
	std::shared_ptr<qgl::PipelineBoneAnimated> pipelineAnimated
		= std::make_shared<qgl::PipelineBoneAnimated>(engine);
	engine->AddPipeline(pipelineAnimated);
	
	// load animated models
	auto meshManagerAnimated = pipelineAnimated->GetMeshManager();
	meshManagerAnimated->LoadModels("../samples/WobblyThing1.fbx");
	meshManagerAnimated->LoadModels("../samples/WobblyThing2.fbx");
	meshManagerAnimated->LoadModels("../samples/WobblyThingAnimations3.fbx");
	
	// add animated object
	if(1){
		uint32_t entity = pipelineAnimated->CreateEntity();
		pipelineAnimated->SetEntityMesh(entity, 0);
		pipelineAnimated->SetEntityTransformsQuat(entity, glm::vec3{0,0,-1});
		pipelineAnimated->SetAnimationState(entity, 0, rand()/30.0f, true, 0, true);
		printf("entity = %i\n", entity);
	}
	if(1){
		uint32_t entity = pipelineAnimated->CreateEntity();
		pipelineAnimated->SetEntityMesh(entity, 1);
		pipelineAnimated->SetEntityTransformsQuat(entity, glm::vec3{0,0,-10});
		pipelineAnimated->SetAnimationState(entity, 0, rand()/30.0f, true, 0, true);
		printf("entity = %i\n", entity);
	}
	
	
	// create static pipeline
	std::shared_ptr<qgl::PipelineStatic> pipelineStatic
		= std::make_shared<qgl::PipelineStatic>(engine);
	engine->AddPipeline(pipelineStatic);
	
	// load models
	auto meshManagerStatic = pipelineStatic->GetMeshManager();
	meshManagerStatic->LoadModels("../samples/terrain.fbx");
	meshManagerStatic->LoadModels("../samples/chest.fbx");
	meshManagerStatic->LoadModels("../samples/temple.fbx");
	
	// add terrain object
	if(1){
	uint32_t terrainId = pipelineStatic->CreateEntity();
	pipelineStatic->SetEntityMeshByName(terrainId, "Grid");
	pipelineStatic->SetEntityTransformsQuat(terrainId, glm::vec3{0,-30,0});
	}
	
	// add box object
	if(1){
	uint32_t chestId = pipelineStatic->CreateEntity();
	pipelineStatic->SetEntityMeshByName(chestId, "temple");
	pipelineStatic->SetEntityTransformsQuat(chestId, glm::vec3{0,-10,0});
	}
	
	// add fire stand object
	{
	uint32_t standId = pipelineStatic->CreateEntity();
	pipelineStatic->SetEntityMeshByName(standId, "fireStand");
	pipelineStatic->SetEntityTransformsQuat(standId, glm::vec3{-20,0,0});
	}
	
	// add 2. fire stand object
	{
	uint32_t standId = pipelineStatic->CreateEntity();
	pipelineStatic->SetEntityMeshByName(standId, "fireStand");
	pipelineStatic->SetEntityTransformsQuat(standId, glm::vec3{0,20,0});
	}
	
	int I=0;
	const uint32_t fireStandIdMesh = pipelineStatic->GetMeshManager()->GetMeshIdByName("fireStand");
	
	
	bool mouseLocked = true, fullscreen = false;
	engine->GetInputManager().LockMouse();
	engine->SetFullscreen(fullscreen);
	
	
	// create and init camera
	std::shared_ptr<qgl::FreeFlyCamera> camera
		= std::make_shared<qgl::FreeFlyCamera>();
	engine->SetMainCamera(camera);
	camera->SetFov(75);
	camera->SetPosition({0,-2,100});
	
	
	std::shared_ptr<qgl::FreeFlyCamera> camera2
		= std::make_shared<qgl::FreeFlyCamera>();
	camera2->SetRenderTargetDimensions(256, 256);
	engine->AddCamera(camera2);
	camera2->SetFov(75);
	camera2->SetPosition({0,-2,100});
	
	
	std::vector<uint32_t> entSta, entAni;
	
	TerrainGenerator tg(engine, pipelineStatic, pipelineAnimated);
	
	{
		int size = 96; // 1024;
		tg.Generate(-size, size);
	}
	
	uint32_t II = 0;
	auto AddRandomEntity = [&]() {
		if(rand()%2) {
			uint32_t standId = pipelineStatic->CreateEntity();
			pipelineStatic->SetEntityMesh(standId, fireStandIdMesh);
			pipelineStatic->SetEntityTransformsQuat(standId, glm::vec3{4*((I%400)-200),4*((I/400)-200),II*4});
			entSta.emplace_back(standId);
		} else {
			uint32_t entity = pipelineAnimated->CreateEntity();
			pipelineAnimated->SetEntityMesh(entity, rand()%2);
			pipelineAnimated->SetEntityTransformsQuat(entity, glm::vec3{4*((I%400)-200),4*((I/400)-200),II*4});
			pipelineAnimated->SetAnimationState(entity, rand()%4, rand()/300.0f, true, rand()%4, true);
			entAni.emplace_back(entity);
		}
		++I;
	};
	
	auto DeleteRandomEntity = [&]() {
		if(rand()%2) {
			if(entSta.size() > 10) {
				uint32_t p = rand() % (entSta.size()-4) + 4;
				pipelineStatic->DeleteEntity(entSta[p]);
				std::swap(entSta[p], entSta.back());
				entSta.erase(entSta.begin() + entSta.size()-1);
			}
		} else {
			if(entAni.size() > 10) {
				uint32_t p = rand() % (entAni.size()-4) + 4;
				pipelineAnimated->DeleteEntity(entAni[p]);
				std::swap(entAni[p], entAni.back());
				entAni.erase(entAni.begin() + entAni.size()-1);
			}
		}
	};
	
	engine->EnableProfiling(false);
	
	bool stressTestigEntityCreateAndDelete = false;
	bool stressDeleting;
	
	uint32_t BSIZE = 1;
	gl::VBO memb(1024*16, gl::SHADER_STORAGE_BUFFER, gl::DYNAMIC_DRAW);
	memb.Init();
	
	bool useMainCameraMovement = true;
	
	while(!engine->IsQuitRequested()) {
		qgl::Log::sync = false;
		qgl::Log::EmptyLine(10);
		QUICKGL_LOG("start new frame");
#ifdef QUICKGL_ENABLE_LOG
		auto frame_start_timepoint = std::chrono::steady_clock::now();
#endif
		
		bool pressedSomething = false;
		
		// process inputs
		if(engine->GetInputManager().WasKeyPressed(GLFW_KEY_F11)) {
			mouseLocked = !mouseLocked;
			engine->SetFullscreen(fullscreen);
			pressedSomething = true;
		}
		
		if(engine->GetInputManager().WasKeyPressed(GLFW_KEY_BACKSLASH)) {
			I = 0;
			II++;
		}
		
		if(engine->GetInputManager().IsKeyDown(GLFW_KEY_K)) {
			memb.Resize(BSIZE);
			BSIZE+=16;
		}
		
		if(engine->GetInputManager().WasKeyPressed(GLFW_KEY_ENTER)) {
			mouseLocked = !mouseLocked;
			if(mouseLocked)
				engine->GetInputManager().LockMouse();
			else
				engine->GetInputManager().UnlockMouse();
			pressedSomething = true;
		}
		
		if(engine->GetInputManager().IsKeyDown(GLFW_KEY_ESCAPE)) {
			break;
		}
		
		if(engine->GetInputManager().WasKeyPressed(GLFW_KEY_P)) {
			engine->EnableProfiling(!engine->GetProfiling());
		}
		
		if(engine->GetInputManager().IsKeyDown(GLFW_KEY_Y)) {
			for(int i=0; i<5; ++i)
				AddRandomEntity();
			pressedSomething = true;
		}
		if(engine->GetInputManager().IsKeyDown(GLFW_KEY_T)) {
			for(int i=0; i<500; ++i)
				AddRandomEntity();
			pressedSomething = true;
		}
		if(engine->GetInputManager().WasKeyPressed(GLFW_KEY_0)) {
			for(int i=0; i<3000*1000; ++i)
				AddRandomEntity();
			pressedSomething = true;
		}
		if(engine->GetInputManager().WasKeyPressed(GLFW_KEY_9)) {
			for(int i=0; i<1000*100; ++i)
				AddRandomEntity();
			pressedSomething = true;
		}
		if(engine->GetInputManager().WasKeyPressed(GLFW_KEY_8)) {
			for(int i=0; i<1000*10; ++i)
				AddRandomEntity();
			pressedSomething = true;
		}
		
		if(engine->GetInputManager().IsKeyDown(GLFW_KEY_DELETE)) {
			uint32_t count = 1000;
			if(engine->GetInputManager().IsKeyDown(GLFW_KEY_LEFT_SHIFT)) {
				count *= 30;
			}
			for(int i=0; i<count; ++i) {
				DeleteRandomEntity();
			}
		}
		
		if(engine->GetInputManager().WasKeyPressed(GLFW_KEY_L)) {
			stressTestigEntityCreateAndDelete = !stressTestigEntityCreateAndDelete;
		}
		
		if(stressTestigEntityCreateAndDelete) {
			if(stressDeleting) {
				for(int i=0; i<1000*10; ++i)
					DeleteRandomEntity();
				if(engine->GetEntitiesCount() < 100) {
					stressDeleting = false;
				}
			} else {
				for(int i=0; i<1000*10; ++i)
					AddRandomEntity();
				if(engine->GetEntitiesCount() > 1000000) {
					stressDeleting = true;
				}
			}
		}
		
		
		if(engine->GetInputManager().IsKeyDown(GLFW_KEY_1)) {
			for(int i=1; i<=2; ++i)
				pipelineAnimated->SetAnimationState(i, 0, 0, true, 0, true);
			pressedSomething = true;
		}
		if(engine->GetInputManager().WasKeyPressed(GLFW_KEY_2)) {
			for(int i=1; i<=2; ++i)
				pipelineAnimated->SetAnimationState(i, 1, 0, true, 1, true);
			pressedSomething = true;
		}
		if(engine->GetInputManager().WasKeyPressed(GLFW_KEY_3)) {
			for(int i=1; i<=2; ++i)
				pipelineAnimated->SetAnimationState(i, 2, 0, true, 0, true);
			pressedSomething = true;
		}
		if(engine->GetInputManager().WasKeyPressed(GLFW_KEY_4)) {
			for(int i=1; i<=2; ++i)
				pipelineAnimated->SetAnimationState(i, 2, 0, true, 2, true);
			pressedSomething = true;
		}
		if(engine->GetInputManager().WasKeyPressed(GLFW_KEY_5)) {
			for(int i=1; i<=2; ++i)
				pipelineAnimated->SetAnimationState(i, 3, 0, true, 3, false);
			pressedSomething = true;
		}
		
		
		if(engine->GetInputManager().WasKeyPressed(GLFW_KEY_M)) {
			useMainCameraMovement = !useMainCameraMovement;
		}
		
		// begin new frame
		engine->BeginNewFrame();
		if(mouseLocked) {
			if(useMainCameraMovement)
				camera->ProcessDefaultInput(engine);
			else
				camera2->ProcessDefaultInput(engine);
		}
		
		// render
		auto s = std::chrono::steady_clock::now();
		engine->Render();
		// optionally sync CPU with all GPU draw calls
		gl::Flush();
		auto e1 = std::chrono::steady_clock::now();
		uint64_t renderTime = (e1-s).count();
		
		qgl::Log::sync = true;
		QUICKGL_LOG("end frame duration: %.3f ms",
				std::chrono::duration_cast<
					std::chrono::duration<double>>(
							e1 - frame_start_timepoint).count()*1000.0f);
		if(pressedSomething)
			QUICKGL_LOG("finished frame had something pressed");

		// render gui
		ImGui::SetNextWindowBgAlpha(0.5);
			ImGui::Begin("Frames per second", NULL,
					ImGuiWindowFlags_NoTitleBar |
					ImGuiWindowFlags_NoFocusOnAppearing |
					ImGuiWindowFlags_AlwaysAutoResize);
			ImGui::Text("fps: %f", 1.0f/engine->GetInputManager().GetDeltaTime());
			ImGui::Text("Entities count: %i / %i",
					pipelineStatic->GetEntitiesToRender()
						+ pipelineAnimated->GetEntitiesToRender(),
					pipelineStatic->GetEntitiesCount()
						+ pipelineAnimated->GetEntitiesCount());
			ImGui::Text("Rendering static entities: %i / %i",
					pipelineStatic->GetEntitiesToRender(),
					pipelineStatic->GetEntitiesCount());
			ImGui::Text("Rendering animated entities: %i / %i",
					pipelineAnimated->GetEntitiesToRender(),
					pipelineAnimated->GetEntitiesCount());
			glm::vec3 p = camera->GetPosition();
			ImGui::Text("Position: %f %f %f", p.x, p.y, p.z);
			ImGui::Text("VRAM usage: %.3f + %.3f = %.3f [MiB]",
					gl::VBO::CountAllVBOMemoryUsage()/(1024*1024.f),
					gl::Texture::CountAllTextureMemoryUsage()/(1024*1024.f),
					(gl::VBO::CountAllVBOMemoryUsage() +
						gl::Texture::CountAllTextureMemoryUsage())/(1024*1024.f)
					);
			ImGui::Text("all entities added: %lu",
					qgl::EntityBufferManager::GetAllEntitiesAdded());
		ImGui::End();
		
		ImGui::SetNextWindowBgAlpha(0.0);
		ImGui::Begin("Timings", NULL,
				ImGuiWindowFlags_NoTitleBar |
				ImGuiWindowFlags_NoFocusOnAppearing |
				ImGuiWindowFlags_AlwaysAutoResize);
			for(auto t : engine->GetTimings()) {
				ImGui::Text("Stage: %10.3f us \t  %24s | %s",
						t.measuredSeconds*1000000.0,
						t.stage->pipeline->GetName().c_str(),
						t.stage->name.c_str());
			}
			ImGui::Text("Full render time: %6.lu.%6.6lu ms",
					renderTime/1000000, renderTime%1000000);
			ImGui::Text("Cpu time spent on each task separately sum: %6.6f us",
					engine->CountCpuTime()*1000000);
		ImGui::End();
		
		ImGui::Begin("Other camera");
			ImGui::Image(
					(ImTextureID)(int64_t)camera2->GetMainColorTexture()->GetTexture(),
					{256, 256}, {0,1}, {1,0});
		ImGui::End();
		
		gl::Flush();
		
		// swap buffers
		engine->SwapBuffers();
		engine->PrintErrors();
		
		// optionally sync CPU with all gui GPU draw calls and buffer swap
		if(engine->GetProfiling()) {
			gl::MemoryBarrier(gl::ALL_BARRIER_BITS);
			gl::Sync sync;
			sync.StartFence();
			while(sync.IsDone() == false) {
				gl::Flush();
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
			sync.Destroy();
		}
	}
	
	engine->Destroy();	
	return 0;
}

