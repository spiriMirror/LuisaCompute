#include <luisa/muda/device.h>

namespace luisa::muda {
Device::Device(LCDevice &device) noexcept
    : _lc_device{device.impl_shared()} {
    using namespace luisa::compute;
    _default_stream = this->create_stream(StreamTag::COMPUTE);
}

luisa::compute::Stream Device::create_stream(luisa::compute::StreamTag tag) noexcept {
    auto stream = _lc_device.create_stream(tag);
    stream.set_log_callback(
        [](luisa::string_view msg) {
            // if start with "[ERROR]", abort the program
            if (msg.starts_with("[ERROR]")) {
                // remove "[ERROR]" prefix
                auto error_msg = msg.substr(7);
                LUISA_ERROR("[Device] {}", error_msg);
                std::abort();
            }
        });
    return stream;
}
}// namespace luisa::muda