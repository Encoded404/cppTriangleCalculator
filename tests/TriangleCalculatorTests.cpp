#include <gtest/gtest.h>

#include <logging/logging.hpp>
#include <nlohmann/json.hpp>

#include <TriangleCalculatorLib/Triangle.hpp>
#include <TriangleCalculatorLib/ReturnCode.hpp>
#include <TriangleCalculatorLib/TriangleCalculator.hpp>

#include <array>
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <optional>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include "test_logging.hpp"

using TriangleCalculatorLib::Triangle;
using TriangleCalculatorLib::TriangleCalculator;
using nlohmann::json;

namespace {
enum class Difficulty {
    Basic,
    Advanced,
    HardEdge,
};

// Install the per-test file logger once for the test binary.
[[maybe_unused]] const bool kLoggerInstalled = [] {
    test_logging::InstallPerTestFileLogger();
    return true;
}();

std::filesystem::path FixturePath() {
    return std::filesystem::path(__FILE__).parent_path() / "triangles_fp.json";
}

json LoadFixture() {
    const auto path = FixturePath();
    std::ifstream input(path);
    if (!input.is_open()) {
        ADD_FAILURE() << "Unable to open fixture file at " << path;
        return {};
    }

    json data;
    input >> data;
    return data;
}

Triangle ToTriangle(const json& value) {
    Triangle t;
    const auto& sides = value.at("sides");
    const auto& angles = value.at("angles");
    t.sideA = sides.at(0).get<double>();
    t.sideB = sides.at(1).get<double>();
    t.sideC = sides.at(2).get<double>();
    t.angleA = angles.at(0).get<double>();
    t.angleB = angles.at(1).get<double>();
    t.angleC = angles.at(2).get<double>();
    return t;
}

std::vector<Triangle> CollectAllTriangles(const json& fixture) {
    std::vector<Triangle> triangles;
    for (const auto& category : {"right", "equilateral", "isosceles", "scalene"}) {
        for (const auto& entry : fixture.at(category)) {
            triangles.push_back(ToTriangle(entry));
        }
    }
    return triangles;
}

std::string PrintTriangle(const Triangle& triangle) {
    const std::string missing = "missing";

    std::ostringstream oss;
    oss << "Triangle (\n";
    oss << "\tsideA: " << (triangle.sideA ? std::to_string(*triangle.sideA) : missing) << ",\n";
    oss << "\tsideB: " << (triangle.sideB ? std::to_string(*triangle.sideB) : missing) << ",\n";
    oss << "\tsideC: " << (triangle.sideC ? std::to_string(*triangle.sideC) : missing) << ",\n";
    oss << "\tangleA: " << (triangle.angleA ? std::to_string(*triangle.angleA) : missing) << ",\n";
    oss << "\tangleB: " << (triangle.angleB ? std::to_string(*triangle.angleB) : missing) << ",\n";
    oss << "\tangleC: " << (triangle.angleC ? std::to_string(*triangle.angleC) : missing) << "\n";
    oss << ")";
    return oss.str();
}

std::string FormatTrace(const Triangle& input, const Triangle& expected, const Triangle& output) {
    std::ostringstream oss;
    oss << "Input: " << PrintTriangle(input) << "\n";
    oss << "Expected: " << PrintTriangle(expected) << "\n";
    oss << "Output: " << PrintTriangle(output);
    return oss.str();
}

std::array<std::reference_wrapper<std::optional<double>>, 6> Fields(Triangle& t) {
    return {t.sideA, t.sideB, t.sideC, t.angleA, t.angleB, t.angleC};
}

void ExpectTriangleClose(const Triangle& actual, const Triangle& expected, double tol = 5e-4) {
    auto checkField = [&](const std::optional<double>& got, const std::optional<double>& exp, const char* name) {
        ASSERT_TRUE(exp.has_value()) << "Expected value missing for " << name;
        ASSERT_TRUE(got.has_value()) << name << " not computed";
        EXPECT_NEAR(*got, *exp, tol) << name << " mismatch";
    };

    checkField(actual.sideA, expected.sideA, "sideA");
    checkField(actual.sideB, expected.sideB, "sideB");
    checkField(actual.sideC, expected.sideC, "sideC");
    checkField(actual.angleA, expected.angleA, "angleA");
    checkField(actual.angleB, expected.angleB, "angleB");
    checkField(actual.angleC, expected.angleC, "angleC");
}

std::size_t KnownSideCount(const Triangle& triangle) {
    return static_cast<std::size_t>(triangle.sideA.has_value()) +
           static_cast<std::size_t>(triangle.sideB.has_value()) +
           static_cast<std::size_t>(triangle.sideC.has_value());
}

std::size_t KnownAngleCount(const Triangle& triangle) {
    return static_cast<std::size_t>(triangle.angleA.has_value()) +
           static_cast<std::size_t>(triangle.angleB.has_value()) +
           static_cast<std::size_t>(triangle.angleC.has_value());
}

std::size_t KnownValueCount(const Triangle& triangle) {
    return KnownSideCount(triangle) + KnownAngleCount(triangle);
}

bool IsSolvableByCalculator(const Triangle& triangle) {
    // The backend rejects cases with fewer than three known values or with no known sides.
    return KnownValueCount(triangle) >= 3 && KnownSideCount(triangle) > 0;
}

bool IsSSA(const Triangle& triangle) {
    if (KnownAngleCount(triangle) != 1 || KnownSideCount(triangle) != 2) {
        return false;
    }

    const bool angleAKnown = triangle.angleA.has_value();
    const bool angleBKnown = triangle.angleB.has_value();
    const bool angleCKnown = triangle.angleC.has_value();

    const bool sideAKnown = triangle.sideA.has_value();
    const bool sideBKnown = triangle.sideB.has_value();
    const bool sideCKnown = triangle.sideC.has_value();

    if (angleAKnown) {
        const int adjacentKnown = static_cast<int>(sideBKnown) + static_cast<int>(sideCKnown);
        return sideAKnown && adjacentKnown == 1;
    }
    if (angleBKnown) {
        const int adjacentKnown = static_cast<int>(sideAKnown) + static_cast<int>(sideCKnown);
        return sideBKnown && adjacentKnown == 1;
    }
    if (angleCKnown) {
        const int adjacentKnown = static_cast<int>(sideAKnown) + static_cast<int>(sideBKnown);
        return sideCKnown && adjacentKnown == 1;
    }

    return false;
}

bool IsAmbiguousSSA(const Triangle& triangle) {
    if (!IsSSA(triangle)) {
        return false;
    }

    // define a function that checks for ambiguity given known angle and sides
    auto hasAmbiguity = [](double knownAngleDeg, double oppositeSide, double otherSide) {
        const double knownAngleRad = knownAngleDeg * M_PI / 180.0;
        const double h = otherSide * std::sin(knownAngleRad);
        return h < oppositeSide && oppositeSide < otherSide;
    };


    // check which angle-side pair is known and apply the ambiguity check
    if (triangle.angleA && triangle.sideA) {
        if (triangle.sideB) {
            return hasAmbiguity(*triangle.angleA, *triangle.sideA, *triangle.sideB);
        }
        if (triangle.sideC) {
            return hasAmbiguity(*triangle.angleA, *triangle.sideA, *triangle.sideC);
        }
    }

    if (triangle.angleB && triangle.sideB) {
        if (triangle.sideA) {
            return hasAmbiguity(*triangle.angleB, *triangle.sideB, *triangle.sideA);
        }
        if (triangle.sideC) {
            return hasAmbiguity(*triangle.angleB, *triangle.sideB, *triangle.sideC);
        }
    }

    if (triangle.angleC && triangle.sideC) {
        if (triangle.sideA) {
            return hasAmbiguity(*triangle.angleC, *triangle.sideC, *triangle.sideA);
        }
        if (triangle.sideB) {
            return hasAmbiguity(*triangle.angleC, *triangle.sideC, *triangle.sideB);
        }
    }

    return false;
}

TriangleCalculatorLib::AmbiguousCaseSolution PickAmbiguousSolution(const Triangle& expected, const Triangle& partial) {
    using TriangleCalculatorLib::AmbiguousCaseSolution;

    // define a helper to choose the solution based on expected other angle
    // knownAngleDeg: the known angle in degrees (A)
    // oppositeSide: the side opposite the known angle (a)
    // otherSide: the other known side (b or c)
    // expectedOtherAngleDeg: the expected value of the other angle (B or C)
    auto choose = [](const std::optional<double>& expectedOtherAngleDeg) {
        if (!expectedOtherAngleDeg) {
            return AmbiguousCaseSolution::FirstSolution;
        }
        
        return *expectedOtherAngleDeg < 90.0 ? AmbiguousCaseSolution::FirstSolution : AmbiguousCaseSolution::SecondSolution;
    };

    // determine which angle-side pair is known and pick solution accordingly
    if (partial.angleA && partial.sideA) {
        if (partial.sideB) {
            return choose(expected.angleB);
        }
        if (partial.sideC) {
            return choose(expected.angleC);
        }
    }

    if (partial.angleB && partial.sideB) {
        if (partial.sideA) {
            return choose(expected.angleA);
        }
        if (partial.sideC) {
            return choose(expected.angleC);
        }
    }

    if (partial.angleC && partial.sideC) {
        if (partial.sideA) {
            return choose(expected.angleA);
        }
        if (partial.sideB) {
            return choose(expected.angleB);
        }
    }

    return AmbiguousCaseSolution::FirstSolution;
}

struct GenerationOptions {
    std::size_t minRemove;
    std::size_t maxRemove;
    bool requireSideRemoval;
    bool requireAngleRemoval;
    bool allowSSA;
    bool preferSSA;
};

GenerationOptions OptionsForDifficulty(Difficulty difficulty) {
    switch (difficulty) {
        case Difficulty::Basic:
            return GenerationOptions{1, 1, false, false, false, false};
        case Difficulty::Advanced:
            return GenerationOptions{2, 3, false, false, false, false};
        case Difficulty::HardEdge:
            return GenerationOptions{3, 3, true, true, true, true};
    }
    return GenerationOptions{1, 1, false, false, false, false};
}

struct PartialTriangleCase {
    Triangle partial;
    bool isSSA;
    bool isAmbiguousSSA;
    bool isSolvable;
    TriangleCalculatorLib::AmbiguousCaseSolution ambiguousChoice;
};

PartialTriangleCase GeneratePartialTriangle(const Triangle& expected, Difficulty difficulty, std::mt19937& rng) {
    const GenerationOptions options = OptionsForDifficulty(difficulty);
    constexpr int kMaxAttempts = 32;

    auto attemptGeneration = [&](bool enforceSSAPreference) {
        Triangle partial = expected;
        auto fields = Fields(partial);

        std::array<std::size_t, 6> order{0, 1, 2, 3, 4, 5};
        std::shuffle(order.begin(), order.end(), rng);

        const std::size_t removeRange = options.maxRemove - options.minRemove + 1;
        const std::size_t removeCount = options.minRemove + (removeRange > 1 ? rng() % removeRange : 0);

        std::size_t removedSides = 0;
        std::size_t removedAngles = 0;
        for (std::size_t i = 0; i < removeCount; ++i) {
            const std::size_t idx = order[i];
            if (idx < 3) {
                ++removedSides;
            } else {
                ++removedAngles;
            }
            fields[idx].get() = std::nullopt;
        }

        if (options.requireSideRemoval && removedSides == 0) {
            return std::optional<PartialTriangleCase>{};
        }
        if (options.requireAngleRemoval && removedAngles == 0) {
            return std::optional<PartialTriangleCase>{};
        }

        const bool isSSA = IsSSA(partial);
        if (!options.allowSSA && isSSA) {
            return std::optional<PartialTriangleCase>{};
        }
        if (options.preferSSA && enforceSSAPreference && !isSSA) {
            return std::optional<PartialTriangleCase>{};
        }

        const bool isAmbiguousSSA = isSSA && IsAmbiguousSSA(partial);
        const bool isSolvable = IsSolvableByCalculator(partial);

        auto ambiguousChoice = TriangleCalculatorLib::AmbiguousCaseSolution::NoSolution;
        if (isSSA) {
            ambiguousChoice = isAmbiguousSSA ? PickAmbiguousSolution(expected, partial)
                                             : TriangleCalculatorLib::AmbiguousCaseSolution::FirstSolution;
        }

        return std::optional<PartialTriangleCase>{PartialTriangleCase{partial, isSSA, isAmbiguousSSA, isSolvable, ambiguousChoice}};
    };

    for (int attempt = 0; attempt < kMaxAttempts; ++attempt) {
        const bool enforceSSA = options.preferSSA && attempt < (kMaxAttempts / 2);
        if (auto generated = attemptGeneration(enforceSSA)) {
            return *generated;
        }
    }

    // Fallback: deterministic removal of the minimum required fields.
    Triangle fallback = expected;
    auto fields = Fields(fallback);
    for (std::size_t i = 0; i < options.minRemove && i < fields.size(); ++i) {
        fields[i].get() = std::nullopt;
    }
    const bool isSSA = IsSSA(fallback);
    const bool isAmbiguousSSA = isSSA && IsAmbiguousSSA(fallback);
    const bool isSolvable = IsSolvableByCalculator(fallback);
    auto ambiguousChoice = TriangleCalculatorLib::AmbiguousCaseSolution::NoSolution;
    if (isSSA) {
        ambiguousChoice = isAmbiguousSSA ? PickAmbiguousSolution(expected, fallback)
                                         : TriangleCalculatorLib::AmbiguousCaseSolution::FirstSolution;
    }

    return PartialTriangleCase{fallback, isSSA, isAmbiguousSSA, isSolvable, ambiguousChoice};
}

void RunFinalizeTriangleTest(Difficulty difficulty) {
    const json fixture = LoadFixture();
    const auto seed = fixture.at("seed").get<uint32_t>();
    std::mt19937 rng(seed);

    auto expectedTriangles = CollectAllTriangles(fixture);
    ASSERT_FALSE(expectedTriangles.empty());

    int trianglesTested = 0;

    for (const auto& expected : expectedTriangles) {
        const PartialTriangleCase generation = GeneratePartialTriangle(expected, difficulty, rng);
        const TriangleCalculatorLib::Result finalizedResult = TriangleCalculator::finalizeTriangle(
            generation.partial, generation.ambiguousChoice);

        if (!generation.isSolvable) {
            EXPECT_EQ(finalizedResult.code, TriangleCalculatorLib::ResultCode::InsufficientData);
            continue;
        }

        SCOPED_TRACE('\n' + FormatTrace(generation.partial, expected, finalizedResult.triangle) + "\nTriangles tested so far: " + std::to_string(trianglesTested + 1));

        EXPECT_EQ(finalizedResult.code, TriangleCalculatorLib::ResultCode::Success);
        ExpectTriangleClose(finalizedResult.triangle, expected);
        ++trianglesTested;
    }
    
    GTEST_LOG_(INFO) << "All triangles tested successfully. Total triangles tested: " << trianglesTested;
}
}  // namespace

TEST(TriangleCalculatorTests, FinalizeTriangleBasicSingleMissingValue) {
    RunFinalizeTriangleTest(Difficulty::Basic);
}

TEST(TriangleCalculatorTests, FinalizeTriangleAdvancedMultipleMissingValues) {
    RunFinalizeTriangleTest(Difficulty::Advanced);
}

TEST(TriangleCalculatorTests, FinalizeTriangleHardEdgeCases) {
    RunFinalizeTriangleTest(Difficulty::HardEdge);
}