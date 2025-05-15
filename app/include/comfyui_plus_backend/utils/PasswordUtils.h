#pragma once

#include <string>
#include <vector> // For salt
#include <cstdint> // For uint32_t and other integer types

namespace comfyui_plus_backend
{
namespace app
{
namespace utils
{

class PasswordUtils
{
  public:
    // Hashes a plain text password using Argon2.
    // Returns the full encoded hash string (including salt, params, etc.).
    // Returns an empty string on failure.
    static std::string hashPassword(const std::string &plainPassword);

    // Verifies a plain text password against a stored Argon2 hash.
    // Returns true if the password matches the hash, false otherwise.
    static bool verifyPassword(const std::string &plainPassword, const std::string &hashedPassword);

  private:
    // Argon2 parameters - you can tune these.
    // Higher values are more secure but slower.
    static const uint32_t T_COST = 2;    // Iterations (time cost)
    static const uint32_t M_COST = (1 << 16); // Memory cost in KiB (65536 KiB = 64 MiB)
    static const uint32_t PARALLELISM = 1; // Number of parallel threads
    static const uint32_t SALT_LENGTH = 16; // Bytes
    static const uint32_t HASH_LENGTH = 32; // Bytes for the raw hash output
};

} // namespace utils
} // namespace app
} // namespace comfyui_plus_backend