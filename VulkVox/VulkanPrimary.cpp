

#include <glm/glm.hpp>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "VulkanPrimary.hpp"
#include "VulkanBootStrapper.hpp"
#include "VulkanMemoryHelpers.hpp"


    VulkanPrimary::VulkanPrimary() : 
        wo(nullptr)
    {
        mBootStrapper = std::make_unique<VulkanBootStrapper>(*this);
    }

    void VulkanPrimary::bootstrap() {
        mBootStrapper->run();
    }

    void VulkanPrimary::recreateSwapChain() {
        mBootStrapper->recreateSwapChain();
    }

    void VulkanPrimary::cleanup() {
        mBootStrapper->cleanup();
    }

    void VulkanPrimary::updateUniformBuffer(uint32_t currentImage) {
        UniformBufferObject ubo{};
        ubo.model = camera.model;
        ubo.view = glm::lookAt(
            camera.eye, // eye
            camera.center, // center (0,0,0) is center
            camera.up); // up, (0,1,0) is positive Y... fixme?

        ubo.proj = glm::perspective(
            glm::radians(camera.fovy),  // fovy
            swapChainExtent.width / (float)swapChainExtent.height, // aspect 
            camera.zNear, // zNear
            camera.zFar); // zFar

        ubo.proj[1][1] *= -1;

        void* data;
        vkMapMemory(getDevice(), uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(getDevice(), uniformBuffersMemory[currentImage]);
    }

    VkDevice& VulkanPrimary::getDevice() {
        return mDevice;
    }

    VkInstance& VulkanPrimary::getInstance() {
        return mInstance;
    }

    VkDescriptorSetLayout& VulkanPrimary::getDescriptorSetLayout() {
        return mDescriptorSetLayout;
    }

    void VulkanPrimary::setWorldObjects(WorldObjects* in) {
        wo = in;
    }

    // Updates the secondary command buffers
    // and puts them into the primary command buffer that's
    // submitted to the queue for rendering
    void VulkanPrimary::updateSecondaryCommandBuffers(VkFramebuffer& frameBuffer, VkCommandBuffer& primaryCommandBuffer, VkCommandBuffer& secondaryCommandBuffer, VkCommandBufferInheritanceInfo inheritanceInfo) {

        // todo later - for static objects. background, ui, etc

    }

    VkCommandBuffer VulkanPrimary::getSecondaryCommandBuffer() {
        if (availableSecondaryCommandBuffers.empty()) {
            createSecondaryCommandBuffer();
            return getSecondaryCommandBuffer();
        }
        VkCommandBuffer ret = availableSecondaryCommandBuffers.front();
        availableSecondaryCommandBuffers.pop();
        return ret;
    }

    void VulkanPrimary::createSecondaryCommandBuffer() {
        // create command buffer
        VkCommandBufferAllocateInfo secondaryAllocInfo{};
        secondaryAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        secondaryAllocInfo.commandPool = objectCommandPool;
        secondaryAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
        secondaryAllocInfo.commandBufferCount = 1;

        VkCommandBuffer newBuffer;
        if (vkAllocateCommandBuffers(getDevice(), &secondaryAllocInfo, &newBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate secondary/object command buffers!");
        }
        objectCommandBuffers.push_back(newBuffer);
        availableSecondaryCommandBuffers.push(newBuffer);
    }

    void VulkanPrimary::updateCommandBuffers(size_t index)
    {
        // Contains the list of secondary command buffers to be submitted
        std::vector<VkCommandBuffer> secondaryCommandSubmitBuffers;

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0; // Optional
        beginInfo.pInheritanceInfo = nullptr; // Optional

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { 0.0f, 0.0f, 0.0f, 0.1f };
        clearValues[1].depthStencil = { 0.1f, 0 };

        // Inheritance info for the secondary command buffers
        VkCommandBufferInheritanceInfo inheritanceInfo{};
        inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
        inheritanceInfo.pNext = nullptr;
        inheritanceInfo.renderPass = renderPass;
        // Secondary command buffer also use the currently active framebuffer
        inheritanceInfo.framebuffer = swapChainFramebuffers[index];

            auto secondaryBuf = getSecondaryCommandBuffer();
            VkCommandBufferBeginInfo commandBufferBeginInfo{};
            commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
            commandBufferBeginInfo.pInheritanceInfo = &inheritanceInfo;

            vkBeginCommandBuffer(secondaryBuf, &commandBufferBeginInfo);

            vkCmdBindPipeline(secondaryBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
            vkCmdBindDescriptorSets(secondaryBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[index], 0, nullptr);

            VkBuffer vertexBuffers[] = { vertexBuffer };
            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(secondaryBuf, 0, 1, vertexBuffers, offsets);

            vkCmdBindIndexBuffer(secondaryBuf, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

            int offset=0;
            size_t numVertices = wo->size() * VERTEX_INDICES_PER_CUBE;
            vkCmdDrawIndexed(secondaryBuf, numVertices, 1, 0 + offset, 0, 0);
            
            vkEndCommandBuffer(secondaryBuf);

            secondaryCommandSubmitBuffers.push_back(secondaryBuf);


        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = swapChainFramebuffers[index];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = swapChainExtent;
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        // Set target frame buffer

        if (vkBeginCommandBuffer(commandBuffers[index], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        vkCmdBeginRenderPass(commandBuffers[index], &renderPassInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

        vkCmdExecuteCommands(commandBuffers[index], secondaryCommandSubmitBuffers.size(), secondaryCommandSubmitBuffers.data());

        vkCmdEndRenderPass(commandBuffers[index]);

        if (vkEndCommandBuffer(commandBuffers[index]) != VK_SUCCESS) {
            throw std::runtime_error("failed to record primary command buffer!");
        }
    }

    void VulkanPrimary::drawFrame() {
        // wait for frame to be finished
        vkWaitForFences(getDevice(), 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        while (!availableSecondaryCommandBuffers.empty()) {
            availableSecondaryCommandBuffers.pop();
        }
        for (auto buf : objectCommandBuffers) {
            availableSecondaryCommandBuffers.push(buf);
        }

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(getDevice(), swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }
        // Check if a previous frame is using this image (i.e. there is its fence to wait on)
        if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
            vkWaitForFences(getDevice(), 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
        }
        // Mark the image as now being in use by this frame
        imagesInFlight[imageIndex] = inFlightFences[currentFrame];

        // %% vertex buffer
        
        void* data;

        cubeVertices.resize(wo->size() * VERTICES_PER_CUBE);
        cubeVertexIndices.resize(wo->size() * VERTEX_INDICES_PER_CUBE);
        
        for (size_t kCube = 0; kCube < wo->size(); ++kCube){
            wo->cubes[kCube].copyVertices(cubeVertices, cubeVertexIndices, kCube);
        }

        assert(vertexBufferSize);

        // map the buffer memory into CPU accessible memory

        VkDeviceSize usedVertexBufferSize = VERTEX_SIZE * wo->size() * VERTICES_PER_CUBE;
        vkMapMemory(getDevice(), vertexStagingBufferMemory, 0, usedVertexBufferSize, 0, &data);
        memcpy(data, cubeVertices.data(), VERTEX_SIZE * cubeVertices.size());
        vkUnmapMemory(getDevice(), vertexStagingBufferMemory);
        copyBuffer(*this, vertexStagingBuffer, vertexBuffer, usedVertexBufferSize);
        

        // %%
        // now, index buffer

        VkDeviceSize usedIndexBufferSize = sizeof(VertexIndex) * wo->size() * VERTEX_INDICES_PER_CUBE;
        assert(usedIndexBufferSize);

        vkMapMemory(getDevice(), indexStagingBufferMemory, 0, usedIndexBufferSize, 0, &data);
        memcpy(data, cubeVertexIndices.data(), usedIndexBufferSize);
        vkUnmapMemory(getDevice(), indexStagingBufferMemory);

        copyBuffer(*this, indexStagingBuffer, indexBuffer, usedIndexBufferSize);

        for (size_t i = 0; i < swapChainFramebuffers.size(); i++) {
            updateCommandBuffers(i);
        }

        // %%

        updateUniformBuffer(imageIndex);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[imageIndex];
        VkSemaphore signalSemaphores[] = { imageAvailableSemaphores[currentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        vkResetFences(getDevice(), 1, &inFlightFences[currentFrame]);

        const VkResult submitResult = vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]);
        if (VK_SUCCESS != submitResult) {
            std::cout << "vk error code: " << submitResult << std::endl;
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        VkSwapchainKHR swapChains[] = { swapChain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr; // Optional
        result = vkQueuePresentKHR(presentQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
            framebufferResized = false;
            recreateSwapChain();
        }
        else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }
        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }
