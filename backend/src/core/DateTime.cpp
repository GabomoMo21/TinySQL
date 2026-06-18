#include "core/DateTime.hpp"

#include <iomanip>
#include <sstream>

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
}
