#include <iostream>
#include <string>
#include <vector>

#include <TriangleCalculatorLib/Triangle.hpp>
#include <TriangleCalculatorLib/ReturnCode.hpp>
#include <TriangleCalculatorLib/TriangleCalculator.hpp>

#include <logging/logging.hpp>
#include "ostream_logger.hpp"
#include "version.hpp"

using namespace TriangleCalculatorLib;

// forward declarations
void initializeLogger();

int main(int argc, char** argv) {
    std::vector<std::string> args(argv + 1, argv + argc);

    initializeLogger();

    if(args.size() == 0) {
        LOGIFACE_LOG(info, "No arguments provided. Exiting.");
        return 0;
    }

    if(args[0] == "--help" || args[0] == "-h") {
        std::cout << "Triangle Calculator Application\n"
                  << "Usage: " << argv[0] << " [options] {values}\n"
                  << "Options:\n"
                  << "  -h, --help       Show this help message\n"
                  << "  -v, --version    Show application version\n"
                  << "  -c, --calculate <A> <B> <C> <a> <b> <c>\n"
                  << "           Calculate a triangle using positional arguments:\n"
                  << "           1: angleA (degrees)\n"
                  << "           2: angleB (degrees)\n"
                  << "           3: angleC (degrees)\n"
                  << "           4: sideA\n"
                  << "           5: sideB\n"
                  << "           6: sideC\n\n"

                  << "           Use ? for unknown values.\n\n"

                  << "   -s, --solution <n>\n"
                  << "           Select solution:\n"
                  << "           0   no solution (default: first valid solution)\n"
                  << "           1   first solution\n"
                  << "           2   second solution\n";
        return 0;
    }

    if(args[0] == "--version" || args[0] == "-v") {
        std::cout << "Triangle Calculator Application Version " + std::string(APP_VERSION) + "\n";
        return 0;
    }

    if(args[0] == "--calculate" || args[0] == "-c") {
        if(args.size() < 7) {
            LOGIFACE_LOG(error, "Invalid number of arguments for calculation. Expected 6 values.");
            return 1;
        }

        Triangle triangle;
        try {
            triangle.angleA = args[1] == "?" ? std::nullopt : std::optional<double>(std::stod(args[1]));
            triangle.angleB = args[2] == "?" ? std::nullopt : std::optional<double>(std::stod(args[2]));
            triangle.angleC = args[3] == "?" ? std::nullopt : std::optional<double>(std::stod(args[3]));
            triangle.sideA  = args[4] == "?" ? std::nullopt : std::optional<double>(std::stod(args[4]));
            triangle.sideB  = args[5] == "?" ? std::nullopt : std::optional<double>(std::stod(args[5]));
            triangle.sideC  = args[6] == "?" ? std::nullopt : std::optional<double>(std::stod(args[6]));
        } catch (const std::exception& e) {
            LOGIFACE_LOG(error, "Error parsing input values: " + std::string(e.what()));
            return 1;
        }

        AmbiguousCaseSolution ambiguousCaseSolution = AmbiguousCaseSolution::NoSolution;

        if(args.size() == 9)
        {
            if(args[7] == "-s" || args[7] == "--solution")
            {
                // proceed
                if(args[8] == "0")
                {
                    LOGIFACE_LOG(info, "No solution will be provided for ambiguous SSA cases.");
                    ambiguousCaseSolution = AmbiguousCaseSolution::NoSolution;
                }
                else if(args[8] == "1")
                {
                    LOGIFACE_LOG(info, "Using first solution for ambiguous SSA case.");
                    ambiguousCaseSolution = AmbiguousCaseSolution::FirstSolution;
                }
                else if(args[8] == "2")
                {
                    LOGIFACE_LOG(info, "Using second solution for ambiguous SSA case.");
                    ambiguousCaseSolution = AmbiguousCaseSolution::SecondSolution;
                }
                else
                {
                    LOGIFACE_LOG(error, "Invalid solution option provided. Use 0, 1, or 2.");
                    return 1;
                }
            }
        }

        TriangleCalculator calculator;
        Result result = calculator.finalizeTriangle(triangle, ambiguousCaseSolution);
        std::cout << "Calculated Triangle Properties:\n"
                  << "  angleA: " << (result.triangle.angleA.has_value() ? std::to_string(result.triangle.angleA.value()) : "?") << "\n"
                  << "  angleB: " << (result.triangle.angleB.has_value() ? std::to_string(result.triangle.angleB.value()) : "?") << "\n"
                  << "  angleC: " << (result.triangle.angleC.has_value() ? std::to_string(result.triangle.angleC.value()) : "?") << "\n"
                  << "  sideA: "  << (result.triangle.sideA.has_value()  ? std::to_string(result.triangle.sideA.value())  : "?") << "\n"
                  << "  sideB: "  << (result.triangle.sideB.has_value()  ? std::to_string(result.triangle.sideB.value())  : "?") << "\n"
                  << "  sideC: "  << (result.triangle.sideC.has_value()  ? std::to_string(result.triangle.sideC.value())  : "?") << "\n";
    }

    return 0;
}

void initializeLogger() {
    static logiface::ostream_logger app_logger{};
    app_logger.set_level(logiface::level::info);
    logiface::set_logger(&app_logger);
}
