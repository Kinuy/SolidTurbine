#pragma once

#include "bladenoise/io/IOTypes.h"
#include <string>

namespace bladenoise {
namespace io {

class StreamlineWriter {
public:
    bool write(const std::string& filename,
               const StreamlineData& data,
               const std::string& format = "text") const;

    std::string get_error() const { return error_message_; }

private:
    bool write_text(const std::string& filename, const StreamlineData& data) const;
    bool write_csv(const std::string& filename, const StreamlineData& data) const;
    bool write_tecplot(const std::string& filename, const StreamlineData& data) const;
    bool write_vtk(const std::string& filename, const StreamlineData& data) const;

    mutable std::string error_message_;
};

}  // namespace io
}  // namespace bladenoise
