#ifndef VULKAN_VB_HELPERS
#define VULKAN_VB_HELPERS

#include <vector>
#include <cstdint>

#include "VulkanPrimary.hpp"
#include "VulkanMemoryHelpers.hpp"
#include "Vertex.hpp"
#include "Constants.hpp"

static void createIndexBuffer(VulkanPrimary& vk) {
    vk.indexBufferSize = sizeof(VertexIndex) * VERTEX_INDICES_PER_CUBE * MAX_CUBES;
    vk.indexStagingBufferSize = vk.indexBufferSize;

    createBuffer(vk, vk.indexStagingBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vk.indexStagingBuffer, vk.indexStagingBufferMemory);
    createBuffer(vk, vk.indexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vk.indexBuffer, vk.indexBufferMemory);
}

static void createVertexBuffer(VulkanPrimary& vk) {
    vk.vertexBufferSize = VERTEX_SIZE * MAX_CUBES * VERTICES_PER_CUBE;
    vk.vertexStagingBufferSize = vk.vertexBufferSize;

    // create staging buffer

    // create vertex buffer
    createBuffer(vk, vk.vertexStagingBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vk.vertexStagingBuffer, vk.vertexStagingBufferMemory);

    createBuffer(vk, vk.vertexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vk.vertexBuffer, vk.vertexBufferMemory);
}

#endif

