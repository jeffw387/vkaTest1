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
#include <optional>
#include <move_into.hpp>
#include <Logger.hpp>
#include <glm/glm.hpp>
#include <string>
#include <tiny_gltf.h>
#include <memory_allocator.hpp>
#include <cstring>

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

  VkQueue queue{};
  vkGetDeviceQueue(*devicePtr, queueFamily.familyIndex, 0, &queue);

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

  std::array<VkImage, 3> swapImages{};
  std::array<std::unique_ptr<image_view>, 3> swapViews{};
  std::array<std::unique_ptr<framebuffer>, 3> framebuffers{};

  auto getSwapImages = [&]() {
    uint32_t imageCount{};
    vkGetSwapchainImagesKHR(*devicePtr, *swapchainPtr, &imageCount, nullptr);
    if (imageCount != 3) {
      multi_logger::get()->critical(
          "Swap image count doesn't match. Expected: 3, Actual {}", imageCount);
      exit(1);
    }
    vkGetSwapchainImagesKHR(
        *devicePtr, *swapchainPtr, &imageCount, swapImages.data());
  };
  getSwapImages();

  std::unique_ptr<shader_module> shaderVertex3D{};
  shader_module_builder{}
      .build(*devicePtr, "triangle.vert.spv")
      .map(move_into{shaderVertex3D})
      .map_error([](auto error) {
        multi_logger::get()->critical("Error creating 3D vertex shader!");
        exit(1);
      });

  std::unique_ptr<shader_module> shaderFragment3D{};
  shader_module_builder{}
      .build(*devicePtr, "triangle.frag.spv")
      .map(move_into{shaderFragment3D})
      .map_error([](auto error) {
        multi_logger::get()->critical("Error creating 3D fragment shader!");
        exit(1);
      });

  std::unique_ptr<pipeline_layout> pipelineLayoutPtr{};
  pipeline_layout_builder{}
      .build(*devicePtr)
      .map(move_into{pipelineLayoutPtr})
      .map_error([](auto error) {
        multi_logger::get()->critical("Error creating pipeline layout!");
        exit(error);
      });

  auto subpass3D =
      subpass_builder{}
          .color_attachment(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
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
      .add_subpass(subpass3D)
      .build(*devicePtr)
      .map(move_into{renderPassPtr})
      .map_error([](auto error) {
        multi_logger::get()->critical("Error creating render pass!");
        exit(error);
      });

  VkViewport viewport{};
  viewport.width = 900;
  viewport.height = 900;
  viewport.minDepth = 0.f;
  viewport.maxDepth = 1.f;

  VkRect2D scissor{};
  scissor.extent.width = 900;
  scissor.extent.height = 900;

  for (int i{}; i < 3; ++i) {
    image_view_builder{}
        .image_source(swapImages[i])
        .image_format(VK_FORMAT_B8G8R8A8_UNORM)
        .array_layers(1)
        .image_aspect(VK_IMAGE_ASPECT_COLOR_BIT)
        .image_type(VK_IMAGE_TYPE_2D)
        .build(*devicePtr)
        .map(move_into{swapViews[i]});

    framebuffer_builder{}
        .render_pass(*renderPassPtr)
        .dimensions(900, 900)
        .attachments({*swapViews[i]})
        .build(*devicePtr)
        .map(move_into{framebuffers[i]});
  }

  std::unique_ptr<pipeline> pipeline3DPtr{};
  graphics_pipeline_builder{}
      .pipeline_layout(*pipelineLayoutPtr)
      .render_pass(*renderPassPtr, 0)
      .color_attachment(no_blend_attachment{})
      .shader_stage(shader_stage_builder{}
                        .vertex()
                        .shader_module(*shaderVertex3D, "main")
                        .build())
      .shader_stage(shader_stage_builder{}
                        .fragment()
                        .shader_module(*shaderFragment3D, "main")
                        .build())
      .vertex_binding<glm::vec3>(0, VK_VERTEX_INPUT_RATE_VERTEX)
      .vertex_binding<glm::vec4>(1, VK_VERTEX_INPUT_RATE_VERTEX)
      .vertex_attribute(0, 0, 3, 0)
      .vertex_attribute(1, 1, 4, 0)
      .viewport_scissor(viewport, scissor)
      .polygon_mode(VK_POLYGON_MODE_FILL)
      .cull_mode(VK_CULL_MODE_NONE)
      .primitive_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
      .winding_cw()
      .build(*devicePtr)
      .map(move_into{pipeline3DPtr})
      .map_error([](auto error) {
        multi_logger::get()->critical("Error creating 3D pipeline!");
        exit(error);
      });

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

  void* vertPtr{};
  vertexBuffer->map().map(move_into{vertPtr});
  std::array<glm::vec3, 3> vertices{glm::vec3{-0.5f, 0.5f, 0.f},
                                    glm::vec3{0.5f, 0.5f, 0.f},
                                    glm::vec3{0.f, -0.5f, 0.f}};
  std::memcpy(
      vertPtr, reinterpret_cast<void*>(vertices.data()), sizeof(glm::vec3) * 3);

  std::unique_ptr<buffer> vertexColorBuffer{};
  buffer_builder{}
      .cpu_to_gpu()
      .vertex_buffer()
      .queue_family_index(queueFamily.familyIndex)
      .size(sizeof(glm::vec4) * 3)
      .build(*allocatorPtr)
      .map(move_into{vertexColorBuffer})
      .map_error([](auto error) {
        multi_logger::get()->critical("Error creating vertex color buffer!");
        exit(error);
      });
  void* colorArrayPtr{};
  vertexColorBuffer->map().map(move_into{colorArrayPtr});
  std::array<glm::vec4, 3> vertexColors{glm::vec4{1.f, 0.f, 0.f, 1.f},
                                        glm::vec4{0.f, 1.f, 0.f, 1.f},
                                        glm::vec4{0.f, 0.f, 1.f, 1.f}};
  std::memcpy(
      colorArrayPtr,
      reinterpret_cast<void*>(vertexColors.data()),
      sizeof(glm::vec4) * 3);
  // /** round n down to nearest multiple of m */
  // auto round_down = [](VkDeviceSize n, VkDeviceSize m) {
  //     return (m == 0) ? n : (n / m) * m;
  // };

  // /** round n up to nearest multiple of m */
  // auto round_up = [](VkDeviceSize n, VkDeviceSize m) {
  //     return (m == 0) ? n : ((n + m - 1) / m) * m;
  // };

  // auto getAtomSize = [physicalDevice]() {
  //   VkPhysicalDeviceProperties props{};
  //   vkGetPhysicalDeviceProperties(physicalDevice, &props);
  //   return props.limits.nonCoherentAtomSize;
  // };

  vmaFlushAllocation(*allocatorPtr, *vertexBuffer, 0, VK_WHOLE_SIZE);
  vmaFlushAllocation(*allocatorPtr, *vertexColorBuffer, 0, VK_WHOLE_SIZE);

  std::unique_ptr<command_pool> cmdPoolPtr{};
  command_pool_builder{}
      .queue_family_index(queueFamily.familyIndex)
      .build(*devicePtr)
      .map(move_into{cmdPoolPtr})
      .map_error([](auto error) {
        multi_logger::get()->critical("Error creating command pool!");
        exit(error);
      });

  std::array<std::unique_ptr<command_buffer>, 3> cmdPtr{};
  for (auto& cmd : cmdPtr) {
    command_buffer_allocator{}
        .set_command_pool(cmdPoolPtr.get())
        .allocate(*devicePtr)
        .map(move_into{cmd})
        .map_error([](auto error) {
          multi_logger::get()->critical("Error allocating command buffer!");
          exit(error);
        });
  }

  {
    std::unique_ptr<command_buffer> utilityCmd{};
    command_buffer_allocator{}
        .set_command_pool(cmdPoolPtr.get())
        .allocate(*devicePtr)
        .map(move_into{utilityCmd})
        .map_error([](auto error) {
          multi_logger::get()->critical("Error allocating command buffer!");
          exit(error);
        });

    VkCommandBufferBeginInfo beginInfo{
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    vkBeginCommandBuffer(*utilityCmd, &beginInfo);
    VkMemoryBarrier vertexFlushBarrier{VK_STRUCTURE_TYPE_MEMORY_BARRIER};
    vertexFlushBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    vertexFlushBarrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
    vkCmdPipelineBarrier(
        *utilityCmd,
        VK_PIPELINE_STAGE_HOST_BIT,
        VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
        0,
        1,
        &vertexFlushBarrier,
        0,
        nullptr,
        0,
        nullptr);
    vkEndCommandBuffer(*utilityCmd);

    std::unique_ptr<fence> flushFence{};
    fence_builder{}.build(*devicePtr).map(move_into{flushFence});

    VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    VkCommandBuffer cmd = *utilityCmd;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;
    vkQueueSubmit(queue, 1, &submitInfo, *flushFence);

    VkFence vkFlushFence = *flushFence;
    vkWaitForFences(*devicePtr, 1, &vkFlushFence, true, ~uint64_t{});
  }

  std::unique_ptr<fence> imageReady{};
  fence_builder{}.build(*devicePtr).map(move_into{imageReady});

  std::unique_ptr<semaphore> drawFinished{};
  semaphore_builder{}.build(*devicePtr).map(move_into{drawFinished});

  std::array<std::unique_ptr<fence>, 3> commandBufferExecuted{};
  for (auto& cmdFence : commandBufferExecuted) {
    fence_builder{}.signaled().build(*devicePtr).map(move_into{cmdFence});
  }

  auto buildCmdBuffer = [&](uint32_t imageIndex) {
    VkCommandBuffer cmd = *cmdPtr[imageIndex];
    VkCommandBufferBeginInfo beginInfo{
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    vkBeginCommandBuffer(cmd, &beginInfo);

    VkImageMemoryBarrier swapBarrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    swapBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    swapBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    swapBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    swapBarrier.image = swapImages[imageIndex];
    swapBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    swapBarrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
    swapBarrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
    vkCmdPipelineBarrier(
        cmd,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        0,
        0,
        nullptr,
        0,
        nullptr,
        1,
        &swapBarrier);

    VkClearValue clearValue;
    clearValue.color = {0.f, 0.f, 0.f, 1.f};
    VkRenderPassBeginInfo renderBeginInfo{
        VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    renderBeginInfo.clearValueCount = 1;
    renderBeginInfo.pClearValues = &clearValue;
    renderBeginInfo.renderPass = *renderPassPtr;
    renderBeginInfo.renderArea = scissor;
    renderBeginInfo.framebuffer = *framebuffers[imageIndex];
    vkCmdBeginRenderPass(cmd, &renderBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipeline3DPtr);
    vkCmdBindVertexBuffers(
        cmd,
        0,
        2,
        std::vector<VkBuffer>{*vertexBuffer, *vertexColorBuffer}.data(),
        std::vector<VkDeviceSize>{0, 0}.data());

    vkCmdDraw(cmd, 3, 1, 0, 0);

    vkCmdEndRenderPass(cmd);
    vkEndCommandBuffer(cmd);
  };
  for (uint32_t i{}; i < 3; ++i) {
    buildCmdBuffer(i);
  }

  auto acquireImage = [&]() -> std::optional<uint32_t> {
    uint32_t imageIndex{};
    auto result = vkAcquireNextImageKHR(
        *devicePtr,
        *swapchainPtr,
        ~uint64_t{},
        VK_NULL_HANDLE,
        *imageReady,
        &imageIndex);
    if (result == VK_SUCCESS) {
      return imageIndex;
    }
    return {};
  };

  auto submitDraw = [&](uint32_t imageIndex) {
    VkFence imageReadyFence = *imageReady;
    std::vector<VkFence> fences = {*imageReady,
                                   *commandBufferExecuted[imageIndex]};
    vkWaitForFences(*devicePtr, 2, fences.data(), true, ~uint64_t{});
    vkResetFences(*devicePtr, 2, fences.data());

    VkSubmitInfo drawSubmit{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    drawSubmit.commandBufferCount = 1;
    VkCommandBuffer cmd = *cmdPtr[imageIndex];
    drawSubmit.pCommandBuffers = &cmd;
    drawSubmit.signalSemaphoreCount = 1;
    VkSemaphore sem4 = *drawFinished;
    drawSubmit.pSignalSemaphores = &sem4;

    vkQueueSubmit(queue, 1, &drawSubmit, *commandBufferExecuted[imageIndex]);
  };

  auto presentImage = [&](uint32_t imageIndex) {
    VkPresentInfoKHR presentInfo{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    presentInfo.swapchainCount = 1;
    VkSwapchainKHR swapchain = *swapchainPtr;
    presentInfo.pSwapchains = &swapchain;
    presentInfo.waitSemaphoreCount = 1;
    VkSemaphore sem4 = *drawFinished;
    presentInfo.pWaitSemaphores = &sem4;
    presentInfo.pImageIndices = &imageIndex;
    vkQueuePresentKHR(queue, &presentInfo);
  };

  platform::window_should_close shouldClose{};
  while (!(shouldClose = platform::glfw::poll_os(*surfacePtr))) {
    if (auto has_index = acquireImage()) {
      auto index = *has_index;
      submitDraw(index);
      presentImage(index);
    }
  }
  vkDeviceWaitIdle(*devicePtr);
}