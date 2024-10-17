// Generated by the gRPC C++ plugin.
// If you make any local change, they will be lost.
// source: proto/MarketAccess.proto

#include "proto/MarketAccess.pb.h"
#include "proto/MarketAccess.grpc.pb.h"

#include <functional>
#include <grpcpp/support/async_stream.h>
#include <grpcpp/support/async_unary_call.h>
#include <grpcpp/impl/channel_interface.h>
#include <grpcpp/impl/client_unary_call.h>
#include <grpcpp/support/client_callback.h>
#include <grpcpp/support/message_allocator.h>
#include <grpcpp/support/method_handler.h>
#include <grpcpp/impl/rpc_service_method.h>
#include <grpcpp/support/server_callback.h>
#include <grpcpp/impl/server_callback_handlers.h>
#include <grpcpp/server_context.h>
#include <grpcpp/impl/service_type.h>
#include <grpcpp/support/sync_stream.h>
namespace marketAccess {

static const char* Communication_method_names[] = {
  "/marketAccess.Communication/Display",
  "/marketAccess.Communication/Delete",
  "/marketAccess.Communication/Insertion",
  "/marketAccess.Communication/Update",
};

std::unique_ptr< Communication::Stub> Communication::NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options) {
  (void)options;
  std::unique_ptr< Communication::Stub> stub(new Communication::Stub(channel, options));
  return stub;
}

Communication::Stub::Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options)
  : channel_(channel), rpcmethod_Display_(Communication_method_names[0], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_Delete_(Communication_method_names[1], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_Insertion_(Communication_method_names[2], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_Update_(Communication_method_names[3], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  {}

::grpc::Status Communication::Stub::Display(::grpc::ClientContext* context, const ::marketAccess::DisplayParameters& request, ::marketAccess::OrderBookContent* response) {
  return ::grpc::internal::BlockingUnaryCall< ::marketAccess::DisplayParameters, ::marketAccess::OrderBookContent, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_Display_, context, request, response);
}

void Communication::Stub::async::Display(::grpc::ClientContext* context, const ::marketAccess::DisplayParameters* request, ::marketAccess::OrderBookContent* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::marketAccess::DisplayParameters, ::marketAccess::OrderBookContent, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_Display_, context, request, response, std::move(f));
}

void Communication::Stub::async::Display(::grpc::ClientContext* context, const ::marketAccess::DisplayParameters* request, ::marketAccess::OrderBookContent* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_Display_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::marketAccess::OrderBookContent>* Communication::Stub::PrepareAsyncDisplayRaw(::grpc::ClientContext* context, const ::marketAccess::DisplayParameters& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::marketAccess::OrderBookContent, ::marketAccess::DisplayParameters, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_Display_, context, request);
}

::grpc::ClientAsyncResponseReader< ::marketAccess::OrderBookContent>* Communication::Stub::AsyncDisplayRaw(::grpc::ClientContext* context, const ::marketAccess::DisplayParameters& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncDisplayRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status Communication::Stub::Delete(::grpc::ClientContext* context, const ::marketAccess::DeletionParameters& request, ::marketAccess::DeletionConfirmation* response) {
  return ::grpc::internal::BlockingUnaryCall< ::marketAccess::DeletionParameters, ::marketAccess::DeletionConfirmation, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_Delete_, context, request, response);
}

void Communication::Stub::async::Delete(::grpc::ClientContext* context, const ::marketAccess::DeletionParameters* request, ::marketAccess::DeletionConfirmation* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::marketAccess::DeletionParameters, ::marketAccess::DeletionConfirmation, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_Delete_, context, request, response, std::move(f));
}

void Communication::Stub::async::Delete(::grpc::ClientContext* context, const ::marketAccess::DeletionParameters* request, ::marketAccess::DeletionConfirmation* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_Delete_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::marketAccess::DeletionConfirmation>* Communication::Stub::PrepareAsyncDeleteRaw(::grpc::ClientContext* context, const ::marketAccess::DeletionParameters& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::marketAccess::DeletionConfirmation, ::marketAccess::DeletionParameters, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_Delete_, context, request);
}

::grpc::ClientAsyncResponseReader< ::marketAccess::DeletionConfirmation>* Communication::Stub::AsyncDeleteRaw(::grpc::ClientContext* context, const ::marketAccess::DeletionParameters& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncDeleteRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status Communication::Stub::Insertion(::grpc::ClientContext* context, const ::marketAccess::InsertionParameters& request, ::marketAccess::InsertionConfirmation* response) {
  return ::grpc::internal::BlockingUnaryCall< ::marketAccess::InsertionParameters, ::marketAccess::InsertionConfirmation, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_Insertion_, context, request, response);
}

void Communication::Stub::async::Insertion(::grpc::ClientContext* context, const ::marketAccess::InsertionParameters* request, ::marketAccess::InsertionConfirmation* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::marketAccess::InsertionParameters, ::marketAccess::InsertionConfirmation, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_Insertion_, context, request, response, std::move(f));
}

void Communication::Stub::async::Insertion(::grpc::ClientContext* context, const ::marketAccess::InsertionParameters* request, ::marketAccess::InsertionConfirmation* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_Insertion_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::marketAccess::InsertionConfirmation>* Communication::Stub::PrepareAsyncInsertionRaw(::grpc::ClientContext* context, const ::marketAccess::InsertionParameters& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::marketAccess::InsertionConfirmation, ::marketAccess::InsertionParameters, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_Insertion_, context, request);
}

::grpc::ClientAsyncResponseReader< ::marketAccess::InsertionConfirmation>* Communication::Stub::AsyncInsertionRaw(::grpc::ClientContext* context, const ::marketAccess::InsertionParameters& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncInsertionRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status Communication::Stub::Update(::grpc::ClientContext* context, const ::marketAccess::UpdateParameters& request, ::marketAccess::UpdateConfirmation* response) {
  return ::grpc::internal::BlockingUnaryCall< ::marketAccess::UpdateParameters, ::marketAccess::UpdateConfirmation, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_Update_, context, request, response);
}

void Communication::Stub::async::Update(::grpc::ClientContext* context, const ::marketAccess::UpdateParameters* request, ::marketAccess::UpdateConfirmation* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::marketAccess::UpdateParameters, ::marketAccess::UpdateConfirmation, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_Update_, context, request, response, std::move(f));
}

void Communication::Stub::async::Update(::grpc::ClientContext* context, const ::marketAccess::UpdateParameters* request, ::marketAccess::UpdateConfirmation* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_Update_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::marketAccess::UpdateConfirmation>* Communication::Stub::PrepareAsyncUpdateRaw(::grpc::ClientContext* context, const ::marketAccess::UpdateParameters& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::marketAccess::UpdateConfirmation, ::marketAccess::UpdateParameters, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_Update_, context, request);
}

::grpc::ClientAsyncResponseReader< ::marketAccess::UpdateConfirmation>* Communication::Stub::AsyncUpdateRaw(::grpc::ClientContext* context, const ::marketAccess::UpdateParameters& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncUpdateRaw(context, request, cq);
  result->StartCall();
  return result;
}

Communication::Service::Service() {
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      Communication_method_names[0],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< Communication::Service, ::marketAccess::DisplayParameters, ::marketAccess::OrderBookContent, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](Communication::Service* service,
             ::grpc::ServerContext* ctx,
             const ::marketAccess::DisplayParameters* req,
             ::marketAccess::OrderBookContent* resp) {
               return service->Display(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      Communication_method_names[1],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< Communication::Service, ::marketAccess::DeletionParameters, ::marketAccess::DeletionConfirmation, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](Communication::Service* service,
             ::grpc::ServerContext* ctx,
             const ::marketAccess::DeletionParameters* req,
             ::marketAccess::DeletionConfirmation* resp) {
               return service->Delete(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      Communication_method_names[2],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< Communication::Service, ::marketAccess::InsertionParameters, ::marketAccess::InsertionConfirmation, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](Communication::Service* service,
             ::grpc::ServerContext* ctx,
             const ::marketAccess::InsertionParameters* req,
             ::marketAccess::InsertionConfirmation* resp) {
               return service->Insertion(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      Communication_method_names[3],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< Communication::Service, ::marketAccess::UpdateParameters, ::marketAccess::UpdateConfirmation, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](Communication::Service* service,
             ::grpc::ServerContext* ctx,
             const ::marketAccess::UpdateParameters* req,
             ::marketAccess::UpdateConfirmation* resp) {
               return service->Update(ctx, req, resp);
             }, this)));
}

Communication::Service::~Service() {
}

::grpc::Status Communication::Service::Display(::grpc::ServerContext* context, const ::marketAccess::DisplayParameters* request, ::marketAccess::OrderBookContent* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status Communication::Service::Delete(::grpc::ServerContext* context, const ::marketAccess::DeletionParameters* request, ::marketAccess::DeletionConfirmation* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status Communication::Service::Insertion(::grpc::ServerContext* context, const ::marketAccess::InsertionParameters* request, ::marketAccess::InsertionConfirmation* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status Communication::Service::Update(::grpc::ServerContext* context, const ::marketAccess::UpdateParameters* request, ::marketAccess::UpdateConfirmation* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}


}  // namespace marketAccess
