#include "VulkanInitializers.hpp"

VkMemoryAllocateInfo vks::initializers::memoryAllocateInfo()
{
	VkMemoryAllocateInfo memAllocInfo{};
	memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	return memAllocInfo;
}

VkMappedMemoryRange vks::initializers::mappedMemoryRange()
{
	VkMappedMemoryRange mappedMemoryRange{};
	mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	return mappedMemoryRange;
}

VkCommandBufferAllocateInfo vks::initializers::commandBufferAllocateInfo(VkCommandPool commandPool, VkCommandBufferLevel level, uint32_t bufferCount)
{
	VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandPool = commandPool;
	commandBufferAllocateInfo.level = level;
	commandBufferAllocateInfo.commandBufferCount = bufferCount;
	return commandBufferAllocateInfo;
}

VkCommandPoolCreateInfo vks::initializers::commandPoolCreateInfo()
{
	VkCommandPoolCreateInfo cmdPoolCreateInfo{};
	cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	return cmdPoolCreateInfo;
}

VkCommandBufferBeginInfo vks::initializers::commandBufferBeginInfo()
{
	VkCommandBufferBeginInfo cmdBufferBeginInfo{};
	cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	return cmdBufferBeginInfo;
}

VkCommandBufferInheritanceInfo vks::initializers::commandBufferInheritanceInfo()
{
	VkCommandBufferInheritanceInfo cmdBufferInheritanceInfo{};
	cmdBufferInheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
	return cmdBufferInheritanceInfo;
}

VkRenderPassBeginInfo vks::initializers::renderPassBeginInfo()
{
	VkRenderPassBeginInfo renderPassBeginInfo{};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	return renderPassBeginInfo;
}

VkRenderPassCreateInfo vks::initializers::renderPassCreateInfo()
{
	VkRenderPassCreateInfo renderPassCreateInfo{};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	return renderPassCreateInfo;
}

/** @brief Initialize an image memory barrier with no image transfer ownership */
VkImageMemoryBarrier vks::initializers::imageMemoryBarrier()
{
	VkImageMemoryBarrier imageMemoryBarrier{};
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	return imageMemoryBarrier;
}

/** @brief Initialize a buffer memory barrier with no image transfer ownership */
VkBufferMemoryBarrier vks::initializers::bufferMemoryBarrier()
{
	VkBufferMemoryBarrier bufferMemoryBarrier{};
	bufferMemoryBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	bufferMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	bufferMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	return bufferMemoryBarrier;
}

VkMemoryBarrier vks::initializers::memoryBarrier()
{
	VkMemoryBarrier memoryBarrier{};
	memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	return memoryBarrier;
}

VkImageCreateInfo vks::initializers::imageCreateInfo()
{
	VkImageCreateInfo imageCreateInfo{};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	return imageCreateInfo;
}

VkSamplerCreateInfo vks::initializers::samplerCreateInfo()
{
	VkSamplerCreateInfo samplerCreateInfo{};
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.maxAnisotropy = 1.0f;
	return samplerCreateInfo;
}

VkImageViewCreateInfo vks::initializers::imageViewCreateInfo()
{
	VkImageViewCreateInfo imageViewCreateInfo{};
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	return imageViewCreateInfo;
}

VkFramebufferCreateInfo vks::initializers::framebufferCreateInfo()
{
	VkFramebufferCreateInfo framebufferCreateInfo{};
	framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	return framebufferCreateInfo;
}

VkSemaphoreCreateInfo vks::initializers::semaphoreCreateInfo()
{
	VkSemaphoreCreateInfo semaphoreCreateInfo{};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	return semaphoreCreateInfo;
}

VkFenceCreateInfo vks::initializers::fenceCreateInfo(VkFenceCreateFlags flags)
{
	VkFenceCreateInfo fenceCreateInfo{};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = flags;
	return fenceCreateInfo;
}

VkEventCreateInfo vks::initializers::eventCreateInfo()
{
	VkEventCreateInfo eventCreateInfo{};
	eventCreateInfo.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
	return eventCreateInfo;
}

VkSubmitInfo vks::initializers::submitInfo()
{
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	return submitInfo;
}

VkViewport vks::initializers::viewport(float width, float height, float minDepth, float maxDepth)
{
	VkViewport viewport{};
	viewport.width = width;
	viewport.height = height;
	viewport.minDepth = minDepth;
	viewport.maxDepth = maxDepth;
	return viewport;
}


VkRect2D vks::initializers::rect2D(int32_t width, int32_t height, int32_t offsetX, int32_t offsetY)
{
	VkRect2D rect2D{};
	rect2D.extent.width = width;
	rect2D.extent.height = height;
	rect2D.offset.x = offsetX;
	rect2D.offset.y = offsetY;
	return rect2D;
}

VkRect2D vks::initializers::rect2D(VkExtent2D extent, int32_t offsetX, int32_t offsetY)
{
	VkRect2D rect2D{};
	rect2D.extent = extent;
	rect2D.offset.x = offsetX;
	rect2D.offset.y = offsetY;
	return rect2D;
}

VkBufferCreateInfo vks::initializers::bufferCreateInfo()
{
	VkBufferCreateInfo bufCreateInfo{};
	bufCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	return bufCreateInfo;
}

VkDescriptorPoolCreateInfo vks::initializers::descriptorPoolCreateInfo(uint32_t poolSizeCount, VkDescriptorPoolSize* pPoolSizes, uint32_t maxSets)
{
	VkDescriptorPoolCreateInfo descriptorPoolInfo{};
	descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolInfo.poolSizeCount = poolSizeCount;
	descriptorPoolInfo.pPoolSizes = pPoolSizes;
	descriptorPoolInfo.maxSets = maxSets;
	return descriptorPoolInfo;
}

VkDescriptorPoolCreateInfo vks::initializers::descriptorPoolCreateInfo(const std::vector<VkDescriptorPoolSize>& poolSizes, uint32_t maxSets)
{
	VkDescriptorPoolCreateInfo descriptorPoolInfo{};
	descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	descriptorPoolInfo.pPoolSizes = poolSizes.data();
	descriptorPoolInfo.maxSets = maxSets;
	return descriptorPoolInfo;
}

VkDescriptorPoolSize vks::initializers::descriptorPoolSize(VkDescriptorType type, uint32_t descriptorCount)
{
	VkDescriptorPoolSize descriptorPoolSize{};
	descriptorPoolSize.type = type;
	descriptorPoolSize.descriptorCount = descriptorCount;
	return descriptorPoolSize;
}

VkDescriptorSetLayoutBinding vks::initializers::descriptorSetLayoutBinding(VkDescriptorType type, VkShaderStageFlags stageFlags, uint32_t binding, uint32_t descriptorCount)
{
	VkDescriptorSetLayoutBinding setLayoutBinding{};
	setLayoutBinding.descriptorType = type;
	setLayoutBinding.stageFlags = stageFlags;
	setLayoutBinding.binding = binding;
	setLayoutBinding.descriptorCount = descriptorCount;
	return setLayoutBinding;
}

VkDescriptorSetLayoutCreateInfo vks::initializers::descriptorSetLayoutCreateInfo(const VkDescriptorSetLayoutBinding* pBindings, uint32_t bindingCount)
{
	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
	descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutCreateInfo.pBindings = pBindings;
	descriptorSetLayoutCreateInfo.bindingCount = bindingCount;
	return descriptorSetLayoutCreateInfo;
}

VkDescriptorSetLayoutCreateInfo vks::initializers::descriptorSetLayoutCreateInfo(const std::vector<VkDescriptorSetLayoutBinding>& bindings)
{
	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
	descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutCreateInfo.pBindings = bindings.data();
	descriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	return descriptorSetLayoutCreateInfo;
}

VkPipelineLayoutCreateInfo vks::initializers::pipelineLayoutCreateInfo(const VkDescriptorSetLayout* pSetLayouts, uint32_t setLayoutCount)
{
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = setLayoutCount;
	pipelineLayoutCreateInfo.pSetLayouts = pSetLayouts;
	return pipelineLayoutCreateInfo;
}



VkDescriptorSetAllocateInfo vks::initializers::descriptorSetAllocateInfo(VkDescriptorPool descriptorPool, const VkDescriptorSetLayout* pSetLayouts, uint32_t descriptorSetCount)
{
	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.descriptorPool = descriptorPool;
	descriptorSetAllocateInfo.pSetLayouts = pSetLayouts;
	descriptorSetAllocateInfo.descriptorSetCount = descriptorSetCount;
	return descriptorSetAllocateInfo;
}

VkDescriptorImageInfo vks::initializers::descriptorImageInfo(VkSampler sampler, VkImageView imageView, VkImageLayout imageLayout)
{
	VkDescriptorImageInfo descriptorImageInfo{};
	descriptorImageInfo.sampler = sampler;
	descriptorImageInfo.imageView = imageView;
	descriptorImageInfo.imageLayout = imageLayout;
	return descriptorImageInfo;
}

VkDescriptorBufferInfo vks::initializers::descriptorBufferInfo(VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range) 
{
	VkDescriptorBufferInfo descriptorBufferInfo{};
	descriptorBufferInfo.buffer = buffer;
	descriptorBufferInfo.offset = offset;
	descriptorBufferInfo.range = range;	
	return descriptorBufferInfo;
}

VkWriteDescriptorSet vks::initializers::writeDescriptorSet(VkDescriptorSet dstSet, VkDescriptorType type, uint32_t binding, VkDescriptorBufferInfo* bufferInfo, uint32_t descriptorCount)
{
	VkWriteDescriptorSet writeDescriptorSet{};
	writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet.dstSet = dstSet;
	writeDescriptorSet.descriptorType = type;
	writeDescriptorSet.dstBinding = binding;
	writeDescriptorSet.pBufferInfo = bufferInfo;
	writeDescriptorSet.descriptorCount = descriptorCount;
	return writeDescriptorSet;
}

VkWriteDescriptorSet vks::initializers::writeDescriptorSet(VkDescriptorSet dstSet, VkDescriptorType type, uint32_t binding, VkDescriptorImageInfo* imageInfo, uint32_t descriptorCount)
{
	VkWriteDescriptorSet writeDescriptorSet{};
	writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet.dstSet = dstSet;
	writeDescriptorSet.descriptorType = type;
	writeDescriptorSet.dstBinding = binding;
	writeDescriptorSet.pImageInfo = imageInfo;
	writeDescriptorSet.descriptorCount = descriptorCount;
	return writeDescriptorSet;
}

VkVertexInputBindingDescription vks::initializers::vertexInputBindingDescription(uint32_t binding, uint32_t stride, VkVertexInputRate inputRate)
{
	VkVertexInputBindingDescription vInputBindDescription{};
	vInputBindDescription.binding = binding;
	vInputBindDescription.stride = stride;
	vInputBindDescription.inputRate = inputRate;
	return vInputBindDescription;
}

VkVertexInputAttributeDescription vks::initializers::vertexInputAttributeDescription(uint32_t binding, uint32_t location, VkFormat format, uint32_t offset)
{
	VkVertexInputAttributeDescription vInputAttribDescription{};
	vInputAttribDescription.location = location;
	vInputAttribDescription.binding = binding;
	vInputAttribDescription.format = format;
	vInputAttribDescription.offset = offset;
	return vInputAttribDescription;
}

VkPipelineVertexInputStateCreateInfo vks::initializers::pipelineVertexInputStateCreateInfo()
{
	VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo{};
	pipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	return pipelineVertexInputStateCreateInfo;
}

VkPipelineInputAssemblyStateCreateInfo vks::initializers::pipelineInputAssemblyStateCreateInfo(VkPrimitiveTopology topology, VkPipelineInputAssemblyStateCreateFlags flags, VkBool32 primitiveRestartEnable)
{
	VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo{};
	pipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	pipelineInputAssemblyStateCreateInfo.topology = topology;
	pipelineInputAssemblyStateCreateInfo.flags = flags;
	pipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = primitiveRestartEnable;
	return pipelineInputAssemblyStateCreateInfo;
}

VkPipelineRasterizationStateCreateInfo vks::initializers::pipelineRasterizationStateCreateInfo(VkPolygonMode polygonMode, VkCullModeFlags cullMode, VkFrontFace frontFace, VkPipelineRasterizationStateCreateFlags flags)
{
	VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo{};
	pipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	pipelineRasterizationStateCreateInfo.polygonMode = polygonMode;
	pipelineRasterizationStateCreateInfo.cullMode = cullMode;
	pipelineRasterizationStateCreateInfo.frontFace = frontFace;
	pipelineRasterizationStateCreateInfo.flags = flags;
	pipelineRasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
	pipelineRasterizationStateCreateInfo.lineWidth = 1.0f;
	return pipelineRasterizationStateCreateInfo;
}

VkPipelineColorBlendAttachmentState vks::initializers::pipelineColorBlendAttachmentState(VkColorComponentFlags colorWriteMask, VkBool32 blendEnable)
{
	VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState{};
	pipelineColorBlendAttachmentState.colorWriteMask = colorWriteMask;
	pipelineColorBlendAttachmentState.blendEnable = blendEnable;
	return pipelineColorBlendAttachmentState;
}

VkPipelineColorBlendStateCreateInfo vks::initializers::pipelineColorBlendStateCreateInfo(uint32_t attachmentCount, const VkPipelineColorBlendAttachmentState* pAttachments)
{
	VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo{};
	pipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	pipelineColorBlendStateCreateInfo.attachmentCount = attachmentCount;
	pipelineColorBlendStateCreateInfo.pAttachments = pAttachments;
	return pipelineColorBlendStateCreateInfo;
}

VkPipelineDepthStencilStateCreateInfo vks::initializers::pipelineDepthStencilStateCreateInfo(VkBool32 depthTestEnable, VkBool32 depthWriteEnable, VkCompareOp depthCompareOp)
{
	VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo{};
	pipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	pipelineDepthStencilStateCreateInfo.depthTestEnable = depthTestEnable;
	pipelineDepthStencilStateCreateInfo.depthWriteEnable = depthWriteEnable;
	pipelineDepthStencilStateCreateInfo.depthCompareOp = depthCompareOp;
	pipelineDepthStencilStateCreateInfo.front = pipelineDepthStencilStateCreateInfo.back;
	pipelineDepthStencilStateCreateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
	return pipelineDepthStencilStateCreateInfo;
}

VkPipelineViewportStateCreateInfo vks::initializers::pipelineViewportStateCreateInfo(uint32_t viewportCount, uint32_t scissorCount, VkPipelineViewportStateCreateFlags flags)
{
	VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo{};
	pipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	pipelineViewportStateCreateInfo.viewportCount = viewportCount;
	pipelineViewportStateCreateInfo.scissorCount = scissorCount;
	pipelineViewportStateCreateInfo.flags = flags;
	return pipelineViewportStateCreateInfo;
}

VkPipelineMultisampleStateCreateInfo vks::initializers::pipelineMultisampleStateCreateInfo(VkSampleCountFlagBits rasterizationSamples, VkPipelineMultisampleStateCreateFlags flags)
{
	VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo{};
	pipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	pipelineMultisampleStateCreateInfo.rasterizationSamples = rasterizationSamples;
	pipelineMultisampleStateCreateInfo.flags = flags;
	return pipelineMultisampleStateCreateInfo;
}

VkPipelineDynamicStateCreateInfo vks::initializers::pipelineDynamicStateCreateInfo(const VkDynamicState* pDynamicStates, uint32_t dynamicStateCount, VkPipelineDynamicStateCreateFlags flags)
{
	VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo{};
	pipelineDynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	pipelineDynamicStateCreateInfo.pDynamicStates = pDynamicStates;
	pipelineDynamicStateCreateInfo.dynamicStateCount = dynamicStateCount;
	pipelineDynamicStateCreateInfo.flags = flags;
	return pipelineDynamicStateCreateInfo;
}

VkPipelineDynamicStateCreateInfo vks::initializers::pipelineDynamicStateCreateInfo(const std::vector<VkDynamicState>& pDynamicStates, VkPipelineDynamicStateCreateFlags flags)
{
	VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo{};
	pipelineDynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	pipelineDynamicStateCreateInfo.pDynamicStates = pDynamicStates.data();
	pipelineDynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(pDynamicStates.size());
	pipelineDynamicStateCreateInfo.flags = flags;
	return pipelineDynamicStateCreateInfo;
}

VkPipelineTessellationStateCreateInfo vks::initializers::pipelineTessellationStateCreateInfo(uint32_t patchControlPoints)
{
	VkPipelineTessellationStateCreateInfo pipelineTessellationStateCreateInfo{};
	pipelineTessellationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
	pipelineTessellationStateCreateInfo.patchControlPoints = patchControlPoints;
	return pipelineTessellationStateCreateInfo;
}

VkGraphicsPipelineCreateInfo vks::initializers::pipelineCreateInfo(VkPipelineLayout layout, VkRenderPass renderPass, VkPipelineCreateFlags flags)
{
	VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.layout = layout;
	pipelineCreateInfo.renderPass = renderPass;
	pipelineCreateInfo.flags = flags;
	pipelineCreateInfo.basePipelineIndex = -1;
	pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	return pipelineCreateInfo;
}

VkGraphicsPipelineCreateInfo vks::initializers::pipelineCreateInfo()
{
	VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.basePipelineIndex = -1;
	pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	return pipelineCreateInfo;
}

VkComputePipelineCreateInfo vks::initializers::computePipelineCreateInfo(VkPipelineLayout layout, VkPipelineCreateFlags flags)
{
	VkComputePipelineCreateInfo computePipelineCreateInfo{};
	computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	computePipelineCreateInfo.layout = layout;
	computePipelineCreateInfo.flags = flags;
	return computePipelineCreateInfo;
}

VkPushConstantRange vks::initializers::pushConstantRange(VkShaderStageFlags stageFlags, uint32_t size, uint32_t offset)
{
	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = stageFlags;
	pushConstantRange.offset = offset;
	pushConstantRange.size = size;
	return pushConstantRange;
}

VkBindSparseInfo vks::initializers::bindSparseInfo()
{
	VkBindSparseInfo bindSparseInfo{};
	bindSparseInfo.sType = VK_STRUCTURE_TYPE_BIND_SPARSE_INFO;
	return bindSparseInfo;
}

/** @brief Initialize a map entry for a shader specialization constant */
VkSpecializationMapEntry vks::initializers::specializationMapEntry(uint32_t constantID, uint32_t offset, size_t size)
{
	VkSpecializationMapEntry specializationMapEntry{};
	specializationMapEntry.constantID = constantID;
	specializationMapEntry.offset = offset;
	specializationMapEntry.size = size;
	return specializationMapEntry;
}

/** @brief Initialize a specialization constant info structure to pass to a shader stage */
VkSpecializationInfo vks::initializers::specializationInfo(uint32_t mapEntryCount, const VkSpecializationMapEntry* mapEntries, size_t dataSize, const void* data)
{
	VkSpecializationInfo specializationInfo{};
	specializationInfo.mapEntryCount = mapEntryCount;
	specializationInfo.pMapEntries = mapEntries;
	specializationInfo.dataSize = dataSize;
	specializationInfo.pData = data;
	return specializationInfo;
}
