
#include "../../include/quickgl/Engine.hpp"
#include "../../include/quickgl/MeshManager.hpp"
#include "../../include/quickgl/pipelines/PipelineStatic.hpp"
#include "../../include/quickgl/cameras/FreeFlyCamera.hpp"

#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/vector_relational.hpp>
#include <glm/gtc/quaternion.hpp>

int main() {
	std::shared_ptr<qgl::Engine> engine
		= std::make_shared<qgl::Engine>();
	engine->InitGL("Simple conceptual example");
	
	// create pipeline
	std::shared_ptr<qgl::PipelineStatic> pipelineStatic
		= std::make_shared<qgl::PipelineStatic>();
	engine->AddPipeline(pipelineStatic);
	
	// load models
	auto meshManagerStatic = pipelineStatic->GetMeshManager();
	meshManagerStatic->LoadModels("../../terrain.fbx");
	meshManagerStatic->LoadModels("../../chest.fbx");
	
	// create and init camera
	std::shared_ptr<qgl::FreeFlyCamera> camera
		= std::make_shared<qgl::FreeFlyCamera>();
	engine->SetMainCamera(camera);
	
	// add terrain object
	uint32_t terrainId = pipelineStatic->CreateEntity();
	uint32_t terrainMeshId = meshManagerStatic->GetMeshIdByName("terrain");
	pipelineStatic->SetEntityMesh(terrainId, terrainMeshId);
	pipelineStatic->SetEntityPos(terrainId, {0,0,0});
	pipelineStatic->SetEntityRotation(terrainId, glm::quat(0, {0,1,0}));
	pipelineStatic->SetEntityScale(terrainId, {1,1,1});
	
	// add box object
	uint32_t chestId = pipelineStatic->CreateEntity();
	uint32_t chestMeshId = meshManagerStatic->GetMeshIdByName("chest");
	pipelineStatic->SetEntityMesh(chestId, chestMeshId);
	pipelineStatic->SetEntityPos(chestId, {0,10,0});
	pipelineStatic->SetEntityRotation(chestId, glm::quat(0, {0,1,0}));
	pipelineStatic->SetEntityScale(chestId, {1,1,1});
	
	while(!engine->IsQuitRequested()) {
		// process inputs
		engine->BeginNewFrame();
		camera->ProcessDefaultInput(engine);
		
		// render
		engine->Render();
	}
	
	engine->Destroy();	
	return 0;
}

