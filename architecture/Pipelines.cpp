
#include <cinttypes>

#include <functional>
#include <vector>
#include <memory>

namespace gl {
	class VBO;
}

class Camera;

class Material {
public:
	
	virtual void PrepareIndirectBuffer(std::shared_ptr<Camera> camera,
			std::shared_ptr<gl::VBO> ids, uint32_t count);
	virtual void RenderPass(std::shared_ptr<Camera> camera,
			std::shared_ptr<gl::VBO> ids, uint32_t count);
};

enum StageExecutionPolicy : uint32_t {
	EXEC_GLOBAL1 = 1,
	EXEC_GLOBAL2 = 2,
	EXEC_GLOBAL3 = 3,
	EXEC_CAMERA = 4,
	EXEC_FBO_RENDER_PASS_1 = 5,
	EXEC_FBO_RENDER_PASS_2 = 6,
	/*
	 * Execution of EXEC_OCCLUSION_PASS starts exactly after all materials
	 * finish it's EXEC_FBO_RENDER_PASS_2 stage for current camera.
	 */
	EXEC_OCCLUSION_PASS = 7,
	/*
	 * Execution of EXEC_FBO_RENDER_PASS_3 starts exactly after all materials
	 * finish it's EXEC_OCCLUSION_PASS stage for current camera.
	 */
	EXEC_FBO_RENDER_PASS_3 = 8,
	/*
	 * Execution of EXEC_FBO_RENDER_PASS_4 starts exactly after all materials
	 * finish it's EXEC_FBO_RENDER_PASS_3 stage for current camera.
	 */
	EXEC_FBO_RENDER_PASS_4 = 9,
};

const static bool stageExecutionPolicyWaitForAllMaterialsToFinishPreviousForCurrentCamera[] = {
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	true,
	true,
	true,
};

class PipelineStage {
public:
	std::function<void()> executeFunction;
	std::function<bool()> canExecute;
	std::shared_ptr<class Pipeline> pipeline;
	StageExecutionPolicy executionPolicy;
};


class PipelineStagesScheduler {
public:
	
	virtual bool CanExecuteNextStage() { return GetCurrentStage()->canExecute(); }
	virtual bool HasMoreStages() { return GetCurrentStage() != nullptr; }

	virtual void ExecuteNextStage() { GetCurrentStage()->executeFunction(); GoToNextStage(); }
	
	virtual void StartExecutionPerCamera(std::shared_ptr<Camera> camera);
	
protected:
	
	PipelineStage* GetCurrentStage();
	void GoToNextStage();
	
public:
	
	uint32_t currentGlobalStage;
	uint32_t currentPerCameraStage;
	
	std::vector<PipelineStage> globalStages;
	std::vector<PipelineStage> perCameraStages;
};


class Pipeline {
public:

	virtual void Init(); // init buffers and prepare stages

	PipelineStagesScheduler& GetStageScheduler();
	
protected:
	
	std::shared_ptr<Camera> currentCamera;
	
	PipelineStagesScheduler stagesScheduler;
	
	std::shared_ptr<Material> material;
};







