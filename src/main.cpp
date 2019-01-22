#include <platform_glfw.hpp>
#include <instance.hpp>
#include <physical_device.hpp>
#include <surface.hpp>
#include <queue_family.hpp>
#include <device.hpp>
#include <buffer.hpp>
#include <image.hpp>
#include <image_view.hpp>
#include <descriptor_pool.hpp>
#include <descriptor_set_layout.hpp>
#include <descriptor_set.hpp>
#include <swapchain.hpp>
#include <framebuffer.hpp>
#include <fence.hpp>
#include <semaphore.hpp>
#include <shader_module.hpp>
#include <command_pool.hpp>
#include <command_buffer.hpp>
#include <pipeline_layout.hpp>
#include <pipeline.hpp>
#include <memory>
#include <move_into.hpp>
#include <Logger.hpp>

using namespace vka;
int main() {
    platform::glfw::init();
    std::unique_ptr<instance> instancePtr{};
    instance_builder{}
      .add_extensions(platform::glfw::get_required_instance_extensions())
      .add_layer(standard_validation)
      .build()
      .map(move_into{instancePtr})
      .map_error([](auto error){ multi_logger::get()->critical("Error creating instance!"); exit(error); });
    
    std::unique_ptr<surface> surfacePtr{};
    surface_builder{}
      .width(900)
      .height(900)
      .title("vkaTest1")
      .build(*instancePtr)
      .map(move_into{surfacePtr})
      .map_error([](auto error) {multi_logger::get()->critical("Error creating surface!"); exit(1); });

    VkPhysicalDevice physicalDevice{};
    physical_device_selector{}
      .select(*instancePtr)
      .map(move_into{physicalDevice})
      .map_error([](auto error) { multi_logger::get()->critical("Error selecting physical device!"); exit(1); });
    
    queue_family queueFamily{};
    queue_family_builder{}
      .queue(1.f)
      .graphics_support()
      .present_support(*surfacePtr)
      .build(physicalDevice)
      .map(move_into{queueFamily})
      .map_error([](auto error){ multi_logger::get()->critical("Error selecting queue family!"); exit(1); });
    
    std::unique_ptr<device> devicePtr{};
    device_builder{}
      .physical_device(physicalDevice)
      .extension(swapchain_extension)
      .add_queue_family(queueFamily)
      .build(*instancePtr)
      .map(move_into{devicePtr})
      .map_error([](auto error) { multi_logger::get()->critical("Error selecting physical device!"); exit(error); });
    
    platform::window_should_close shouldClose{};
    while (!(shouldClose = platform::glfw::poll_os(*surfacePtr))) {
      
    }
}