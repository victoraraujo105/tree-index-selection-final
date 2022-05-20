#ifndef CSV_HPP
#define CSV_HPP

#include <string>
#include <vector>

template <typename T>
std::string csv(const T& values, const size_t& N)
{
    std::stringstream ss;

    for (size_t i = 0; i < N; i++)
    {
        if (i != 0 ) ss << ",";
        ss << values[i];
    }

    return ss.str();
}

template <typename T>
std::string csv(const T& values)
{
    std::stringstream ss;

    const auto& N = values.size();

    for (size_t i = 0; i < N; i++)
    {
        if (i != 0 ) ss << ",";
        ss << values[i];
    }

    return ss.str();
}

template <size_t N>
std::string csv(const std::string (&values)[N])
{
    return csv(values, N);
}

#endif // CSV_HPP