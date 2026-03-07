#pragma once

#include "bladenoise/io/IOTypes.h"

namespace bladenoise {
namespace io {

class TextResultsWriter : public IResultsWriter {
public:
    bool write(const std::string& filename,
               const RealVector& frequencies,
               const CombinedNoiseResults& results) override;

    std::string get_error() const override { return error_message_; }

private:
    std::string error_message_;
};

}  // namespace io
}  // namespace bladenoise
