#include <luisa/core/logging.h>
#include <luisa/runtime/context.h>
#include <luisa/runtime/stream.h>
#include <luisa/runtime/image.h>
#include <luisa/runtime/shader.h>
#include <luisa/dsl/syntax.h>
#include <stb/stb_image_write.h>
#include <source_location>
#include <luisa/dsl/sugar.h>
#include <luisa/muda/device.h>
#include <luisa/muda/buffer_view.h>

int main(int argc, char *argv[]) {

    using namespace luisa;
    using namespace luisa::compute;

    log_level_verbose();

    // log_level_error();

    Context context{argv[0]};

    auto device = context.create_device("dx");
    auto MUDA = muda::Device{device};
    auto buffer = MUDA.create_buffer<float>();
    buffer.resize(10);

    auto guard = MUDA.guard();

    Kernel1D kernel = [](Var<muda::BufferView<float>> buf) noexcept {
        auto i = dispatch_id().x;
        auto viewer = buf->viewer();
        viewer(i+1) = 1;
    };

    auto shader = device.compile(kernel);

    MUDA.default_stream() << shader(buffer.view()).dispatch(buffer.size()) << synchronize();

    luisa::vector<float> host(10, 0.0f);
    buffer.copy_to(host);

    for (auto v : host) {
        LUISA_INFO("{}", v);
    }
}
