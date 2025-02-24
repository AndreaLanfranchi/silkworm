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

#include <array>
#include <cstdint>
#include <map>
#include <optional>
#include <string_view>

#include <evmc/evmc.h>
#include <intx/intx.hpp>
#include <nlohmann/json.hpp>

#include <silkworm/common/util.hpp>

namespace silkworm {

enum class SealEngineType {
    kNoProof,
    kEthash,
    kClique,
    kAuRA,
};

struct ChainConfig {
    static constexpr const char* kJsonForkNames[]{
        "homesteadBlock",  // EVMC_HOMESTEAD
        // there's no evmc_revision for daoForkBlock
        "eip150Block",          // EVMC_TANGERINE_WHISTLE
        "eip155Block",          // EVMC_SPURIOUS_DRAGON
        "byzantiumBlock",       // EVMC_BYZANTIUM
        "constantinopleBlock",  // EVMC_CONSTANTINOPLE
        "petersburgBlock",      // EVMC_PETERSBURG
        "istanbulBlock",        // EVMC_ISTANBUL
        // there's no evmc_revision for muirGlacierBlock
        "berlinBlock",  // EVMC_BERLIN
        "londonBlock",  // EVMC_LONDON
        // there's no evmc_revision for arrowGlacierBlock, nor for grayGlacierBlock
        "mergeNetsplitBlock",  // EVMC_PARIS, corresponds to FORK_NEXT_VALUE of EIP-3675
        "shanghaiBlock",       // EVMC_SHANGHAI
        "cancunBlock",         // EVMC_CANCUN
    };

    static_assert(std::size(kJsonForkNames) == EVMC_MAX_REVISION);

    //! \brief Returns the chain identifier
    //! \see https://eips.ethereum.org/EIPS/eip-155
    uint64_t chain_id{0};

    //! \brief Holds the hash of genesis block
    std::optional<evmc::bytes32> genesis_hash;

    //! \brief Returns the type of seal engine
    SealEngineType seal_engine{SealEngineType::kNoProof};

    //! \brief Block numbers of forks that have an evmc_revision value
    std::array<std::optional<uint64_t>, EVMC_MAX_REVISION> evmc_fork_blocks{};

    //! \see https://eips.ethereum.org/EIPS/eip-779
    std::optional<uint64_t> dao_block{std::nullopt};

    //! \see https://eips.ethereum.org/EIPS/eip-2387
    std::optional<uint64_t> muir_glacier_block{std::nullopt};

    //! \see https://github.com/ethereum/execution-specs/blob/master/network-upgrades/mainnet-upgrades/arrow-glacier.md
    std::optional<uint64_t> arrow_glacier_block{std::nullopt};

    //! \see https://github.com/ethereum/execution-specs/blob/master/network-upgrades/mainnet-upgrades/gray-glacier.md
    std::optional<uint64_t> gray_glacier_block{std::nullopt};

    //! \brief PoW to PoS switch
    //! \see EIP-3675
    std::optional<intx::uint256> terminal_total_difficulty{std::nullopt};
    std::optional<uint64_t> terminal_block_number{std::nullopt};
    std::optional<evmc::bytes32> terminal_block_hash{std::nullopt};

    //! \brief Returns the revision level at given block number
    //! \details In other words, on behalf of Json chain config data
    //! returns whether specific HF have occurred
    [[nodiscard]] constexpr evmc_revision revision(uint64_t block_number) const noexcept {
        for (size_t i{EVMC_MAX_REVISION}; i > 0; --i) {
            if (evmc_fork_blocks[i - 1].has_value() && block_number >= evmc_fork_blocks[i - 1].value()) {
                return static_cast<evmc_revision>(i);
            }
        }
        return EVMC_FRONTIER;
    }

    //! \details As ancillary to revision this returns at which block
    //! a specific revision has occurred. If return value is std::nullopt
    //! it means the actual chain either does not support such revision
    [[nodiscard]] std::optional<uint64_t> revision_block(evmc_revision rev) const noexcept;

    void set_revision_block(evmc_revision rev, std::optional<uint64_t> block);

    std::vector<BlockNum> distinct_fork_numbers() const;

    //! \brief Return the JSON representation of this object
    [[nodiscard]] nlohmann::json to_json() const noexcept;

    /*Sample JSON input:
    {
            "chainId":1,
            "homesteadBlock":1150000,
            "daoForkBlock":1920000,
            "eip150Block":2463000,
            "eip155Block":2675000,
            "byzantiumBlock":4370000,
            "constantinopleBlock":7280000,
            "petersburgBlock":7280000,
            "istanbulBlock":9069000,
            "muirGlacierBlock":9200000,
            "berlinBlock":12244000
    }
    */
    //! \brief Try parse a JSON object into strongly typed ChainConfig
    //! \remark Should this return std::nullopt the parsing has failed
    static std::optional<ChainConfig> from_json(const nlohmann::json& json) noexcept;

    friend bool operator==(const ChainConfig&, const ChainConfig&) = default;
};

std::ostream& operator<<(std::ostream& out, const ChainConfig& obj);

inline constexpr evmc::bytes32 kMainnetGenesisHash{0xd4e56740f876aef8c010b86a40d5f56745a118d0906a34e69aec8c0db1cb8fa3_bytes32};
inline constexpr ChainConfig kMainnetConfig{
    .chain_id = 1,
    .seal_engine = SealEngineType::kEthash,
    .evmc_fork_blocks =
        {
            1'150'000,   // Homestead
            2'463'000,   // Tangerine Whistle
            2'675'000,   // Spurious Dragon
            4'370'000,   // Byzantium
            7'280'000,   // Constantinople
            7'280'000,   // Petersburg
            9'069'000,   // Istanbul
            12'244'000,  // Berlin
            12'965'000,  // London
        },
    .dao_block = 1'920'000,
    .muir_glacier_block = 9'200'000,
    .arrow_glacier_block = 13'773'000,
    .gray_glacier_block = 15'050'000,
    .terminal_total_difficulty = intx::from_string<intx::uint256>("58750000000000000000000"),
};

inline constexpr evmc::bytes32 kRopstenGenesisHash{0x41941023680923e0fe4d74a34bdac8141f2540e3ae90623718e47d66d1ca4a2d_bytes32};
inline constexpr ChainConfig kRopstenConfig{
    .chain_id = 3,
    .seal_engine = SealEngineType::kEthash,
    .evmc_fork_blocks =
        {
            0,           // Homestead
            0,           // Tangerine Whistle
            10,          // Spurious Dragon
            1'700'000,   // Byzantium
            4'230'000,   // Constantinople
            4'939'394,   // Petersburg
            6'485'846,   // Istanbul
            9'812'189,   // Berlin
            10'499'401,  // London
        },
    .muir_glacier_block = 7'117'117,
    .terminal_total_difficulty = 50000000000000000,
};

inline constexpr evmc::bytes32 kRinkebyGenesisHash{0x6341fd3daf94b748c72ced5a5b26028f2474f5f00d824504e4fa37a75767e177_bytes32};
inline constexpr ChainConfig kRinkebyConfig{
    .chain_id = 4,
    .seal_engine = SealEngineType::kClique,
    .evmc_fork_blocks =
        {
            1,          // Homestead
            2,          // Tangerine Whistle
            3,          // Spurious Dragon
            1'035'301,  // Byzantium
            3'660'663,  // Constantinople
            4'321'234,  // Petersburg
            5'435'345,  // Istanbul
            8'290'928,  // Berlin
            8'897'988,  // London
        },
};

inline constexpr evmc::bytes32 kGoerliGenesisHash{0xbf7e331f7f7c1dd2e05159666b3bf8bc7a8a3a9eb1d518969eab529dd9b88c1a_bytes32};
inline constexpr ChainConfig kGoerliConfig{
    .chain_id = 5,
    .seal_engine = SealEngineType::kClique,
    .evmc_fork_blocks =
        {
            0,          // Homestead
            0,          // Tangerine Whistle
            0,          // Spurious Dragon
            0,          // Byzantium
            0,          // Constantinople
            0,          // Petersburg
            1'561'651,  // Istanbul
            4'460'644,  // Berlin
            5'062'605,  // London
        },
    .terminal_total_difficulty = 10790000,
};

inline constexpr evmc::bytes32 kSepoliaGenesisHash{0x25a5cc106eea7138acab33231d7160d69cb777ee0c2c553fcddf5138993e6dd9_bytes32};
inline constexpr ChainConfig kSepoliaConfig{
    .chain_id = 11155111,
    .seal_engine = SealEngineType::kEthash,
    .evmc_fork_blocks =
        {
            0,        // Homestead
            0,        // Tangerine Whistle
            0,        // Spurious Dragon
            0,        // Byzantium
            0,        // Constantinople
            0,        // Petersburg
            0,        // Istanbul
            0,        // Berlin
            0,        // London
            1735371,  // Merge Netsplit
        },
    .muir_glacier_block = 0,
    .terminal_total_difficulty = 17000000000000000,
};

//! \brief Looks up a known chain config provided its chain ID
std::optional<std::pair<const std::string, const ChainConfig*>> lookup_known_chain(uint64_t chain_id) noexcept;

//! \brief Looks up a known chain config provided its chain identifier (eg. "mainnet")
std::optional<std::pair<const std::string, const ChainConfig*>> lookup_known_chain(std::string_view identifier) noexcept;

//! \brief Returns a map known chains names mapped to their respective chain ids
std::map<std::string, uint64_t> get_known_chains_map() noexcept;

}  // namespace silkworm
