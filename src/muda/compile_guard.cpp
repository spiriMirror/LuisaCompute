#include <luisa/muda/compile_guard.h>
#include <luisa/muda/device.h>

namespace luisa::muda {
thread_local luisa::string CompileGuard::s_current_backend;

CompileGuard::CompileGuard(luisa::compute::Device &device) noexcept {
    s_current_backend = device.backend_name();
}

CompileGuard::~CompileGuard() {
    s_current_backend.clear();
}
}// namespace luisa::muda