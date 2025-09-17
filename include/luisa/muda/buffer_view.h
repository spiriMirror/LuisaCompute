#pragma once
#include <luisa/dsl/syntax.h>
#include <source_location>
#include <luisa/muda/compile_guard.h>

namespace luisa::muda {

template<class T>
class BufferView {
public:
    BufferView(luisa::compute::BufferView<T> view, size_t size) noexcept
        : _view{std::move(view)}, _size{static_cast<uint>(size)} {}
    [[nodiscard]] uint size() const noexcept { return _size; }
    [[nodiscard]] auto subview(uint offset, uint size) const noexcept {
        LUISA_ASSERT(offset + size <= _size, "BufferView subview out of range.");
        return BufferView<T>{_view.subview(offset, size), size};
    }
    [[nodiscard]] void copy_to(luisa::compute::CommandList &cmd_list, luisa::span<T> host) {
        LUISA_ASSERT(host.size() == _size, "size mismatch, dst={}, src={}", host.size(), _size);
        cmd_list << _view.copy_to(host.data());
    }

    [[nodiscard]] void copy_from(luisa::compute::CommandList &cmd_list, luisa::span<const T> host) {
        LUISA_ASSERT(host.size() == _size, "size mismatch, dst={}, src={}", host.size(), _size);
        cmd_list << _view.copy_from(host.data());
    }
    // private: ---------------------------------------------------------
    luisa::compute::BufferView<T> _view;
    uint _size;
};

template<typename T>
class Accessor {
public:
    class Element {
    public:
        Element(luisa::compute::Int index,
                luisa_compute_extension<luisa::muda::BufferView<T>> &buffer,
                const std::source_location &loc = std::source_location::current()) noexcept
            : _index{index}, _impl{buffer}, _loc{loc} {
        }

        template<typename U>
        Element &operator=(U &&value) && noexcept {
            write(std::forward<U>(value));
            return *this;
        }

        friend luisa::compute::Var<T> operator+(Element lhs, luisa::compute::Var<T> rhs) noexcept {
            return lhs.read() + rhs;
        }

        friend luisa::compute::Var<T> operator+(luisa::compute::Var<T> lhs, Element rhs) noexcept {
            return lhs + rhs.read();
        }

        friend luisa::compute::Var<T> operator+(Element lhs, Element rhs) noexcept {
            return lhs.read() + rhs.read();
        }

        operator luisa::compute::Var<T>() noexcept {
            return read();
        }

        luisa::compute::Var<T> read() noexcept {
            using namespace luisa::compute::dsl;
            luisa::compute::Var<T> result = T{0};
            if_(check_in_range(),
                [&] {
                    result = _impl->_view.read(_index);
                });

            return result;
        }

        void write(const luisa::compute::Var<T> &value) noexcept {
            using namespace luisa::compute::dsl;
            if_(check_in_range(),
                [&] {
                    _impl->_view.write(_index, value);
                });
        }

        luisa::compute::Bool check_in_range() noexcept {
            using namespace luisa::compute::dsl;
            auto trace = fmt::format("[{}({})]", _loc.file_name(), _loc.line());
            luisa::compute::Bool in_range = _index >= 0 & _index < _impl->_size;

            if_(!in_range, [&] {
                if (muda::CompileGuard::current_backend() == "cuda") {
                    luisa::compute::cuda_printf("[ERROR][OUT_OF_RANGE_ERROR at (%d,%d,%d)]: Out of range access at index (%d), size (%d)." + trace + "\n", dispatch_id().x, dispatch_id().y, dispatch_id().z, _index, _impl->_size);
                } else {
                    luisa::compute::device_log("[ERROR][OUT_OF_RANGE_ERROR at ({},{},{}) ]: Out of range access at index ({}, size ({})." + trace, dispatch_id().x, dispatch_id().y, dispatch_id().z, _index, _impl->_size);
                }
            });
            luisa::compute::device_assert(in_range, "BufferView out of range." + trace);
            return in_range;
        }

    private:
        luisa::compute::Int _index;
        luisa_compute_extension<luisa::muda::BufferView<T>> &_impl;
        const std::source_location _loc;
    };

    Accessor(luisa_compute_extension<luisa::muda::BufferView<T>> &impl) noexcept
        : _impl{impl} {
    }

    Element operator()(const luisa::compute::Int &i, const std::source_location &loc = std::source_location::current()) {
        return Element{i, _impl, loc};
    }

    auto size() const noexcept { return _impl->_size; }

private:
    luisa_compute_extension<luisa::muda::BufferView<T>> &_impl;
};

}// namespace luisa::muda

#define TEMPLATE_T() \
    template<class T>

LUISA_BINDING_GROUP_TEMPLATE(TEMPLATE_T, luisa::muda::BufferView<T>, _view, _size) {
    auto viewer() noexcept { return luisa::muda::Accessor<T>{*this}; }
};

#undef TEMPLATE_T