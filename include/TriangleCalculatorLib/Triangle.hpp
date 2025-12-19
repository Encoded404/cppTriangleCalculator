#ifndef TRIANGLE_HPP
#define TRIANGLE_HPP

#include <optional>

namespace TriangleCalculatorLib
{
    struct Triangle
    {
        std::optional<double> sideA;
        std::optional<double> sideB;
        std::optional<double> sideC;
        std::optional<double> angleA;
        std::optional<double> angleB;
        std::optional<double> angleC;

        // Method to reset all values to std::nullopt
        void reset() {
            sideA = std::nullopt;
            sideB = std::nullopt;
            sideC = std::nullopt;
            angleA = std::nullopt;
            angleB = std::nullopt;
            angleC = std::nullopt;
        }
    };

    enum class AmbiguousCaseSolution
    {
        NoSolution = 0,
        FirstSolution = 1,
        SecondSolution = 2
    };
} // namespace TriangleCalculatorLib

#endif // TRIANGLE_HPP