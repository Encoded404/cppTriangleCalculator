#include "TriangleCalculatorBackend.hpp"

#include "TrianglePointerView.hpp"

#include <TriangleCalculatorLib/Triangle.hpp>
#include <TriangleCalculatorLib/ReturnCode.hpp>
#include <logging/logging.hpp>

#include <cmath>
#include <iostream>
#include <limits>

namespace TriangleCalculatorLib
{
    // Utility function for floating-point comparison with absolute tolerance
    // Using absolute tolerance (1e-9) instead of relative to avoid precision issues
    constexpr double EPSILON = std::numeric_limits<double>::epsilon();
    constexpr double ABSOLUTE_TOLERANCE = 2e-7;
    
    bool IsEqual(double a, double b, double epsilon = ABSOLUTE_TOLERANCE)
    {
        return std::abs(a - b) <= epsilon * std::max({1.0, std::abs(a), std::abs(b)});
    }
    
    bool IsLess(double a, double b, double epsilon = ABSOLUTE_TOLERANCE)
    {
        return a < b - epsilon * std::max({1.0, std::abs(a), std::abs(b)});
    }
    
    bool IsLessOrEqual(double a, double b, double epsilon = ABSOLUTE_TOLERANCE)
    {
        return a < b + epsilon * std::max({1.0, std::abs(a), std::abs(b)});
    }
    
    bool IsGreater(double a, double b, double epsilon = ABSOLUTE_TOLERANCE)
    {
        return a > b + epsilon * std::max({1.0, std::abs(a), std::abs(b)});
    }

    // if 2 out of 3 angles are known, we can calculate the third angle
    void SimpleSolveAngles(TrianglePointerView& tri)
    {
        LOGIFACE_LOG(trace, "2 angles known, calculating the third angle");
        int unknownAngleIndex = tri.findFirstUnknownAngleIndex();
        // rotate so that the unknown angle is angleA
        tri.Rotate(unknownAngleIndex);
        
        // Calculate the third angle
        double angleSum = 0.0;
        angleSum += **tri.angleB;
        angleSum += **tri.angleC;
        
        // we use angles in radians here, so the sum of angles in a triangle is pi radians (180 degrees)
        double thirdAngle = M_PI - angleSum;
        *tri.angleA = thirdAngle;
    }

    // all sides known and 1 angle, we can solve all angles
    // the angle of the side that is largest is always optuse so we can use the law of cosines to solve it.
    // then we solve the other angles using the law of sines
    
    // law of cosines:
    // c^2 = a^2 + b^2 - 2ab*cos(C)
    // for example:
    // cos(B) = (a^2 + c^2 - b^2) / (2ac)
    
    // law of sines:
    // a / sin(A) = b / sin(B) = c / sin(C)
    // for example:
    // sin(B) = b * sin(A) / a
    void SolveAnglesWithSides(TrianglePointerView& tri)
    {
        LOGIFACE_LOG(trace, "all sides known and 1 angle, solving angles using law of cosines and law of sines");
        // find the largest side
        int largestSideIndex = tri.findLargestSideIndex();
        tri.Rotate(largestSideIndex); // rotate so that largest side is sideA
        
        if(!tri.angleA->has_value())
        {
            // solve angleA using law of cosines
            // this is what we do, but by using fma to reduce floating point errors (less rounding steps)
            // double a2 = **tri.sideA * **tri.sideA;
            // double b2 = **tri.sideB * **tri.sideB;
            // double c2 = **tri.sideC * **tri.sideC;
            double step = std::fma(**tri.sideB, **tri.sideB, std::fma(**tri.sideC, **tri.sideC, -(**tri.sideA * **tri.sideA)));
            double cosA = step / (2 * (**tri.sideB) * (**tri.sideC));
            cosA = std::max(-1.0, std::min(1.0, cosA));
            *tri.angleA = std::acos(cosA);
        }
        
        double aSideAnglePreFactor = std::sin(**tri.angleA) / **tri.sideA;
        
        if(!tri.angleB->has_value())
        {
            if(tri.KnownAngleCount() == 2)
            {
                // if 2 angles are known, we can calculate the third angle
                *tri.angleB = M_PI - **tri.angleA - **tri.angleC;
                return;
            }

            // solve angleB using law of sines
            double sinB = (**tri.sideB) * aSideAnglePreFactor;
            sinB = std::max(-1.0, std::min(1.0, sinB));
            *tri.angleB = std::asin(sinB);

            if(!tri.angleC->has_value())
            {
                // now we can calculate angleC
                *tri.angleC = M_PI - **tri.angleA - **tri.angleB;
            }
        }
        else if(!tri.angleC->has_value())
        {
            *tri.angleC = M_PI - **tri.angleA - **tri.angleB;
        }
    }

    // all angles are known so we just solve the sides using the law of sines by multiplying with a common factor (one side)
    // sideA / sin(angleA) = sideB / sin(angleB) = sideC / sin(angleC)
    // we pick the first known side to calculate the common factor
    // sideA * sin(angleB) / sin(angleA) = sideB
    void SolveSides(TrianglePointerView& tri)
    {
        LOGIFACE_LOG(trace, "all angles known, solving sides using law of sines");
        if (!tri.sideB->has_value() || IsLessOrEqual(**tri.sideB, 0))
        {
            double temp = **tri.sideA * std::sin(**tri.angleB);
            *tri.sideB = temp / std::sin(**tri.angleA);
        }

        if (!tri.sideC->has_value() || IsLessOrEqual(**tri.sideC, 0))
        {
            double temp = **tri.sideA * std::sin(**tri.angleC);
            *tri.sideC = temp / std::sin(**tri.angleA);
        }
    }
    

    double GetSSAHeight(TrianglePointerView& triView)
    {
        // we simply asume its prerotated so that angleA and sidea are known
        // int aIndex = triView.findFirstKnownAngleIndex();
        // triView.Rotate(aIndex); // Rotate so that known angle is A

        // now we determine what the "other" side is (c in calculations)
        double c = 0;
        if(triView.sideB->has_value() && IsGreater(**triView.sideB, 0))
        {
            // side b is known
            c = **triView.sideB;
        }
        else if(triView.sideC->has_value() && IsGreater(**triView.sideC, 0))
        {
            // side c is known
            c = **triView.sideC;
        }
        double h = c * std::sin(**triView.angleA);
        return h;
    }

    // in a side-side-angle (SSA) case, solve the triangle using the law of sines
    bool ResolveSSA(TrianglePointerView& tri, AmbiguousCaseSolution ambiguousCaseSolution)
    {
        tri.Rotate(tri.findFirstKnownAngleIndex()); // Rotate so that known angle is angleA
        
        // a pointer to the side and angle we solve for
        std::optional<double>* sideToSolveFrom = nullptr;
        std::optional<double>* angleToSolve = nullptr;
        if(tri.sideB->has_value())
        {
            sideToSolveFrom = tri.sideB;
            angleToSolve = tri.angleB;
            LOGIFACE_LOG(trace, "Solving SSA case using sideB and angleB");
        }
        else if(tri.sideC->has_value())
        {
            sideToSolveFrom = tri.sideC;
            angleToSolve = tri.angleC;
            LOGIFACE_LOG(trace, "Solving SSA case using sideC and angleC");
        }

        double h = GetSSAHeight(tri);
        double a = **tri.sideA;

        bool hasTwoSolutions = false;
        
        // If a is less than h, it's truly impossible to form a triangle
        // If they're equal (within tolerance), it's a valid degenerate case with one solution
        if(IsLess(a, h) && !IsEqual(a, h))
        {
            // No solution - a is genuinely less than h
            LOGIFACE_LOG(warn, "The provided triangle data results in no valid triangle (side a < h) a: " + std::to_string(a) + " h: " + std::to_string(h));
            return false;
        }
        else if(IsEqual(a, h))
        {
            // Degenerate case: a ≈ h means angle B is 90 degrees
            // Treat as valid with one solution
            LOGIFACE_LOG(trace, "SSA degenerate case detected where a ≈ h (angle B is right angle)");
            *angleToSolve = M_PI / 2.0; // 90 degrees in radians
            return true;
        }
        else if(IsLess(h, a) && IsLess(a, **sideToSolveFrom))
        {
            hasTwoSolutions = true;
            
            if(ambiguousCaseSolution == AmbiguousCaseSolution::NoSolution)
            {
                LOGIFACE_LOG(warn, "The provided triangle data results in an ambiguous SSA case with two possible solutions, either provide more information or specify which solution to use, by default the first solution is used");
            }
            else
            {
                LOGIFACE_LOG(trace, "The provided triangle data results in an ambiguous SSA case with two possible solutions");
            }
        }
        
        LOGIFACE_LOG(trace, "Solving for the unknown angle using the law of sines");
        double sinB = (**sideToSolveFrom) * std::sin(**tri.angleA) / (**tri.sideA);
        sinB = std::max(-1.0, std::min(1.0, sinB));
        *angleToSolve = std::asin(sinB);

        // if this angle becomes NaN something went wrong
        if(!angleToSolve->has_value() || std::isnan(**angleToSolve))
        {
            LOGIFACE_LOG(warn, "Failed to solve SSA case, resulting angle is NaN\n"
                "here is a summary of the triangle data:\n"
                "\tsideA: " + (tri.sideA->has_value() ? std::to_string(**tri.sideA) : "?") + "\n"
                "\tsideB: " + (tri.sideB->has_value() ? std::to_string(**tri.sideB) : "?") + "\n"
                "\tsideC: " + (tri.sideC->has_value() ? std::to_string(**tri.sideC) : "?") + "\n"
                "\tangleA: " + (tri.angleA->has_value() ? std::to_string(**tri.angleA) : "?") + "\n"
                "\tangleB: " + (tri.angleB->has_value() ? std::to_string(**tri.angleB) : "?") + "\n"
                "\tangleC: " + (tri.angleC->has_value() ? std::to_string(**tri.angleC) : "?") + "\n"
                "sideToSolveFrom value: " + std::to_string(**sideToSolveFrom) + "\n"
                "angleToSolve value: " + std::to_string(**angleToSolve) + "\n"
            );
            return false;
        }

        if( hasTwoSolutions && ambiguousCaseSolution == AmbiguousCaseSolution::SecondSolution)
        {
            LOGIFACE_LOG(trace, "Solving for the second solution of the ambiguous SSA case");
            *angleToSolve = M_PI - **angleToSolve;
        }

        return true;
    }

    
    // 2 sides are known and the angle between them is also known
    void SolveSideWithAngleCos(TrianglePointerView& tri)
    {
        LOGIFACE_LOG(trace, "2 sides known and the angle between them is also known, solving the unknown side using the law of cosines");
        // this is what we do, but by using fma to reduce floating point errors (less rounding steps)        
        // double squaredSides = **tri.sideB * **tri.sideB +
        //                       **tri.sideC * **tri.sideC;
        double subtractor = 2 * **tri.sideB * **tri.sideC * std::cos(**tri.angleA);
        // double result = squaredSides - subtractor;
        double result = std::fma(**tri.sideB, **tri.sideB, std::fma(**tri.sideC, **tri.sideC, -subtractor));
        result = std::max(0.0, result); // prevent negative values due to floating point errors
        *tri.sideA = std::sqrt(result);
    }

    Result TriangleCalculatorBackend::finalizeTriangle(Triangle triangle, AmbiguousCaseSolution ambiguousCaseSolution)
    {
        // this is a workflow based triangle calculator

        // there are a set number of solvable cases, we check which case applies and call the relevant solver

        // cases
        // SSS - all sides known
        
        // SAS - 2 sides and the included angle known
        
        // ASA - 2 angles and the included side known
        // AAS - 2 angles and a non-included side known
        
        // SSA - 2 sides and a non-included angle known (ambiguous case)
        
        // here we init some values
        TrianglePointerView triView = TrianglePointerView(triangle);
        Result result;
        result.code = ResultCode::Success;
        result.triangle = triangle; // default to input triangle

        std::string logMessage = std::string("got triangle:") +
                                            "\n\ta=" + (triangle.sideA.has_value() ? std::to_string(triangle.sideA.value()) : "?") +
                                            "\n\tb=" + (triangle.sideB.has_value() ? std::to_string(triangle.sideB.value()) : "?") +
                                            "\n\tc=" + (triangle.sideC.has_value() ? std::to_string(triangle.sideC.value()) : "?") +
                                            "\n\tA=" + (triangle.angleA.has_value() ? std::to_string(triangle.angleA.value()) : "?") +
                                            "\n\tB=" + (triangle.angleB.has_value() ? std::to_string(triangle.angleB.value()) : "?") +
                                            "\n\tC=" + (triangle.angleC.has_value() ? std::to_string(triangle.angleC.value()) : "?");
        LOGIFACE_LOG(info, logMessage);
        
        // check which case applies

        // not solvable cases
        if(triView.KnownAngleCount() + triView.KnownSideCount() < 3)
        {
            // Not enough information to finalize the triangle
            LOGIFACE_LOG(warn, "Not enough information to finalize the triangle");
            result.code = ResultCode::InsufficientData;
            return result;
        }
        else if (triView.KnownSideCount() == 0)
        {
            // Not enough information to finalize the triangle
            LOGIFACE_LOG(warn, "Not enough sides known to finalize the triangle");
            result.code = ResultCode::InsufficientData;
            return result;
        }

        // harder multiple missing values cases
        // SSS case
        else if(triView.KnownSideCount() == 3)
        {
            // SSS case
            LOGIFACE_LOG(trace, "SSS case detected");

            if(triView.KnownAngleCount() == 3)
            {
                // triangle is already complete
                LOGIFACE_LOG(info, "Triangle is already complete");
                result.triangle = triangle;
                return result;
            }
            else if (triView.KnownAngleCount() == 2)
            {
                // two angles known, calculate the third angle
                SimpleSolveAngles(triView);
            }
            else
            {
                // all sides known and 1 angle, solve angles using law of cosines
                SolveAnglesWithSides(triView);
            }

        }
        // SAS SSA
        else if(triView.KnownSideCount() == 2 && triView.KnownAngleCount() == 1)
        {
            // could be SAS or SSA
            LOGIFACE_LOG(trace, "2 sides and 1 angle known, determining if SAS or SSA case");
            int knownAngleIndex = triView.findFirstKnownAngleIndex();
            triView.Rotate(knownAngleIndex); // Rotate so that known angle is angleA

            // now we check if the known angle is included between the two known sides
            bool sideBKnown = triView.sideB->has_value() && IsGreater(**triView.sideB, 0);
            bool sideCKnown = triView.sideC->has_value() && IsGreater(**triView.sideC, 0);

            if(sideBKnown && sideCKnown)
            {
                // SAS case
                LOGIFACE_LOG(trace, "SAS case detected");
                SolveSideWithAngleCos(triView);
                SolveAnglesWithSides(triView);
            }
            else
            {
                // SSA case
                LOGIFACE_LOG(trace, "SSA case detected");
                if(ResolveSSA(triView, ambiguousCaseSolution))
                {
                    SimpleSolveAngles(triView);
                    triView.Rotate(triView.findFirstKnownSideIndex());
                    SolveSides(triView);
                }

            }
        }
        // ASA or AAS
        else if(triView.KnownAngleCount() >= 2 && triView.KnownSideCount() < 3)
        {
            // ASA or AAS case
            LOGIFACE_LOG(trace, "ASA/AAS case detected");
            if(triView.KnownAngleCount() == 2)
            { SimpleSolveAngles(triView); }
            int knownSideIndex = triView.findFirstKnownSideIndex();
            // Rotate so that the known side is in the first position (sideA)
            triView.Rotate(knownSideIndex);
            SolveSides(triView);
        }

        // we are done, copy back the triangle
        result.triangle = triangle;

        std::string finalLogMessage = std::string("finalized triangle:") +
                                                "\n\ta=" + (triangle.sideA.has_value() ? std::to_string(triangle.sideA.value()) : "?") +
                                                "\n\tb=" + (triangle.sideB.has_value() ? std::to_string(triangle.sideB.value()) : "?") +
                                                "\n\tc=" + (triangle.sideC.has_value() ? std::to_string(triangle.sideC.value()) : "?") +
                                                "\n\tA=" + (triangle.angleA.has_value() ? std::to_string(triangle.angleA.value()) : "?") +
                                                "\n\tB=" + (triangle.angleB.has_value() ? std::to_string(triangle.angleB.value()) : "?") +
                                                "\n\tC=" + (triangle.angleC.has_value() ? std::to_string(triangle.angleC.value()) : "?");
        LOGIFACE_LOG(info, finalLogMessage);

        return result;
    }
} // namespace TriangleCalculatorLib