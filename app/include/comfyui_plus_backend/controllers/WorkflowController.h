// app/include/comfyui_plus_backend/controllers/WorkflowController.h
#pragma once

#include <drogon/HttpController.h>
#include <memory>

namespace comfyui_plus_backend
{
namespace app
{
namespace controllers
{

class WorkflowController final : public drogon::HttpController<WorkflowController>
{
public:
    WorkflowController();

    METHOD_LIST_BEGIN
    // Define routes without explicitly mentioning any filter.
    // Our JwtAuthFilter will check paths internally.
    ADD_METHOD_TO(WorkflowController::getWorkflows, "/workflows", {drogon::HttpMethod::Get});
    ADD_METHOD_TO(WorkflowController::createWorkflow, "/workflows", {drogon::HttpMethod::Post});
    ADD_METHOD_TO(WorkflowController::getWorkflowById, "/workflows/{id}", {drogon::HttpMethod::Get});
    ADD_METHOD_TO(WorkflowController::updateWorkflow, "/workflows/{id}", {drogon::HttpMethod::Put});
    ADD_METHOD_TO(WorkflowController::deleteWorkflow, "/workflows/{id}", {drogon::HttpMethod::Delete});
    METHOD_LIST_END

    // Endpoint handler declarations (no changes)
    void getWorkflows(const drogon::HttpRequestPtr &req,
                     std::function<void(const drogon::HttpResponsePtr &)> &&callback);
                     
    void createWorkflow(const drogon::HttpRequestPtr &req,
                       std::function<void(const drogon::HttpResponsePtr &)> &&callback);
                       
    void getWorkflowById(const drogon::HttpRequestPtr &req,
                        std::function<void(const drogon::HttpResponsePtr &)> &&callback);
                        
    void updateWorkflow(const drogon::HttpRequestPtr &req,
                       std::function<void(const drogon::HttpResponsePtr &)> &&callback);
                       
    void deleteWorkflow(const drogon::HttpRequestPtr &req,
                       std::function<void(const drogon::HttpResponsePtr &)> &&callback);

private:
    // Will add WorkflowService when implemented
    // std::shared_ptr<WorkflowService> workflowService_;
};

} // namespace controllers
} // namespace app
} // namespace comfyui_plus_backend