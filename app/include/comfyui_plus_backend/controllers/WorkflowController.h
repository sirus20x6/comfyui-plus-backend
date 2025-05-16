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
    // Define your routes and the methods they map to.
    ADD_METHOD_TO(WorkflowController::getWorkflows, "/workflows", {drogon::HttpMethod::Get}, "JwtAuthFilter");
    ADD_METHOD_TO(WorkflowController::createWorkflow, "/workflows", {drogon::HttpMethod::Post}, "JwtAuthFilter");
    ADD_METHOD_TO(WorkflowController::getWorkflowById, "/workflows/{id}", {drogon::HttpMethod::Get}, "JwtAuthFilter");
    ADD_METHOD_TO(WorkflowController::updateWorkflow, "/workflows/{id}", {drogon::HttpMethod::Put}, "JwtAuthFilter");
    ADD_METHOD_TO(WorkflowController::deleteWorkflow, "/workflows/{id}", {drogon::HttpMethod::Delete}, "JwtAuthFilter");
    METHOD_LIST_END

    // Endpoint handler declarations
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