#include "hasher.h"
#include <openssl/evp.h>
#include <array>
#include <iomanip>
#include <sstream>
#include <span>

namespace backups {

Result<ChunkID> Hasher::hash(const ChunkData& data) {
    EVP_MD_CTX* context = EVP_MD_CTX_new();
    if (!context) {
        return Result<ChunkID>(Error{std::string("Failed to create EVP context")});
    }

    if (EVP_DigestInit_ex(context, EVP_sha256(), nullptr) != 1) {
        EVP_MD_CTX_free(context);
        return Result<ChunkID>(Error{std::string("Failed to init SHA256")});
    }

    if (EVP_DigestUpdate(context, data.data(), data.size()) != 1) {
        EVP_MD_CTX_free(context);
        return Result<ChunkID>(Error{std::string("Failed to update digest")});
    }

    std::array<unsigned char, EVP_MAX_MD_SIZE> hash{};
    unsigned int hash_len = 0;

    if (EVP_DigestFinal_ex(context, hash.data(), &hash_len) != 1) {
        EVP_MD_CTX_free(context);
        return Result<ChunkID>(Error{std::string("Failed to finalize digest")});
    }

    EVP_MD_CTX_free(context);

    return Result<ChunkID>(bytes_to_hex({hash.data(), hash_len}));
}

std::string Hasher::bytes_to_hex(std::span<const unsigned char> bytes) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (auto b : bytes) {
        oss << std::setw(2) << static_cast<int>(b);
    }
    return oss.str();
}

} // namespace backups
