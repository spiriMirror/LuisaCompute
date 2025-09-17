#pragma once
#include <luisa/runtime/context.h>
#include <luisa/runtime/device.h>
#include <luisa/runtime/stream.h>
#include <luisa/muda/buffer.h>
#include <luisa/muda/compile_guard.h>

namespace luisa::muda {
class Device {
private:
    using LCDevice = luisa::compute::Device;
    using LCStream = luisa::compute::Stream;
public:
    Device(LCDevice &device) noexcept;
    template<typename T>
    auto create_buffer() noexcept {
        return Buffer<T>{*this};
    }

    luisa::compute::Stream create_stream(luisa::compute::StreamTag tag = luisa::compute::StreamTag::COMPUTE) noexcept;

    [[nodiscard]] auto &default_stream() noexcept { return _default_stream; }
    [[nodiscard]] auto &lc_device() noexcept { return _lc_device; }
    [[nodiscard]] auto guard() noexcept { return CompileGuard{_lc_device}; }
private:
    LCDevice _lc_device;
    LCStream _default_stream;
};
}// namespace luisa::muda