#include "core/DateTime.hpp"

#include <cctype>
#include <iomanip>
#include <sstream>

namespace
{
    // Determina si un año utiliza 29 días en febrero.
    bool isLeapYear(int year)
    {
        return
            year % 400 == 0 ||
            (year % 4 == 0 && year % 100 != 0);
    }

    // Devuelve la cantidad de días permitida para un mes específico.
    int getDaysInMonth(int year, int month)
    {
        static const int daysPerMonth[] =
        {
            31,
            28,
            31,
            30,
            31,
            30,
            31,
            31,
            30,
            31,
            30,
            31
        };

        if (month == 2 && isLeapYear(year))
        {
            return 29;
        }

        return daysPerMonth[month - 1];
    }

    // Comprueba que cada posición numérica contenga realmente un dígito.
    bool hasValidCharacters(const std::string& text)
    {
        for (std::size_t index = 0; index < text.size(); ++index)
        {
            if (
                index == 4 ||
                index == 7 ||
                index == 10 ||
                index == 13 ||
                index == 16
                )
            {
                continue;
            }

            if (
                !std::isdigit(
                    static_cast<unsigned char>(text[index])
                )
                )
            {
                return false;
            }
        }

        return true;
    }
}

namespace tinysql
{
    // Inicializa todos los componentes en cero.
    DateTime::DateTime()
        : year_(0),
        month_(0),
        day_(0),
        hour_(0),
        minute_(0),
        second_(0)
    {
    }

    // Guarda cada componente recibido dentro del objeto.
    DateTime::DateTime(
        int year,
        int month,
        int day,
        int hour,
        int minute,
        int second
    )
        : year_(year),
        month_(month),
        day_(day),
        hour_(hour),
        minute_(minute),
        second_(second)
    {
    }

    // Devuelve el año almacenado.
    int DateTime::getYear() const
    {
        return year_;
    }

    // Devuelve el mes almacenado.
    int DateTime::getMonth() const
    {
        return month_;
    }

    // Devuelve el día almacenado.
    int DateTime::getDay() const
    {
        return day_;
    }

    // Devuelve la hora almacenada.
    int DateTime::getHour() const
    {
        return hour_;
    }

    // Devuelve los minutos almacenados.
    int DateTime::getMinute() const
    {
        return minute_;
    }

    // Devuelve los segundos almacenados.
    int DateTime::getSecond() const
    {
        return second_;
    }

    // Convierte la fecha al formato utilizado por TinySQLDb.
    std::string DateTime::toString() const
    {
        std::ostringstream output;

        output << std::setfill('0')
            << std::setw(4) << year_ << "-"
            << std::setw(2) << month_ << "-"
            << std::setw(2) << day_ << " "
            << std::setw(2) << hour_ << ":"
            << std::setw(2) << minute_ << ":"
            << std::setw(2) << second_;

        return output.str();
    }

    // Valida el formato y los rangos antes de crear el objeto DateTime.
    bool DateTime::tryParse(
        const std::string& text,
        DateTime& result
    )
    {
        if (text.size() != 19)
        {
            return false;
        }

        if (
            text[4] != '-' ||
            text[7] != '-' ||
            text[10] != ' ' ||
            text[13] != ':' ||
            text[16] != ':'
            )
        {
            return false;
        }

        if (!hasValidCharacters(text))
        {
            return false;
        }

        const int year = std::stoi(text.substr(0, 4));
        const int month = std::stoi(text.substr(5, 2));
        const int day = std::stoi(text.substr(8, 2));
        const int hour = std::stoi(text.substr(11, 2));
        const int minute = std::stoi(text.substr(14, 2));
        const int second = std::stoi(text.substr(17, 2));

        if (year < 1 || year > 9999)
        {
            return false;
        }

        if (month < 1 || month > 12)
        {
            return false;
        }

        if (
            day < 1 ||
            day > getDaysInMonth(year, month)
            )
        {
            return false;
        }

        if (hour < 0 || hour > 23)
        {
            return false;
        }

        if (minute < 0 || minute > 59)
        {
            return false;
        }

        if (second < 0 || second > 59)
        {
            return false;
        }

        result = DateTime(
            year,
            month,
            day,
            hour,
            minute,
            second
        );

        return true;
    }
}
