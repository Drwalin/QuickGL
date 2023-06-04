
#include "../../OpenGLWrapper/include/openglwrapper/OpenGL.hpp"

#include "../../include/quickgl/Engine.hpp"
#include "../../include/quickgl/MeshManager.hpp"
#include "../../include/quickgl/pipelines/PipelineStatic.hpp"
#include "../../include/quickgl/pipelines/PipelineAnimated.hpp"
#include "../../include/quickgl/cameras/FreeFlyCamera.hpp"
#include "../../include/quickgl/Gui.hpp"


#include <ctime>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/vector_relational.hpp>
#include <glm/gtc/quaternion.hpp>

#include <GL/glext.h>

#define PRINT_PARAMETER(X) {int v=0; glGetIntegerv(X, &v); printf(" %s = %i\n", #X, v); fflush(stdout);}


#include <chrono>
#include <cstdio>

int main() {
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
	std::shared_ptr<qgl::PipelineAnimated> pipelineAnimated
		= std::make_shared<qgl::PipelineAnimated>();
	engine->AddPipeline(pipelineAnimated);
	
	// create static pipeline
	std::shared_ptr<qgl::PipelineStatic> pipelineStatic
		= std::make_shared<qgl::PipelineStatic>();
	engine->AddPipeline(pipelineStatic);
	
	// load animated models
	auto meshManagerAnimated = pipelineAnimated->GetMeshManager();
	meshManagerAnimated->LoadModels("../OpenGLWrapper/samples/WobblyThing.fbx");
	
	// add animated object
	if(1){
		uint32_t entity = pipelineAnimated->CreateEntity();
		pipelineAnimated->SetEntityMesh(entity, 0);
		pipelineAnimated->SetEntityTransformsQuat(entity, glm::vec3{0,0,-1});
		pipelineAnimated->SetAnimationState(entity, 0, 0, true, 0, true);
	}
	
	// load models
	auto meshManagerStatic = pipelineStatic->GetMeshManager();
	meshManagerStatic->LoadModels("../samples/terrain.fbx");
	meshManagerStatic->LoadModels("../samples/chest.fbx");
	meshManagerStatic->LoadModels("../samples/temple.fbx");
	
	// create and init camera
	std::shared_ptr<qgl::FreeFlyCamera> camera
		= std::make_shared<qgl::FreeFlyCamera>();
	engine->SetMainCamera(camera);
	camera->SetFov(75);
	
	// add terrain object
	if(1){
	uint32_t terrainId = pipelineStatic->CreateEntity();
	pipelineStatic->SetEntityMeshByName(terrainId, "Grid");
	pipelineStatic->SetEntityTransformsQuat(terrainId, glm::vec3{0,1,0});
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
	
	while(!engine->IsQuitRequested()) {
		// process inputs
		
		if(engine->GetInputManager().WasKeyPressed(GLFW_KEY_F11)) {
			mouseLocked = !mouseLocked;
			engine->SetFullscreen(fullscreen);
		}
		
		if(engine->GetInputManager().WasKeyPressed(GLFW_KEY_ENTER)) {
			mouseLocked = !mouseLocked;
			if(mouseLocked)
				engine->GetInputManager().LockMouse();
			else
				engine->GetInputManager().UnlockMouse();
		}
		
		if(engine->GetInputManager().IsKeyDown(GLFW_KEY_ESCAPE)) {
			break;
		}
		
		if(engine->GetInputManager().IsKeyDown(GLFW_KEY_T)) {
			for(int i=0; i<500; ++i) {
				uint32_t standId = pipelineStatic->CreateEntity();
				pipelineStatic->SetEntityMesh(standId, fireStandIdMesh);
				pipelineStatic->SetEntityTransformsQuat(standId, glm::vec3{4*((I%400)-200),4*((I/400)-200),0});
				++I;
			}
		}
		
		if(engine->GetInputManager().WasKeyPressed(GLFW_KEY_0)) {
			for(int i=0; i<1000*1000; ++i) {
				uint32_t standId = pipelineStatic->CreateEntity();
				pipelineStatic->SetEntityMesh(standId, fireStandIdMesh);
				pipelineStatic->SetEntityTransformsQuat(standId, glm::vec3{4*((I%400)-200),4*((I/400)-200),0});
				++I;
			}
		}
		
		if(engine->GetInputManager().WasKeyPressed(GLFW_KEY_9)) {
			for(int i=0; i<1000*100; ++i) {
				uint32_t standId = pipelineStatic->CreateEntity();
				pipelineStatic->SetEntityMesh(standId, fireStandIdMesh);
				pipelineStatic->SetEntityTransformsQuat(standId, glm::vec3{4*((I%400)-200),4*((I/400)-200),0});
				++I;
			}
		}
		
		
		
		// begin new frame
		camera->SetRenderTargetDimensions(gl::openGL.width, gl::openGL.height);
		engine->BeginNewFrame();
		if(mouseLocked)
			camera->ProcessDefaultInput(engine);
		
		// render
		auto s = std::chrono::steady_clock::now();
		engine->Render();
		// optionally sync CPU with all GPU draw calls
		gl::Finish();
		auto e1 = std::chrono::steady_clock::now();
		uint64_t renderTime = (e1-s).count();
			

		// render gui
		ImGui::SetNextWindowBgAlpha(0.5);
			ImGui::Begin("Frames per second", NULL,
					ImGuiWindowFlags_NoTitleBar |
	// 				ImGuiWindowFlags_NoBackground |
					ImGuiWindowFlags_NoFocusOnAppearing |
					ImGuiWindowFlags_AlwaysAutoResize);
			ImGui::Text("fps: %f", 1.0f/engine->GetInputManager().GetDeltaTime());
			ImGui::Text("Entities count: %i", pipelineStatic->GetEntitiesCount());
			ImGui::Text("Rendering entities: %i", pipelineStatic->GetEntitiesToRender());
			glm::vec3 p = camera->GetPosition();
			ImGui::Text("Position: %f %f %f", p.x, p.y, p.z);
		ImGui::End();
		
		ImGui::SetNextWindowBgAlpha(0.0);
		ImGui::Begin("Timings", NULL,
				ImGuiWindowFlags_NoTitleBar |
// 				ImGuiWindowFlags_NoBackground |
				ImGuiWindowFlags_NoFocusOnAppearing |
				ImGuiWindowFlags_AlwaysAutoResize);
			for(auto t : engine->GetTimings()) {
				ImGui::Text("Stage %u of %u: %6.lu.%3.3lu \t  %s",
						t.stage, t.pipeline,
						t.seconds_queue /1000, t.seconds_queue %1000,
						t.name.c_str());
			}
			ImGui::Text("Full render time: %6.lu.%6.6lu ms",
					renderTime/1000000, renderTime%1000000);
		ImGui::End();
		
		
		// swap buffers
		
		engine->SwapBuffers();
		engine->PrintErrors();
		
		// optionally sync CPU with all gui GPU draw calls and buffer swap
		gl::Finish();
	}
	
	engine->Destroy();	
	return 0;
}

