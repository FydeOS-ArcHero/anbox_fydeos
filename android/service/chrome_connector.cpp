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

#include "android/service/chrome_connector.h"
#include "android/service/local_socket_connection.h"
#include "android/service/chrome_message_processor.h"
#include "android/service/android_api_skeleton.h"
#include "android/service/platform_api_stub.h"

#include "anbox/rpc/channel.h"

#include <functional>
#include <array>

namespace anbox {
ChromeConnector::ChromeConnector() :
    socket_(std::make_shared<LocalSocketConnection>("/dev/anbox_chrome")),
    pending_calls_(std::make_shared<rpc::PendingCallCache>()),
    android_api_skeleton_(std::make_shared<AndroidApiSkeleton>()),
    message_processor_(std::make_shared<ChromeMessageProcessor>(socket_, pending_calls_, android_api_skeleton_)),
    rpc_channel_(std::make_shared<rpc::Channel>(pending_calls_, socket_)),    
    running_(false) {
}

ChromeConnector::~ChromeConnector() {
}

void ChromeConnector::start() {
    if (running_)
        return;

    running_.exchange(true);
    thread_ = std::thread(std::bind(&ChromeConnector::main_loop, this));
}

void ChromeConnector::stop() {
    if (!running_.exchange(false))
        return;

    thread_.join();
}

void ChromeConnector::main_loop() {
    while (running_) {
        std::array<std::uint8_t, 8192> buffer;
        const auto bytes_read = socket_->read_all(buffer.data(), buffer.size());
        if (bytes_read == 0)
            break;

        // MessageProcessor wants an vector so give it what it wants until
        // we refactor this.
        std::vector<std::uint8_t> data;
        for (unsigned n = 0; n < bytes_read; n++)
            data.push_back(buffer[n]);

        if (!message_processor_->process_data(data))
            break;
    }
}

} // namespace anbox
