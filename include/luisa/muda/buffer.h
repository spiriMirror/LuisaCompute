#pragma once
#include <luisa/runtime/buffer.h>
#include <luisa/muda/buffer_view.h>

namespace luisa::muda {
class Device;
// a resizable buffer
template<typename T>
class Buffer {
public:
    Buffer(Device &device);

    Buffer(Device &device, size_t size);

    void resize(size_t size) noexcept;

    [[nodiscard]] size_t size() const noexcept;

    [[nodiscard]] muda::BufferView<T> view() const noexcept;

    [[nodiscard]] size_t capacity() const noexcept { return _capacity; }

    [[nodiscard]] size_t size() noexcept { return _size; }

    void copy_to(luisa::span<T> host) {
        using namespace luisa::compute;
        CommandList cmd_list;
        view().copy_to(cmd_list, host);
        device.default_stream() << cmd_list.commit() << synchronize();
    }

    void copy_from(luisa::span<const T> host) {
        using namespace luisa::compute;
        CommandList cmd_list;
        view().copy_from(cmd_list, host);
        device.default_stream() << cmd_list.commit() << synchronize();
    }
private:
    muda::Device &device;
    luisa::compute::Buffer<T> _buffer;
    size_t _size{0};
    size_t _capacity{0};
};
}// namespace luisa::muda

#include <luisa/muda/device.h>

namespace luisa::muda {
template<typename T>
Buffer<T>::Buffer(Device &device) : device{device} {
}

template<typename T>
Buffer<T>::Buffer(Device &device, size_t size) : device{device} {
    resize(size);
}

template<typename T>
void Buffer<T>::resize(size_t size) noexcept {
    if (size > _capacity) {
        _buffer = device.lc_device().create_buffer<T>(size);
        _capacity = size;
    }
    _size = size;
}

template<typename T>
muda::BufferView<T> Buffer<T>::view() const noexcept {
    return muda::BufferView<T>{_buffer.view(), _size};
}
}// namespace luisa::muda

#include "details/buffer.inl"