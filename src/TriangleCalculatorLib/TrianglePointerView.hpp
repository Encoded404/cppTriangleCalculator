#ifndef TRIANGLE_POINTER_VIEW_HPP
#define TRIANGLE_POINTER_VIEW_HPP

#include <TriangleCalculatorLib/Triangle.hpp>

#include <algorithm>
#include <array>
#include <cmath>

namespace TriangleCalculatorLib
{
    // Lightweight view that reorders triangle fields without moving the underlying values.
    class TrianglePointerView
    {
    public:
        // Construct from explicit pointers (useful for tests) and precalculate all rotations.
        TrianglePointerView(std::optional<double>* a, std::optional<double>* b, std::optional<double>* c,
                            std::optional<double>* A, std::optional<double>* B, std::optional<double>* C)
        {
            InitializeRotationStorage({a, b, c}, {A, B, C});
            ApplyRotation(0);
        }

        // Construct directly from a Triangle reference.
        explicit TrianglePointerView(Triangle& tri)
            : TrianglePointerView(&tri.sideA, &tri.sideB, &tri.sideC,
                                   &tri.angleA, &tri.angleB, &tri.angleC) {}

        // Return a rotated view without mutating the original triangle.
        static TrianglePointerView FromRotation(Triangle& tri, int rotations)
        {
            TrianglePointerView view(tri);
            view.Rotate(rotations);
            return view;
        }

        // Swap the view to a precomputed rotation.
        void Rotate(int rotations)
        {
            const int rot = NormalizeRotation(rotations);
            ApplyRotation(rot);
        }

        void ResetRotation()
        {
            ApplyRotation(0);
        }

        // Current side/angle pointer arrays in ABC order for the active rotation.
        std::array<std::optional<double>*, 3> getSideArray() const { return currentSideArray_; }
        std::array<std::optional<double>*, 3> getResetSideArray() const { return sideRotations_[0]; }
        std::array<std::optional<double>*, 3> getAngleArray() const { return currentAngleArray_; }
        std::array<std::optional<double>*, 3> getResetAngleArray() const { return angleRotations_[0]; }

        int KnownAngleCount() const
        {
            int count = 0;
            if (angleA && angleA->has_value()) ++count;
            if (angleB && angleB->has_value()) ++count;
            if (angleC && angleC->has_value()) ++count;
            return count;
        }

        int KnownSideCount() const
        {
            int count = 0;
            if (sideA && sideA->has_value() && **sideA > 0) ++count;
            if (sideB && sideB->has_value() && **sideB > 0) ++count;
            if (sideC && sideC->has_value() && **sideC > 0) ++count;
            return count;
        }

        int findFirstKnownSideIndex()
        {
            auto sides = getResetSideArray();
            for(int i = 0; i < 3; ++i)
            {
                if (sides[i] && sides[i]->has_value() && **sides[i] > 0)
                {
                    return i;
                }
            }
            return -1; // No known side
        }

        int findFirstUnknownSideIndex()
        {
            auto sides = getResetSideArray();
            for(int i = 0; i < 3; ++i)
            {
                if (!sides[i] || !sides[i]->has_value() || **sides[i] <= 0)
                {
                    return i;
                }
            }
            return -1; // No known side
        }

        int findFirstKnownAngleIndex()
        {
            auto angles = getResetAngleArray();
            for(int i = 0; i < 3; ++i)
            {
                if (angles[i] && angles[i]->has_value())
                {
                    return i;
                }
            }
            return -1; // No known angle
        }

        int findFirstUnknownAngleIndex()
        {
            auto angles = getResetAngleArray();
            for(int i = 0; i < 3; ++i)
            {
                if (!angles[i] || !angles[i]->has_value())
                {
                    return i;
                }
            }
            return -1; // All angles are known
        }

        int findSolvedAngleUnsolvedSidePair()
        {
            auto sides = getResetSideArray();
            auto angles = getResetAngleArray();
            for(int i = 0; i < 3; ++i)
            {
                if ((!sides[i] || !sides[i]->has_value() || *sides[i] <= 0) && angles[i] && angles[i]->has_value())
                {
                    return i;
                }
            }
            return -1; // No solved angle-side pair
        }

        int findSolvedAngleSidePair()
        {
            auto sides = getResetSideArray();
            auto angles = getResetAngleArray();
            for(int i = 0; i < 3; ++i)
            {
                if (sides[i] && sides[i]->has_value() && *sides[i] > 0 && angles[i] && angles[i]->has_value())
                {
                    return i;
                }
            }
            return -1; // No solved angle-side pair
        }

        int findLargestSideIndex()
        {
            auto sides = getResetSideArray();
            int largestIndex = -1;
            double largestValue = -1.0;
            for(int i = 0; i < 3; ++i)
            {
                if (sides[i]->has_value() && **sides[i] > largestValue)
                {
                    largestValue = **sides[i];
                    largestIndex = i;
                }
            }
            return largestIndex;
        }

        // Direct pointers for the active rotation.
        std::optional<double>* sideA{nullptr};
        std::optional<double>* sideB{nullptr};
        std::optional<double>* sideC{nullptr};
        std::optional<double>* angleA{nullptr};
        std::optional<double>* angleB{nullptr};
        std::optional<double>* angleC{nullptr};

    private:
        static int NormalizeRotation(int rotations)
        {
            return ((rotations + 3) % 3); // normalize negative rotations too
        }

        void InitializeRotationStorage(std::array<std::optional<double>*, 3> baseSides,
                                       std::array<std::optional<double>*, 3> baseAngles)
        {
            for (int rot = 0; rot < 3; ++rot)
            {
                auto sides = baseSides;
                auto angles = baseAngles;

                std::rotate(sides.begin(), sides.begin() + rot, sides.end());
                std::rotate(angles.begin(), angles.begin() + rot, angles.end());

                sideRotations_[rot] = sides;
                angleRotations_[rot] = angles;
            }
        }

        void ApplyRotation(int rotationIndex)
        {
            currentRotation_ = rotationIndex;
            currentSideArray_ = sideRotations_[currentRotation_];
            currentAngleArray_ = angleRotations_[currentRotation_];

            sideA = currentSideArray_[0];
            sideB = currentSideArray_[1];
            sideC = currentSideArray_[2];
            angleA = currentAngleArray_[0];
            angleB = currentAngleArray_[1];
            angleC = currentAngleArray_[2];
        }

        std::array<std::array<std::optional<double>*, 3>, 3> sideRotations_{};
        std::array<std::array<std::optional<double>*, 3>, 3> angleRotations_{};
        std::array<std::optional<double>*, 3> currentSideArray_{};
        std::array<std::optional<double>*, 3> currentAngleArray_{};
        int currentRotation_{0};
    };
} // namespace TriangleCalculatorLib

#endif // TRIANGLE_POINTER_VIEW_HPP