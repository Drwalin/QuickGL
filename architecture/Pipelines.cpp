
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

enum StageTypeFlags : uint32_t {
	/*
	 * Waits for aother materials/pipelines to finish all stage before stageId
	 * for current camera.
	 */
	STAGE_SYNC_AFTER_OTHER_MATERIALS_CURRENT_CAMERA = 1,
	STAGE_REQUIRE_BOUND_FBO = 2,
};

enum StageOrder : uint32_t {
	STAGE_GLOBAL = 0,
	STAGE_CAMERA = 4,
	
	STAGE_1_RENDER_PASS_1 = 8 | STAGE_REQUIRE_BOUND_FBO,
	STAGE_2_OCCLUSION_PASS_1 = 8 | STAGE_REQUIRE_BOUND_FBO | STAGE_SYNC_AFTER_OTHER_MATERIALS_CURRENT_CAMERA,
	STAGE_3_RENDER_PASS_2 = 12 | STAGE_REQUIRE_BOUND_FBO,
	STAGE_4_OCCLUSION_PASS_2 = 12 | STAGE_REQUIRE_BOUND_FBO | STAGE_SYNC_AFTER_OTHER_MATERIALS_CURRENT_CAMERA,
	STAGE_5_RENDER_PASS_3 = 16 | STAGE_REQUIRE_BOUND_FBO,
	
	STAGE_RENDER_PASS_WATER = 64 | STAGE_SYNC_AFTER_OTHER_MATERIALS_CURRENT_CAMERA,
	
	STAGE_RENDER_PASS_TRANSLUCENT = 128 | STAGE_SYNC_AFTER_OTHER_MATERIALS_CURRENT_CAMERA,
	
	STAGE_POST_PROCESS = 256 | STAGE_SYNC_AFTER_OTHER_MATERIALS_CURRENT_CAMERA,
};

class PipelineStage {
public:
	std::function<void()> executeFunction;
	std::function<bool()> canExecute;
	std::shared_ptr<class Pipeline> pipeline;
	StageOrder executionPolicy;
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







