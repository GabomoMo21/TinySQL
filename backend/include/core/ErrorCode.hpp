#pragma once

namespace tinysql
{
    // Agrupa los tipos de error que pueden producir las distintas capas del sistema.
    enum class ErrorCode
    {
        None,
        InvalidSyntax,
        DatabaseNotFound,
        TableNotFound,
        ColumnNotFound,
        TypeMismatch,
        DuplicateValue,
        StorageError,
        InternalError
    };
}
