#include "comfyui_plus_backend/utils/PasswordUtils.h"
#include <argon2.h>       // Main Argon2 header
#include <stdexcept>      // For std::runtime_error
#include <vector>
#include <random>         // For cryptographically secure salt generation
#include <algorithm>      // For std::generate
#include <drogon/drogon.h> // For LOG_ERROR

namespace comfyui_plus_backend
{
namespace app
{
namespace utils
{

// Define constants from the header if they are only used here
// const uint32_t PasswordUtils::T_COST = 2;
// const uint32_t PasswordUtils::M_COST = (1 << 16);
// const uint32_t PasswordUtils::PARALLELISM = 1;
// const uint32_t PasswordUtils::SALT_LENGTH = 16;
// const uint32_t PasswordUtils::HASH_LENGTH = 32;


std::string PasswordUtils::hashPassword(const std::string &plainPassword)
{
    if (plainPassword.empty()) {
        LOG_ERROR << "Password hashing attempt with empty password.";
        return "";
    }

    std::vector<uint8_t> salt(SALT_LENGTH);
    std::vector<uint8_t> hash(HASH_LENGTH);

    // Generate a cryptographically secure random salt
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 255);
    std::generate(salt.begin(), salt.end(), [&]() { return static_cast<uint8_t>(distrib(gen)); });

    // The output buffer for the encoded hash string
    // Argon2 encoded hash format: $argon2id$v=19$m=...,t=...,p=...$salt_base64$hash_base64
    // Max length is defined by argon2_encodedlen
    size_t encoded_len = argon2_encodedlen(T_COST, M_COST, PARALLELISM, SALT_LENGTH, HASH_LENGTH, Argon2_id);
    std::vector<char> encoded_hash_cstr(encoded_len);

    int result = argon2id_hash_encoded(
        T_COST,
        M_COST,
        PARALLELISM,
        plainPassword.c_str(),
        plainPassword.length(),
        salt.data(),
        SALT_LENGTH,
        HASH_LENGTH,
        encoded_hash_cstr.data(),
        encoded_len
    );

    if (result != ARGON2_OK)
    {
        LOG_ERROR << "Argon2 hashing failed: " << argon2_error_message(result);
        return "";
    }

    return std::string(encoded_hash_cstr.data()); // The data is null-terminated by argon2id_hash_encoded
}

bool PasswordUtils::verifyPassword(const std::string &plainPassword, const std::string &hashedPassword)
{
    if (plainPassword.empty() || hashedPassword.empty())
    {
        return false;
    }

    // argon2_verify expects a C-style string for the encoded hash
    int result = argon2id_verify(
        hashedPassword.c_str(),
        plainPassword.c_str(),
        plainPassword.length()
    );

    if (result == ARGON2_OK)
    {
        return true;
    }
    else
    {
        if (result != ARGON2_VERIFY_MISMATCH) {
            // Log errors other than mismatch, as mismatch is expected for wrong passwords
            LOG_WARN << "Argon2 verification error (not a mismatch): " << argon2_error_message(result)
                     << " for input hash starting with: " << hashedPassword.substr(0, 30);
        }
        return false;
    }
}

} // namespace utils
} // namespace app
} // namespace comfyui_plus_backend