#ifndef TRIANGLE_CALCULATOR_RETURN_ENUM_HPP
#define TRIANGLE_CALCULATOR_RETURN_ENUM_HPP

#include "Triangle.hpp"

namespace TriangleCalculatorLib
{
    enum class ResultCode
    {
        Success,
        InsufficientData, // Not enough data to solve the triangle
        TriangleAmbiguous, // SSA case with two possible solutions (returned even if only one is requested)
        InvalidData // aka a non-existent triangle
    };
    struct Result
    {
        Triangle triangle;
        ResultCode code;
    };
    
} // namespace TriangleCalculatorLib


#endif // TRIANGLE_CALCULATOR_RETURN_ENUM_HPP