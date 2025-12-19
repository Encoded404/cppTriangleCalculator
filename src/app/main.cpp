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
    
    uint iterator = 0;

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
                  << "           2   second solution\n\n"

                  << "   -l, --log-level <level>\n"
                  << "           Set log level (trace, debug, info, warn, error, critical)\n";
        return 0;
    }

    if(args[0] == "--version" || args[0] == "-v") {
        std::cout << "Triangle Calculator Application Version " + std::string(APP_VERSION) + "\n";
        return 0;
    }

    if(args[iterator] == "--calculate" || args[iterator] == "-c") {
        ++iterator;
        if(args.size() < 7) {
            LOGIFACE_LOG(error, "Invalid number of arguments for calculation. Expected 6 values.");
            return 1;
        }

        Triangle triangle;
        try {
            triangle.angleA = args[iterator] == "?" ? std::nullopt : std::optional<double>(std::stod(args[iterator])); ++iterator;
            triangle.angleB = args[iterator] == "?" ? std::nullopt : std::optional<double>(std::stod(args[iterator])); ++iterator;
            triangle.angleC = args[iterator] == "?" ? std::nullopt : std::optional<double>(std::stod(args[iterator])); ++iterator;
            triangle.sideA  = args[iterator] == "?" ? std::nullopt : std::optional<double>(std::stod(args[iterator])); ++iterator;
            triangle.sideB  = args[iterator] == "?" ? std::nullopt : std::optional<double>(std::stod(args[iterator])); ++iterator;
            triangle.sideC  = args[iterator] == "?" ? std::nullopt : std::optional<double>(std::stod(args[iterator])); ++iterator;
        } catch (const std::exception& e) {
            LOGIFACE_LOG(error, "Error parsing input values: " + std::string(e.what()));
            return 1;
        }

        AmbiguousCaseSolution ambiguousCaseSolution = AmbiguousCaseSolution::NoSolution;

        if(args.size() > iterator + 1)
        {
            if(args[iterator] == "-s" || args[iterator] == "--solution")
            {
                // proceed
                ++iterator;
                if(args[iterator] == "0")
                {
                    LOGIFACE_LOG(info, "No solution will be provided for ambiguous SSA cases.");
                    ++iterator;
                    ambiguousCaseSolution = AmbiguousCaseSolution::NoSolution;
                }
                else if(args[iterator] == "1")
                {
                    LOGIFACE_LOG(info, "Using first solution for ambiguous SSA case.");
                    ++iterator;
                    ambiguousCaseSolution = AmbiguousCaseSolution::FirstSolution;
                }
                else if(args[iterator] == "2")
                {
                    LOGIFACE_LOG(info, "Using second solution for ambiguous SSA case.");
                    ++iterator;
                    ambiguousCaseSolution = AmbiguousCaseSolution::SecondSolution;
                }
                else
                {
                    LOGIFACE_LOG(error, "Invalid solution option provided. Use 0, 1, or 2.");
                    return 1;
                }
            }
        }

        // Process log level flag if present
        if(args.size() > iterator && (args[iterator] == "-l" || args[iterator] == "--log-level"))
        {
            if(args.size() <= iterator + 1)
            {
                LOGIFACE_LOG(error, "Log level flag requires an argument.");
                return 1;
            }
            ++iterator;
            logiface::logger* lg = logiface::get_logger();
            if(!lg)
            {
                LOGIFACE_LOG(error, "No logger initialized to set log level.");
                return 1;
            }
            if(args[iterator] == "trace")
            {
                lg->set_level(logiface::level::trace);
                LOGIFACE_LOG(trace, "logging logs at trace level or above.");
            }
            else if(args[iterator] == "debug")
            {
                lg->set_level(logiface::level::debug);
                LOGIFACE_LOG(debug, "logging logs at debug level or above.");
            }
            else if(args[iterator] == "info")
            {
                lg->set_level(logiface::level::info);
                LOGIFACE_LOG(info, "logging logs at info level or above.");
            }
            else if (args[iterator] == "warn")
            {
                lg->set_level(logiface::level::warn);
                LOGIFACE_LOG(warn, "logging logs at warn level or above.");
            }
            else if (args[iterator] == "error")
            {
                lg->set_level(logiface::level::error);
                LOGIFACE_LOG(error, "logging logs at error level or above.");
            }
            else if (args[iterator] == "critical")
            {
                lg->set_level(logiface::level::critical);
                LOGIFACE_LOG(error, "logging logs at critical level."); // we cant use critical log here, that would cause a actual exception
            }
            else
            {
                LOGIFACE_LOG(error, "Invalid log level provided. Use either trace, debug, info, warn, error, or critical.");
                return 1;
            }
            ++iterator;
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
