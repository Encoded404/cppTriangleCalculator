#include <TriangleCalculatorLib/Triangle.hpp>
#include <TriangleCalculatorLib/TriangleCalculator.hpp>

#include "TriangleCalculatorBackend.hpp"

#include <cmath>

namespace TriangleCalculatorLib
{
    double degreesToRadians(double degrees)
    {
        return degrees * M_PI / 180.0;
    }

    double radiansToDegrees(double radians)
    {
        return radians * 180.0 / M_PI;
    }

    Triangle ConvertTriangleToRadians(const Triangle& triangle)
    {
        Triangle radTriangle = triangle;
        if (radTriangle.angleA.has_value())
        {
            radTriangle.angleA = degreesToRadians(*radTriangle.angleA);
        }
        if (radTriangle.angleB.has_value())
        {
            radTriangle.angleB = degreesToRadians(*radTriangle.angleB);
        }
        if (radTriangle.angleC.has_value())
        {
            radTriangle.angleC = degreesToRadians(*radTriangle.angleC);
        }
        return radTriangle;
    }

    Triangle ConvertTriangleToDegrees(const Triangle& triangle)
    {
        Triangle degTriangle = triangle;
        if (degTriangle.angleA.has_value())
        {
            degTriangle.angleA = radiansToDegrees(*degTriangle.angleA);
        }
        if (degTriangle.angleB.has_value())
        {
            degTriangle.angleB = radiansToDegrees(*degTriangle.angleB);
        }
        if (degTriangle.angleC.has_value())
        {
            degTriangle.angleC = radiansToDegrees(*degTriangle.angleC);
        }
        return degTriangle;
    }

    Result TriangleCalculator::finalizeTriangle(Triangle triangle, AmbiguousCaseSolution ambiguousCaseSolution)
    {
        triangle = ConvertTriangleToRadians(triangle);
        
        TriangleCalculatorBackend backend;
        Result result = backend.finalizeTriangle(triangle, ambiguousCaseSolution);

        result.triangle = ConvertTriangleToDegrees(result.triangle);
        return result;
    }

    std::pair<double, double> TriangleCalculator::getBaseHeight(Triangle triangle)
    {
        // Implementation goes here
        return {0.0, 0.0};
    }

    double TriangleCalculator::area(Triangle triangle)
    {
        // Implementation goes here
        return 0.0;
    }

    double TriangleCalculator::perimeter(Triangle triangle)
    {
        // Implementation goes here
        return 0.0;
    }
} // namespace TriangleCalculatorLib