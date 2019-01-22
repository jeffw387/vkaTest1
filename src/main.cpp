#include <platform_glfw.hpp>
#include <instance.hpp>
#include <physical_device.hpp>
#include <surface.hpp>
#include <queue_family.hpp>
#include <device.hpp>
#include <buffer.hpp>
#include <image.hpp>
#include <image_view.hpp>
#include <descriptor_set_layout.hpp>
#include <descriptor_pool.hpp>
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
      .map_error([](auto error) { multi_logger::get()->critical("Error creating device!"); exit(error); });
    
    std::unique_ptr<swapchain> swapchainPtr{};
    swapchain_builder{}
      .image_count(3)
      .present_mode(VK_PRESENT_MODE_FIFO_KHR)
      .queue_family_index(queueFamily.familyIndex)
      .build(physicalDevice, *surfacePtr, *devicePtr)
      .map(move_into{swapchainPtr})
      .map_error([](auto error) { multi_logger::get()->critical("Error creating swapchain!"); exit(error); });

    std::unique_ptr<descriptor_set_layout> set0LayoutPtr{};
    descriptor_set_layout_builder{}
      .storage_buffer(0, 1, VK_SHADER_STAGE_FRAGMENT_BIT)
      .build(*devicePtr)
      .map(move_into{set0LayoutPtr})
      .map_error([](auto error) { multi_logger::get()->critical("Error creating set layout 0!"); });
    
    std::unique_ptr<descriptor_set_layout> set1LayoutPtr{};
    descriptor_set_layout_builder{}
      .storage_buffer(1, 1, VK_SHADER_STAGE_FRAGMENT_BIT)
      .build(*devicePtr)
      .map(move_into{set1LayoutPtr})
      .map_error([](auto error) { multi_logger::get()->critical("Error creating set layout 1!"); });

    std::unique_ptr<descriptor_set_layout> set2LayoutPtr{};
    descriptor_set_layout_builder{}
      .uniform_buffer(2, 1, VK_SHADER_STAGE_FRAGMENT_BIT)
      .build(*devicePtr)
      .map(move_into{set2LayoutPtr})
      .map_error([](auto error) { multi_logger::get()->critical("Error creating set layout 2!"); });

    std::unique_ptr<descriptor_set_layout> set3LayoutPtr{};
    descriptor_set_layout_builder{}
      .uniform_buffer(3, 1, VK_SHADER_STAGE_VERTEX_BIT)
      .build(*devicePtr)
      .map(move_into{set3LayoutPtr})
      .map_error([](auto error) { multi_logger::get()->critical("Error creating set layout 3!"); });

    std::unique_ptr<descriptor_set_layout> set4LayoutPtr{};
    descriptor_set_layout_builder{}
      .uniform_buffer_dynamic(4, 1, VK_SHADER_STAGE_VERTEX_BIT)
      .build(*devicePtr)
      .map(move_into{set4LayoutPtr})
      .map_error([](auto error) { multi_logger::get()->critical("Error creating set layout 4!"); });

    std::unique_ptr<descriptor_pool> descriptorPoolPtr{};
    descriptor_pool_builder{}
      .add_layout(setLayoutPtr.get(), 3)
      .build(*devicePtr)
      .map(move_into{descriptorPoolPtr})
      .map_error([](auto error) { multi_logger::get()->critical("Error creating descriptor pool!"); exit(error); });
    platform::window_should_close shouldClose{};
    while (!(shouldClose = platform::glfw::poll_os(*surfacePtr))) {
      
    }
}