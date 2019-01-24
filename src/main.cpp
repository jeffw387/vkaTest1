#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

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
#include <render_pass.hpp>
#include <pipeline_layout.hpp>
#include <pipeline.hpp>
#include <memory>
#include <array>
#include <vector>
#include <move_into.hpp>
#include <Logger.hpp>
#include <glm/glm.hpp>
#include <string>
#include <tiny_gltf.h>
#include <memory_allocator.hpp>

using namespace vka;
int main() {
  platform::glfw::init();
  std::unique_ptr<instance> instancePtr{};
  instance_builder{}
      .add_extensions(platform::glfw::get_required_instance_extensions())
      .add_layer(standard_validation)
      .build()
      .map(move_into{instancePtr})
      .map_error([](auto error) {
        multi_logger::get()->critical("Error creating instance!");
        exit(error);
      });

  std::unique_ptr<surface> surfacePtr{};
  surface_builder{}
      .width(900)
      .height(900)
      .title("vkaTest1")
      .build(*instancePtr)
      .map(move_into{surfacePtr})
      .map_error([](auto error) {
        multi_logger::get()->critical("Error creating surface!");
        exit(1);
      });

  VkPhysicalDevice physicalDevice{};
  physical_device_selector{}
      .select(*instancePtr)
      .map(move_into{physicalDevice})
      .map_error([](auto error) {
        multi_logger::get()->critical("Error selecting physical device!");
        exit(1);
      });

  queue_family queueFamily{};
  queue_family_builder{}
      .queue(1.f)
      .graphics_support()
      .present_support(*surfacePtr)
      .build(physicalDevice)
      .map(move_into{queueFamily})
      .map_error([](auto error) {
        multi_logger::get()->critical("Error selecting queue family!");
        exit(1);
      });

  std::unique_ptr<device> devicePtr{};
  device_builder{}
      .physical_device(physicalDevice)
      .extension(swapchain_extension)
      .add_queue_family(queueFamily)
      .build(*instancePtr)
      .map(move_into{devicePtr})
      .map_error([](auto error) {
        multi_logger::get()->critical("Error creating device!");
        exit(error);
      });

  std::unique_ptr<swapchain> swapchainPtr{};
  swapchain_builder{}
      .image_count(3)
      .present_mode(VK_PRESENT_MODE_FIFO_KHR)
      .queue_family_index(queueFamily.familyIndex)
      .build(physicalDevice, *surfacePtr, *devicePtr)
      .map(move_into{swapchainPtr})
      .map_error([](auto error) {
        multi_logger::get()->critical("Error creating swapchain!");
        exit(error);
      });

  std::unique_ptr<descriptor_set_layout> set0LayoutPtr{};
  descriptor_set_layout_builder{}
      .storage_buffer(0, 1, VK_SHADER_STAGE_FRAGMENT_BIT)
      .build(*devicePtr)
      .map(move_into{set0LayoutPtr})
      .map_error([](auto error) {
        multi_logger::get()->critical("Error creating set layout 0!");
        exit(error);
      });

  std::unique_ptr<descriptor_set_layout> set1LayoutPtr{};
  descriptor_set_layout_builder{}
      .storage_buffer(1, 1, VK_SHADER_STAGE_FRAGMENT_BIT)
      .build(*devicePtr)
      .map(move_into{set1LayoutPtr})
      .map_error([](auto error) {
        multi_logger::get()->critical("Error creating set layout 1!");
        exit(error);
      });

  std::unique_ptr<descriptor_set_layout> set2LayoutPtr{};
  descriptor_set_layout_builder{}
      .uniform_buffer(2, 1, VK_SHADER_STAGE_FRAGMENT_BIT)
      .build(*devicePtr)
      .map(move_into{set2LayoutPtr})
      .map_error([](auto error) {
        multi_logger::get()->critical("Error creating set layout 2!");
        exit(error);
      });

  std::unique_ptr<descriptor_set_layout> set3LayoutPtr{};
  descriptor_set_layout_builder{}
      .uniform_buffer(3, 1, VK_SHADER_STAGE_VERTEX_BIT)
      .build(*devicePtr)
      .map(move_into{set3LayoutPtr})
      .map_error([](auto error) {
        multi_logger::get()->critical("Error creating set layout 3!");
        exit(error);
      });

  std::unique_ptr<descriptor_set_layout> set4LayoutPtr{};
  descriptor_set_layout_builder{}
      .uniform_buffer_dynamic(4, 1, VK_SHADER_STAGE_VERTEX_BIT)
      .build(*devicePtr)
      .map(move_into{set4LayoutPtr})
      .map_error([](auto error) {
        multi_logger::get()->critical("Error creating set layout 4!");
        exit(error);
      });

  std::unique_ptr<descriptor_pool> descriptorPoolPtr{};
  descriptor_pool_builder{}
      .add_layout(set0LayoutPtr.get(), 3)
      .add_layout(set1LayoutPtr.get(), 3)
      .add_layout(set2LayoutPtr.get(), 3)
      .add_layout(set3LayoutPtr.get(), 3)
      .add_layout(set4LayoutPtr.get(), 3)
      .build(*devicePtr)
      .map(move_into{descriptorPoolPtr})
      .map_error([](auto error) {
        multi_logger::get()->critical("Error creating descriptor pool!");
        exit(error);
      });

  std::array<std::unique_ptr<descriptor_set>, 3> set0Array;
  descriptor_set_allocator set0Allocator{};
  set0Allocator.set_layout(set0LayoutPtr.get());
  for (auto& set : set0Array) {
    set0Allocator.allocate(*devicePtr, *descriptorPoolPtr)
        .map(move_into{set})
        .map_error([](auto error) {
          multi_logger::get()->critical("Error allocating descriptor set 0!");
          exit(error);
        });
  }
  std::array<std::unique_ptr<descriptor_set>, 3> set1Array;
  descriptor_set_allocator set1Allocator{};
  set1Allocator.set_layout(set1LayoutPtr.get());
  for (auto& set : set1Array) {
    set1Allocator.allocate(*devicePtr, *descriptorPoolPtr)
        .map(move_into{set})
        .map_error([](auto error) {
          multi_logger::get()->critical("Error allocating descriptor set 1!");
          exit(error);
        });
  }
  std::array<std::unique_ptr<descriptor_set>, 3> set2Array;
  descriptor_set_allocator set2Allocator{};
  set2Allocator.set_layout(set2LayoutPtr.get());
  for (auto& set : set2Array) {
    set2Allocator.allocate(*devicePtr, *descriptorPoolPtr)
        .map(move_into{set})
        .map_error([](auto error) {
          multi_logger::get()->critical("Error allocating descriptor set 2!");
          exit(error);
        });
  }
  std::array<std::unique_ptr<descriptor_set>, 3> set3Array;
  descriptor_set_allocator set3Allocator{};
  set3Allocator.set_layout(set3LayoutPtr.get());
  for (auto& set : set3Array) {
    set3Allocator.allocate(*devicePtr, *descriptorPoolPtr)
        .map(move_into{set})
        .map_error([](auto error) {
          multi_logger::get()->critical("Error allocating descriptor set 3!");
          exit(error);
        });
  }
  std::array<std::unique_ptr<descriptor_set>, 3> set4Array;
  descriptor_set_allocator set4Allocator{};
  set4Allocator.set_layout(set4LayoutPtr.get());
  for (auto& set : set4Array) {
    set4Allocator.allocate(*devicePtr, *descriptorPoolPtr)
        .map(move_into{set})
        .map_error([](auto error) {
          multi_logger::get()->critical("Error allocating descriptor set 4!");
          exit(error);
        });
  }

  std::unique_ptr<shader_module> shaderVertex3D{};
  shader_module_builder{}
      .build(*devicePtr, "../3d.vert.spv")
      .map(move_into{shaderVertex3D})
      .map_error([](auto error) {
        multi_logger::get()->critical("Error creating 3D vertex shader!");
        exit(1);
      });

  std::unique_ptr<shader_module> shaderFragment3D{};
  shader_module_builder{}
      .build(*devicePtr, "../3d.frag.spv")
      .map(move_into{shaderFragment3D})
      .map_error([](auto error) {
        multi_logger::get()->critical("Error creating 3D fragment shader!");
        exit(1);
      });

  std::unique_ptr<pipeline_layout> pipelineLayoutPtr{};
  pipeline_layout_builder{}
      .set_layout(*set0LayoutPtr)
      .set_layout(*set1LayoutPtr)
      .set_layout(*set2LayoutPtr)
      .set_layout(*set3LayoutPtr)
      .set_layout(*set4LayoutPtr)
      .push_range(VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(uint32_t))
      .build(*devicePtr)
      .map(move_into{pipelineLayoutPtr})
      .map_error([](auto error) {
        multi_logger::get()->critical("Error creating pipeline layout!");
        exit(error);
      });

  auto subpass3D =
      subpass_builder{}
          .color_attachment(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
          .depth_attachment(1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
          .build();

  std::unique_ptr<render_pass> renderPassPtr{};
  render_pass_builder{}
      .add_attachment(
          attachment_builder{}
              .initial_layout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
              .final_layout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
              .format(VK_FORMAT_B8G8R8A8_UNORM)
              .loadOp(VK_ATTACHMENT_LOAD_OP_CLEAR)
              .storeOp(VK_ATTACHMENT_STORE_OP_STORE)
              .build())
      .add_attachment(
          attachment_builder{}
              .initial_layout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
              .final_layout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
              .format(VK_FORMAT_D32_SFLOAT)
              .loadOp(VK_ATTACHMENT_LOAD_OP_CLEAR)
              .storeOp(VK_ATTACHMENT_STORE_OP_DONT_CARE)
              .build())
      .add_subpass(subpass3D)
      .build(*devicePtr)
      .map(move_into{renderPassPtr})
      .map_error([](auto error) {
        multi_logger::get()->critical("Error creating render pass!");
        exit(error);
      });

  std::unique_ptr<pipeline> pipeline3DPtr{};
  graphics_pipeline_builder{}
      .pipeline_layout(*pipelineLayoutPtr)
      .render_pass(*renderPassPtr, 0)
      .color_attachment(no_blend_attachment{})
      .depth_test()
      .depth_write()
      .shader_stage(shader_stage_builder{}
                        .vertex()
                        .shader_module(*shaderVertex3D, "main")
                        .build())
      .shader_stage(shader_stage_builder{}
                        .fragment()
                        .shader_module(*shaderFragment3D, "main")
                        .build())
      .vertex_binding<glm::vec3>(0, VK_VERTEX_INPUT_RATE_VERTEX)
      .vertex_binding<glm::vec3>(1, VK_VERTEX_INPUT_RATE_VERTEX)
      .vertex_attribute(0, 0, 3, 0)
      .vertex_attribute(1, 1, 3, sizeof(glm::vec3))
      .viewport_scissor({}, {})
      .polygon_mode(VK_POLYGON_MODE_FILL)
      .cull_mode(VK_CULL_MODE_BACK_BIT)
      .primitive_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
      .dynamic_state(VK_DYNAMIC_STATE_VIEWPORT)
      .dynamic_state(VK_DYNAMIC_STATE_SCISSOR)
      .build(*devicePtr)
      .map(move_into{pipeline3DPtr})
      .map_error([](auto error) {
        multi_logger::get()->critical("Error creating 3D pipeline!");
        exit(error);
      });

  auto loadModelFromFile = [](auto filePath) -> tinygltf::Model {
    tinygltf::TinyGLTF loader{};
    tinygltf::Model model{};
    std::string err{};
    std::string warn{};
    auto result = loader.LoadASCIIFromFile(&model, &err, &warn, filePath);
    if (!warn.empty()) {
      multi_logger::get()->warn("tinygltf: {}", warn);
    }
    if (!err.empty()) {
      multi_logger::get()->error("tinygltf: {}", err);
    }
    if (result) {
      return model;
    } else {
      exit(1);
    }
  };
  tinygltf::Model terrainModel{loadModelFromFile("models/terrain.gltf")};

  std::unique_ptr<allocator> allocatorPtr{};
  allocator_builder{}
      .physical_device(physicalDevice)
      .device(*devicePtr)
      .preferred_block_size(4096)
      .build()
      .map(move_into{allocatorPtr})
      .map_error([](auto error) {
        multi_logger::get()->critical("Error creating device allocator!");
        exit(error);
      });

  std::unique_ptr<buffer> materialsBuffer{};
  std::unique_ptr<buffer> dynamicLightsBuffer{};
  std::unique_ptr<buffer> lightDataBuffer{};
  std::unique_ptr<buffer> cameraBuffer{};
  std::unique_ptr<buffer> instanceBuffer{};

  auto hostStorageBuilder =
      buffer_builder{}.cpu_to_gpu().storage_buffer().queue_family_index(
          queueFamily.familyIndex);

  hostStorageBuilder.size(sizeof(glm::vec4))
      .build(*allocatorPtr)
      .map(move_into{materialsBuffer})
      .map_error([](auto error) {
        multi_logger::get()->critical("Error creating materials buffer!");
        exit(error);
      });

  hostStorageBuilder.size(sizeof(glm::vec4) * 2)
      .build(*allocatorPtr)
      .map(move_into{dynamicLightsBuffer})
      .map_error([](auto error) {
        multi_logger::get()->critical("Error creating dynamic lights buffer!");
        exit(error);
      });

  std::unique_ptr<buffer> vertexBuffer{};
  buffer_builder{}
      .cpu_to_gpu()
      .vertex_buffer()
      .queue_family_index(queueFamily.familyIndex)
      .size(sizeof(glm::vec3) * 3)
      .build(*allocatorPtr)
      .map(move_into{vertexBuffer})
      .map_error([](auto error) {
        multi_logger::get()->critical("Error creating vertex buffer!");
        exit(error);
      });

  platform::window_should_close shouldClose{};
  while (!(shouldClose = platform::glfw::poll_os(*surfacePtr))) {
  }
}