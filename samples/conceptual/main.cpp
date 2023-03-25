
#include "../../OpenGLWrapper/include/openglwrapper/OpenGL.hpp"

#include "../../include/quickgl/Engine.hpp"
#include "../../include/quickgl/MeshManager.hpp"
#include "../../include/quickgl/pipelines/PipelineStatic.hpp"
#include "../../include/quickgl/cameras/FreeFlyCamera.hpp"

#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/vector_relational.hpp>
#include <glm/gtc/quaternion.hpp>

#include <GL/glext.h>

#define PRINT_PARAMETER(X) {int v=0; glGetIntegerv(X, &v); printf(" %s = %i\n", #X, v); fflush(stdout);}

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
	
	// create pipeline
	GL_CHECK_PUSH_ERROR;
	std::shared_ptr<qgl::PipelineStatic> pipelineStatic
		= std::make_shared<qgl::PipelineStatic>();
	GL_CHECK_PUSH_ERROR;
	engine->AddPipeline(pipelineStatic);
	
	// load models
	auto meshManagerStatic = pipelineStatic->GetMeshManager();
	meshManagerStatic->LoadModels("../samples/terrain.fbx");
	meshManagerStatic->LoadModels("../samples/chest.fbx");
	meshManagerStatic->LoadModels("../samples/temple.fbx");
	
	// create and init camera
	std::shared_ptr<qgl::FreeFlyCamera> camera
		= std::make_shared<qgl::FreeFlyCamera>();
	engine->SetMainCamera(camera);
	
	// add terrain object
	if(1){
	uint32_t terrainId = pipelineStatic->CreateEntity();
	pipelineStatic->SetEntityMeshByName(terrainId, "Grid");
	pipelineStatic->SetEntityTransformsQuat(terrainId, glm::vec3{0,0,0});
	}
	
	// add box object
	if(1){
	uint32_t chestId = pipelineStatic->CreateEntity();
	pipelineStatic->SetEntityMeshByName(chestId, "temple");
	pipelineStatic->SetEntityTransformsQuat(chestId, glm::vec3{0,0,0});
	}
	
	// add box object
	{
	uint32_t standId = pipelineStatic->CreateEntity();
	pipelineStatic->SetEntityMeshByName(standId, "fireStand");
	pipelineStatic->SetEntityTransformsQuat(standId, glm::vec3{10,0,0});
	}
	
	while(!engine->IsQuitRequested()) {
		// process inputs
		
		if(engine->GetInputManager().IsKeyDown(GLFW_KEY_ESCAPE)) {
			break;
		}
		
		engine->BeginNewFrame();
		camera->ProcessDefaultInput(engine);
		
		// render
		engine->Render();
		engine->PrintErrors();
	}
	
	engine->Destroy();	
	return 0;
}

