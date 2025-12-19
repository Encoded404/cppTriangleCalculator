// Simple ostream-based logger that routes messages to stdout/stderr streams.
#ifndef LOGGING_OSTREAM_LOGGER_HPP
#define LOGGING_OSTREAM_LOGGER_HPP

#include <iostream>

#include <logging/logging.hpp>

namespace logiface {

inline constexpr std::string_view to_string(level lvl) noexcept {
    switch (lvl) {
        case level::trace: return "trace";
        case level::debug: return "debug";
        case level::info: return "info";
        case level::warn: return "warn";
        case level::error: return "error";
        case level::critical: return "critical";
    }
    return "unknown";
}

class ostream_logger final : public logger {
public:
    explicit ostream_logger(std::ostream& out = std::clog,
                            std::ostream& err = std::cerr,
                            level min_level = level::trace) noexcept
        : out_{out}, err_{err}, min_level_{min_level} {}

    void log(const record& r) noexcept override {
        if (r.lvl < min_level_) return;
        
        auto& os = (r.lvl == level::error || r.lvl == level::critical || r.lvl == level::warn)
                        ? err_
                        : out_;
        os << '[' << to_string(r.lvl) << "] "
           /*<< r.file */<< ' ' << r.function << ':' << r.line
           << " | " << r.message << '\n';
    }

    void set_level(level lvl) noexcept override { min_level_ = lvl; }
    level get_level() const noexcept override { return min_level_; }

private:
    std::ostream& out_;
    std::ostream& err_;
    level min_level_;
};

} // namespace logiface

#endif // LOGGING_OSTREAM_LOGGER_HPP