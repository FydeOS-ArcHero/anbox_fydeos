/*
 * Copyright (C) 2016 Simon Fels <morphis@gravedo.de>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#define LOG_TAG "Anboxd"

#include "android/service/message_processor.h"
#include "android/service/android_api_skeleton.h"

#include "anbox/rpc/template_message_processor.h"

#include "anbox_rpc.pb.h"
#include "anbox_bridge.pb.h"
#include "anbox_chrome.pb.h"

namespace anbox {
MessageProcessor::MessageProcessor(const std::shared_ptr<network::MessageSender> &sender,
                                   const std::shared_ptr<rpc::PendingCallCache> &pending_calls,
                                   const std::shared_ptr<AndroidApiSkeleton> &platform_api,
                                   const std::shared_ptr<rpc::Channel> &chrome_rpc_channel) :
    rpc::MessageProcessor(sender, pending_calls),
    platform_api_(platform_api),
    chrome_rpc_channel_(chrome_rpc_channel) {
}

MessageProcessor::~MessageProcessor() {
}

void MessageProcessor::dispatch(rpc::Invocation const& invocation) {
  ALOGI("== MessageProcessor::dispatch %s", invocation.method_name().c_str());

  if (invocation.method_name() == "launch_application")
    invoke(this, platform_api_.get(), &AndroidApiSkeleton::launch_application, invocation);
  else if (invocation.method_name() == "set_focused_task")
    invoke(this, platform_api_.get(), &AndroidApiSkeleton::set_focused_task, invocation);
  else if (invocation.method_name() == "remove_task")
    invoke(this, platform_api_.get(), &AndroidApiSkeleton::remove_task, invocation);
  else if (invocation.method_name() == "resize_task")
    invoke(this, platform_api_.get(), &AndroidApiSkeleton::resize_task, invocation);
  //////////////////////////////////////////////////////////////////////////////////  
  else if (invocation.method_name() == "app_created")      
    invoke(this, this, &MessageProcessor::app_created, invocation);    
  else if (invocation.method_name() == "app_removed")
    invoke(this, this, &MessageProcessor::app_removed, invocation);    
  else if (invocation.method_name() == "task_created")      
    invoke(this, this, &MessageProcessor::task_created, invocation);    
  else if (invocation.method_name() == "task_removed")
    invoke(this, this, &MessageProcessor::task_removed, invocation);      
}

void MessageProcessor::app_created(anbox::protobuf::bridge::Application const *request,
                                     anbox::protobuf::rpc::Void *response,
                                     google::protobuf::Closure *done) {
  ALOGI("== MessageProcessor::app_created");
  chrome_rpc_channel_->call_method("app_created", request, nullptr, nullptr);

  (void)response;
  (void)done;
}

void MessageProcessor::app_removed(anbox::protobuf::bridge::Application const *request,
                                     anbox::protobuf::rpc::Void *response,
                                     google::protobuf::Closure *done) {
  ALOGI("== MessageProcessor::app_removed");                                     
  chrome_rpc_channel_->call_method("app_removed", request, nullptr, nullptr);                                     

  (void)response;
  (void)done;
}

void MessageProcessor::task_created(anbox::protobuf::chrome::CreatedTask const *request,
                                     anbox::protobuf::rpc::Void *response,
                                     google::protobuf::Closure *done) {  
  chrome_rpc_channel_->call_method("task_created", request, nullptr, nullptr);

  (void)response;
  (void)done;
}

void MessageProcessor::task_removed(anbox::protobuf::chrome::RemovedTask const *request,
                                     anbox::protobuf::rpc::Void *response,
                                     google::protobuf::Closure *done) {  
  chrome_rpc_channel_->call_method("task_removed", request, nullptr, nullptr);

  (void)response;
  (void)done;
}

void MessageProcessor::process_event_sequence(const std::string&) {
}
} // namespace network
