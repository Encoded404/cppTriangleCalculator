#ifndef TRIANGLE_CALCULATOR_BACKEND_HPP
#define TRIANGLE_CALCULATOR_BACKEND_HPP

#include <TriangleCalculatorLib/Triangle.hpp>
#include <TriangleCalculatorLib/ReturnCode.hpp>

#include <logging/logging.hpp>

namespace TriangleCalculatorLib
{
    class TriangleCalculatorBackend
    {
    public:
        static Result finalizeTriangle(Triangle triangle, AmbiguousCaseSolution ambiguousCaseSolution = AmbiguousCaseSolution::NoSolution);
    };
} // namespace TriangleCalculatorLib


#endif // TRIANGLE_CALCULATOR_BACKEND_HPP