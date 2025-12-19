#ifndef TRIANGLE_CALCULATOR_HPP
#define TRIANGLE_CALCULATOR_HPP

#include "ReturnCode.hpp"
#include "Triangle.hpp"

#include <optional>
#include <utility>

namespace TriangleCalculatorLib
{
    class TriangleCalculator {
    public:
        /// Finalize the triangle by calculating missing sides and angles
        /// @param triangle The triangle to finalize
        /// @return The finalized triangle with all sides and angles calculated
        static Result finalizeTriangle(Triangle triangle, AmbiguousCaseSolution ambiguousCaseSolution = AmbiguousCaseSolution::NoSolution);
        
        /// Get the base and height of a triangle
        /// @param triangle The triangle for which to get the base and height
        /// @return A pair containing the base and height of the triangle (in that order)
        static std::pair<double, double> getBaseHeight(Triangle triangle);
    
        /// Calculate the area of a triangle given its base and height
        /// @param triangle The triangle for which to calculate the area
        /// @return The area of the triangle
        static double area(Triangle triangle);
    
        /// Calculate the perimeter of a triangle given the lengths of its three sides
        /// @param triangle The triangle for which to calculate the perimeter
        /// @return The perimeter of the triangle
        static double perimeter(Triangle triangle);
    };
} // namespace TriangleCalculatorLib

#endif