#pragma once

#include <string>

#include "core/ErrorCode.hpp"

namespace tinysql
{
    // Representa un error mediante un código identificable y un mensaje entendible.
    class TinySqlError
    {
    public:
        TinySqlError(ErrorCode code, std::string message);

        ErrorCode getCode() const;
        const std::string& getMessage() const;

    private:
        ErrorCode code_;
        std::string message_;
    };
}
