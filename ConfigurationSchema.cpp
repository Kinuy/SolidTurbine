#include "ConfigurationSchema.h"

// Single value parameters
ConfigurationSchema& ConfigurationSchema::add(
    const std::string& name, 
    std::unique_ptr<IValueParser> parser,
    bool required = true, 
    const std::string& description = "") 
{
    parameters.emplace_back(name, std::move(parser), required, description);
    return *this;
}

// Multi-value parameters
ConfigurationSchema& ConfigurationSchema::addMultiValue(
    const std::string& name, 
    std::unique_ptr<IMultiValueParser> parser,
    bool required = true, 
    const std::string& description = "") 
{
    parameters.emplace_back(name, std::move(parser), required, description);
    return *this;
}

const std::vector<ConfigurationParameter>& ConfigurationSchema::getParameters() const 
{ 
    return parameters; 
}

// Helper methods for common types
ConfigurationSchema& ConfigurationSchema::addDouble(
    const std::string& name, 
    bool required = true, 
    const std::string& desc = "") 
{
    return add(name, std::make_unique<DoubleParser>(), required, desc);
}

ConfigurationSchema& ConfigurationSchema::addInt(
    const std::string& name, 
    bool required = true, 
    const std::string& desc = "") 
{
    return add(name, std::make_unique<IntParser>(), required, desc);
}

ConfigurationSchema& ConfigurationSchema::addString(
    const std::string& name, 
    bool required = true, 
    const std::string& desc = "") 
{
    return add(name, std::make_unique<StringParser>(), required, desc);
}

ConfigurationSchema& ConfigurationSchema::addBool(
    const std::string& name, 
    bool required = true, 
    const std::string& desc = "") 
{
    return add(name, std::make_unique<BoolParser>(), required, desc);
}

// Helper methods for multi-value patterns
ConfigurationSchema& ConfigurationSchema::addRange(
    const std::string& triggerKey, 
    const std::string& startKey,
    const std::string& endKey, 
    const std::string& stepKey,
    bool required = true, 
    const std::string& desc = "") 
{
    auto rangeParser = std::make_unique<RangeParser>(startKey, endKey, stepKey,
        std::make_unique<DoubleParser>());
    return addMultiValue(triggerKey, std::move(rangeParser), required, desc);
}

ConfigurationSchema& ConfigurationSchema::addIntRange(
    const std::string& triggerKey, 
    const std::string& startKey,
    const std::string& endKey, 
    const std::string& stepKey,
    bool required = true, 
    const std::string& desc = "") 
{
    auto rangeParser = std::make_unique<RangeParser>(startKey, endKey, stepKey,
        std::make_unique<IntParser>());
    return addMultiValue(triggerKey, std::move(rangeParser), required, desc);
}

ConfigurationSchema& ConfigurationSchema::addDataFile(
    const std::string& name,
    bool required=true,
    const std::string& desc = "") {
    return add(name, std::make_unique<FilePathParser>(), required, desc);
}

bool ConfigurationSchema::isDataFileParameter(const std::string& name) const {
    for (const auto& param : parameters) {
        if (param.name == name && !param.isMultiValue) {
            return dynamic_cast<const FilePathParser*>(param.parser.get()) != nullptr;
        }
    }
    return false;
}