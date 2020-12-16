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

#include<fcntl.h>

#include "android/service/chrome_message_processor.h"
#include "android/service/android_api_skeleton.h"

#include "anbox/rpc/template_message_processor.h"

#include "anbox_chrome.pb.h"
#include "anbox_rpc.pb.h"
#include "anbox_bridge.pb.h"

namespace anbox {
ChromeMessageProcessor::ChromeMessageProcessor(const std::shared_ptr<network::MessageSender> &sender,
                                   const std::shared_ptr<rpc::PendingCallCache> &pending_calls,
                                   const std::shared_ptr<AndroidApiSkeleton> &platform_api) :
    rpc::MessageProcessor(sender, pending_calls),
    platform_api_(platform_api){
}

ChromeMessageProcessor::~ChromeMessageProcessor() {
}

void ChromeMessageProcessor::dispatch(rpc::Invocation const& invocation) {
  ALOGI("== ChromeMessageProcessor::dispatch %s", invocation.method_name().c_str());

  if (invocation.method_name() == "launch_application")
    invoke(this, platform_api_.get(), &AndroidApiSkeleton::launch_application, invocation);
  else if (invocation.method_name() == "set_focused_task")
    invoke(this, platform_api_.get(), &AndroidApiSkeleton::set_focused_task, invocation);
  else if (invocation.method_name() == "remove_task")
    invoke(this, platform_api_.get(), &AndroidApiSkeleton::remove_task, invocation);
  else if (invocation.method_name() == "resize_task")
    invoke(this, platform_api_.get(), &AndroidApiSkeleton::resize_task, invocation);  
  //////////////////////////////////////////////
  else if (invocation.method_name() == "install_app")  
    invoke(this, this, &ChromeMessageProcessor::install_app, invocation);
}

void ChromeMessageProcessor::install_app(anbox::protobuf::chrome::InstallApp const *request,
                                     anbox::protobuf::rpc::Void *response,
                                     google::protobuf::Closure *done) {                                       
  ALOGI("== ChromeMessageProcessor::install_app %s %d %d %lu", request->name().c_str(), request->pos(), request->size(), request->data().size());

  (void)response;
  std::string file_path = std::string("/data/local/") + request->name().c_str();  
  
  if (request->size() > 0){
    auto fd = open(file_path.c_str(), O_RDWR|O_CREAT, 0766);
    if (fd <= 0){
      ALOGI("== ChromeMessageProcessor::install_app open file failed %s", file_path.c_str());

      done->Run();
      return;
    }

    lseek(fd, request->pos(), SEEK_SET);
    write(fd, request->data().c_str(), request->data().size());

    close(fd);

    done->Run();
  }else{
    platform_api_->install_app(file_path, response, done);    
  }  
}

void ChromeMessageProcessor::process_event_sequence(const std::string&) {
}
} // namespace network
