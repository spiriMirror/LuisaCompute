#pragma once
#include <luisa/runtime/device.h>

namespace luisa::muda {
class CompileGuard {
public:
    CompileGuard(luisa::compute::Device &device) noexcept;
    ~CompileGuard();
    [[nodiscard]] static luisa::string_view current_backend() noexcept { return s_current_backend; }
private:
    static thread_local luisa::string s_current_backend;
};
}// namespace luisa::muda