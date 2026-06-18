#pragma once

#include <string>

namespace tinysql
{
    // Representa una fecha y hora mediante sus componentes numéricos.
    class DateTime
    {
    public:
        DateTime();

        DateTime(
            int year,
            int month,
            int day,
            int hour,
            int minute,
            int second
        );

        int getYear() const;
        int getMonth() const;
        int getDay() const;
        int getHour() const;
        int getMinute() const;
        int getSecond() const;

        std::string toString() const;

    private:
        int year_;
        int month_;
        int day_;
        int hour_;
        int minute_;
        int second_;
    };
}
