#pragma once
#include <string>
#include <memory>

namespace JsonLoader {
    template<typename T>
    std::shared_ptr<T> load(const std::string& filePath);
}
