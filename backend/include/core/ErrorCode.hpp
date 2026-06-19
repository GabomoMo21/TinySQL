#pragma once

namespace tinysql
{
    // Identifica los distintos errores que pueden ocurrir dentro del sistema.
    enum class ErrorCode
    {
        None,
        InvalidSyntax,
        InvalidIdentifier,
        DatabaseAlreadyExists,
        DatabaseNotFound,
        TableNotFound,
        ColumnNotFound,
        TypeMismatch,
        DuplicateValue,
        StorageError,
        InternalError
    };
}
