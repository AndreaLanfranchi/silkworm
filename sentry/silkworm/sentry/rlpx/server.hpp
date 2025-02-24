/*
   Copyright 2022 The Silkworm Authors

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#pragma once

#include <string>

#include <silkworm/concurrency/coroutine.hpp>

#include <boost/asio/awaitable.hpp>

#include <silkworm/rpc/server/server_context_pool.hpp>
#include <silkworm/sentry/common/ecc_key_pair.hpp>

namespace silkworm::sentry::rlpx {

class Server final {
  public:
    Server(std::string host, uint16_t port);

    boost::asio::awaitable<void> start(
        silkworm::rpc::ServerContextPool& context_pool,
        common::EccKeyPair node_key,
        std::string client_id);

  private:
    std::string host_;
    uint16_t port_;
};

}  // namespace silkworm::sentry::rlpx
