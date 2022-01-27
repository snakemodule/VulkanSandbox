
#include "VulkanInitializers.hpp"

namespace vks
{
	namespace initializers
	{
		vk::PipelineShaderStageCreateInfo pipeline_shader_stage_create_info(vk::ShaderStageFlagBits stage, vk::ShaderModule shaderModule)
		{
			vk::PipelineShaderStageCreateInfo info;
			info.pNext = nullptr;
			info.stage = stage;
			info.module = shaderModule;
			info.pName = "main";
			return info;
		}

		vk::CommandBufferAllocateInfo commandBufferAllocateInfo(
			vk::CommandPool commandPool,
			vk::CommandBufferLevel level,
			uint32_t bufferCount)
		{
			vk::CommandBufferAllocateInfo commandBufferAllocateInfo{};
			commandBufferAllocateInfo.commandPool = commandPool;
			commandBufferAllocateInfo.level = level;
			commandBufferAllocateInfo.commandBufferCount = bufferCount;
			return commandBufferAllocateInfo;
		}

		vk::CommandPoolCreateInfo commandPoolCreateInfo()
		{
			vk::CommandPoolCreateInfo cmdPoolCreateInfo{};
			return cmdPoolCreateInfo;
		}

		vk::CommandBufferBeginInfo commandBufferBeginInfo()
		{
			vk::CommandBufferBeginInfo cmdBufferBeginInfo{};
			return cmdBufferBeginInfo;
		}

		vk::CommandBufferInheritanceInfo commandBufferInheritanceInfo()
		{
			vk::CommandBufferInheritanceInfo cmdBufferInheritanceInfo{};
			return cmdBufferInheritanceInfo;
		}

		vk::RenderPassBeginInfo renderPassBeginInfo()
		{
			vk::RenderPassBeginInfo renderPassBeginInfo{};
			return renderPassBeginInfo;
		}

		vk::RenderPassCreateInfo renderPassCreateInfo()
		{
			vk::RenderPassCreateInfo renderPassCreateInfo{};
			return renderPassCreateInfo;
		}

		/** @brief Initialize an image memory barrier with no image transfer ownership */
		vk::ImageMemoryBarrier imageMemoryBarrier()
		{
			vk::ImageMemoryBarrier imageMemoryBarrier{};
			imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			return imageMemoryBarrier;
		}

		/** @brief Initialize a buffer memory barrier with no image transfer ownership */
		vk::BufferMemoryBarrier bufferMemoryBarrier()
		{
			vk::BufferMemoryBarrier bufferMemoryBarrier{};
			bufferMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			bufferMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			return bufferMemoryBarrier;
		}

		vk::MemoryBarrier memoryBarrier()
		{
			vk::MemoryBarrier memoryBarrier{};
			return memoryBarrier;
		}

		vk::ImageCreateInfo imageCreateInfo()
		{
			vk::ImageCreateInfo imageCreateInfo{};
			return imageCreateInfo;
		}

		vk::SamplerCreateInfo samplerCreateInfo()
		{
			vk::SamplerCreateInfo samplerCreateInfo{};
			samplerCreateInfo.maxAnisotropy = 1.0f;
			return samplerCreateInfo;
		}

		vk::ImageViewCreateInfo imageViewCreateInfo()
		{
			vk::ImageViewCreateInfo imageViewCreateInfo{};
			return imageViewCreateInfo;
		}

		vk::FramebufferCreateInfo framebufferCreateInfo()
		{
			vk::FramebufferCreateInfo framebufferCreateInfo{};
			return framebufferCreateInfo;
		}

		vk::SemaphoreCreateInfo semaphoreCreateInfo()
		{
			vk::SemaphoreCreateInfo semaphoreCreateInfo{};
			return semaphoreCreateInfo;
		}

		vk::FenceCreateInfo fenceCreateInfo(vk::FenceCreateFlags flags)
		{
			vk::FenceCreateInfo fenceCreateInfo{};
			fenceCreateInfo.flags = flags;
			return fenceCreateInfo;
		}

		vk::EventCreateInfo eventCreateInfo()
		{
			vk::EventCreateInfo eventCreateInfo{};
			return eventCreateInfo;
		}

		vk::SubmitInfo submitInfo()
		{
			vk::SubmitInfo submitInfo{};
			return submitInfo;
		}

		vk::Viewport viewport(
			float width,
			float height,
			float minDepth,
			float maxDepth)
		{
			vk::Viewport viewport{};
			viewport.width = width;
			viewport.height = height;
			viewport.minDepth = minDepth;
			viewport.maxDepth = maxDepth;
			return viewport;
		}

		vk::Rect2D rect2D(
			int32_t width,
			int32_t height,
			int32_t offsetX,
			int32_t offsetY)
		{
			vk::Rect2D rect2D{};
			rect2D.extent.width = width;
			rect2D.extent.height = height;
			rect2D.offset.x = offsetX;
			rect2D.offset.y = offsetY;
			return rect2D;
		}

		vk::Rect2D rect2D(
			vk::Extent2D extent,
			int32_t offsetX,
			int32_t offsetY)
		{
			vk::Rect2D rect2D{};
			rect2D.extent = extent;
			rect2D.offset.x = offsetX;
			rect2D.offset.y = offsetY;
			return rect2D;
		}

		vk::BufferCreateInfo bufferCreateInfo()
		{
			vk::BufferCreateInfo bufCreateInfo{};
			return bufCreateInfo;
		}

		vk::BufferCreateInfo bufferCreateInfo(
			vk::BufferUsageFlags usage,
			vk::DeviceSize size)
		{
			vk::BufferCreateInfo bufCreateInfo{};
			bufCreateInfo.usage = usage;
			bufCreateInfo.size = size;
			return bufCreateInfo;
		}

		vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo(
			uint32_t poolSizeCount,
			vk::DescriptorPoolSize* pPoolSizes,
			uint32_t maxSets)
		{
			vk::DescriptorPoolCreateInfo descriptorPoolInfo{};
			descriptorPoolInfo.poolSizeCount = poolSizeCount;
			descriptorPoolInfo.pPoolSizes = pPoolSizes;
			descriptorPoolInfo.maxSets = maxSets;
			return descriptorPoolInfo;
		}

		vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo(
			const std::vector<vk::DescriptorPoolSize>& poolSizes,
			uint32_t maxSets)
		{
			vk::DescriptorPoolCreateInfo descriptorPoolInfo{};
			descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
			descriptorPoolInfo.pPoolSizes = poolSizes.data();
			descriptorPoolInfo.maxSets = maxSets;
			return descriptorPoolInfo;
		}

		vk::DescriptorPoolSize descriptorPoolSize(
			vk::DescriptorType type,
			uint32_t descriptorCount)
		{
			vk::DescriptorPoolSize descriptorPoolSize{};
			descriptorPoolSize.type = type;
			descriptorPoolSize.descriptorCount = descriptorCount;
			return descriptorPoolSize;
		}

		vk::DescriptorSetLayoutBinding descriptorSetLayoutBinding(
			vk::DescriptorType type,
			vk::ShaderStageFlags stageFlags,
			uint32_t binding,
			uint32_t descriptorCount)
		{
			vk::DescriptorSetLayoutBinding setLayoutBinding{};
			setLayoutBinding.descriptorType = type;
			setLayoutBinding.stageFlags = stageFlags;
			setLayoutBinding.binding = binding;
			setLayoutBinding.descriptorCount = descriptorCount;
			return setLayoutBinding;
		}

		vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo(
			const vk::DescriptorSetLayoutBinding* pBindings,
			uint32_t bindingCount)
		{
			vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
			descriptorSetLayoutCreateInfo.pBindings = pBindings;
			descriptorSetLayoutCreateInfo.bindingCount = bindingCount;
			return descriptorSetLayoutCreateInfo;
		}

		vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo(
			const std::vector<vk::DescriptorSetLayoutBinding>& bindings)
		{
			vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
			descriptorSetLayoutCreateInfo.pBindings = bindings.data();
			descriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(bindings.size());
			return descriptorSetLayoutCreateInfo;
		}

		vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo(
			const std::vector<VkDescriptorSetLayoutBinding>& bindings) //todo deprecate this
		{
			VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
			descriptorSetLayoutCreateInfo.pBindings = bindings.data();
			descriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(bindings.size());
			return descriptorSetLayoutCreateInfo;
		}

		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo() {
			vk::PipelineLayoutCreateInfo info;
			info.pNext = nullptr;
			//empty defaults
			info.flags = vk::PipelineLayoutCreateFlags();
			info.setLayoutCount = 0;
			info.pSetLayouts = nullptr;
			info.pushConstantRangeCount = 0;
			info.pPushConstantRanges = nullptr;
			return info;
		}

		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo(
			const vk::DescriptorSetLayout* pSetLayouts,
			uint32_t setLayoutCount)
		{
			vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
			pipelineLayoutCreateInfo.setLayoutCount = setLayoutCount;
			pipelineLayoutCreateInfo.pSetLayouts = pSetLayouts;
			return pipelineLayoutCreateInfo;
		}


		vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo(
			vk::DescriptorPool descriptorPool,
			const vk::DescriptorSetLayout* pSetLayouts,
			uint32_t descriptorSetCount)
		{
			vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo{};
			descriptorSetAllocateInfo.descriptorPool = descriptorPool;
			descriptorSetAllocateInfo.pSetLayouts = pSetLayouts;
			descriptorSetAllocateInfo.descriptorSetCount = descriptorSetCount;
			return descriptorSetAllocateInfo;
		}

		vk::DescriptorImageInfo descriptorImageInfo(vk::Sampler sampler, vk::ImageView imageView, vk::ImageLayout imageLayout)
		{
			vk::DescriptorImageInfo descriptorImageInfo{};
			descriptorImageInfo.sampler = sampler;
			descriptorImageInfo.imageView = imageView;
			descriptorImageInfo.imageLayout = imageLayout;
			return descriptorImageInfo;
		}

		vk::WriteDescriptorSet writeDescriptorSet(
			vk::DescriptorSet dstSet,
			vk::DescriptorType type,
			uint32_t binding,
			vk::DescriptorBufferInfo* bufferInfo,
			uint32_t descriptorCount)
		{
			vk::WriteDescriptorSet writeDescriptorSet{};
			writeDescriptorSet.dstSet = dstSet;
			writeDescriptorSet.descriptorType = type;
			writeDescriptorSet.dstBinding = binding;
			writeDescriptorSet.pBufferInfo = bufferInfo;
			writeDescriptorSet.descriptorCount = descriptorCount;
			return writeDescriptorSet;
		}

		vk::WriteDescriptorSet writeDescriptorSet(
			vk::DescriptorSet dstSet,
			vk::DescriptorType type,
			uint32_t binding,
			vk::DescriptorImageInfo* imageInfo,
			uint32_t descriptorCount)
		{
			vk::WriteDescriptorSet writeDescriptorSet{};
			writeDescriptorSet.dstSet = dstSet;
			writeDescriptorSet.descriptorType = type;
			writeDescriptorSet.dstBinding = binding;
			writeDescriptorSet.pImageInfo = imageInfo;
			writeDescriptorSet.descriptorCount = descriptorCount;
			return writeDescriptorSet;
		}

		vk::VertexInputBindingDescription vertexInputBindingDescription(
			uint32_t binding,
			uint32_t stride,
			vk::VertexInputRate inputRate)
		{
			vk::VertexInputBindingDescription vInputBindDescription{};
			vInputBindDescription.binding = binding;
			vInputBindDescription.stride = stride;
			vInputBindDescription.inputRate = inputRate;
			return vInputBindDescription;
		}

		vk::VertexInputAttributeDescription vertexInputAttributeDescription(
			uint32_t binding,
			uint32_t location,
			vk::Format format,
			uint32_t offset)
		{
			vk::VertexInputAttributeDescription vInputAttribDescription{};
			vInputAttribDescription.location = location;
			vInputAttribDescription.binding = binding;
			vInputAttribDescription.format = format;
			vInputAttribDescription.offset = offset;
			return vInputAttribDescription;
		}

		vk::PipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo()
		{
			vk::PipelineVertexInputStateCreateInfo info = {};
			info.pNext = nullptr;
			info.vertexBindingDescriptionCount = 0;
			info.vertexAttributeDescriptionCount = 0;
			return info;
		}

		vk::PipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo(
			vk::PrimitiveTopology topology,
			vk::PipelineInputAssemblyStateCreateFlags flags,
			vk::Bool32 primitiveRestartEnable)
		{
			vk::PipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo{};
			pipelineInputAssemblyStateCreateInfo.topology = topology;
			pipelineInputAssemblyStateCreateInfo.flags = flags;
			pipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = primitiveRestartEnable;
			return pipelineInputAssemblyStateCreateInfo;
		}

		vk::PipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo(
			vk::PolygonMode polygonMode,
			vk::CullModeFlags cullMode,
			vk::FrontFace frontFace,
			vk::PipelineRasterizationStateCreateFlags flags)
		{
			vk::PipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo{};
			pipelineRasterizationStateCreateInfo.polygonMode = polygonMode;
			pipelineRasterizationStateCreateInfo.cullMode = cullMode;
			pipelineRasterizationStateCreateInfo.frontFace = frontFace;
			pipelineRasterizationStateCreateInfo.flags = flags;
			pipelineRasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
			pipelineRasterizationStateCreateInfo.lineWidth = 1.0f;
			return pipelineRasterizationStateCreateInfo;
		}

		vk::PipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo(vk::PolygonMode polygonMode)
		{
			vk::PipelineRasterizationStateCreateInfo info;
			info.pNext = nullptr;
			info.depthClampEnable = VK_FALSE;
			//discards all primitives before the rasterization stage if enabled which we don't want
			info.rasterizerDiscardEnable = VK_FALSE;
			info.polygonMode = polygonMode;
			info.lineWidth = 1.0f;
			//no backface cull
			info.cullMode = vk::CullModeFlagBits::eNone;
			info.frontFace = vk::FrontFace::eCounterClockwise;
			//no depth bias
			info.depthBiasEnable = VK_FALSE;
			info.depthBiasConstantFactor = 0.0f;
			info.depthBiasClamp = 0.0f;
			info.depthBiasSlopeFactor = 0.0f;

			return info;
		}


		vk::PipelineColorBlendAttachmentState pipelineColorBlendAttachmentState() {
			vk::PipelineColorBlendAttachmentState colorBlendAttachment = {};
			colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR |
				vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
			colorBlendAttachment.blendEnable = VK_FALSE;
			return colorBlendAttachment;
		}


		vk::PipelineColorBlendAttachmentState pipelineColorBlendAttachmentState(
			vk::ColorComponentFlags colorWriteMask,
			vk::Bool32 blendEnable)
		{
			vk::PipelineColorBlendAttachmentState pipelineColorBlendAttachmentState{};
			pipelineColorBlendAttachmentState.colorWriteMask = colorWriteMask;
			pipelineColorBlendAttachmentState.blendEnable = blendEnable;
			return pipelineColorBlendAttachmentState;
		}

		vk::PipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo(
			uint32_t attachmentCount,
			const vk::PipelineColorBlendAttachmentState* pAttachments)
		{
			vk::PipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo{};
			pipelineColorBlendStateCreateInfo.attachmentCount = attachmentCount;
			pipelineColorBlendStateCreateInfo.pAttachments = pAttachments;
			return pipelineColorBlendStateCreateInfo;
		}

		vk::PipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo(
			vk::Bool32 depthTestEnable,
			vk::Bool32 depthWriteEnable,
			vk::CompareOp depthCompareOp)
		{
			vk::PipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo{};
			pipelineDepthStencilStateCreateInfo.depthTestEnable = depthTestEnable;
			pipelineDepthStencilStateCreateInfo.depthWriteEnable = depthWriteEnable;
			pipelineDepthStencilStateCreateInfo.depthCompareOp = depthCompareOp;
			pipelineDepthStencilStateCreateInfo.front = pipelineDepthStencilStateCreateInfo.back;
			pipelineDepthStencilStateCreateInfo.back.compareOp = vk::CompareOp::eAlways;
			return pipelineDepthStencilStateCreateInfo;
		}

		vk::PipelineViewportStateCreateInfo pipelineViewportStateCreateInfo(
			uint32_t viewportCount,
			uint32_t scissorCount,
			vk::PipelineViewportStateCreateFlags flags)
		{
			vk::PipelineViewportStateCreateInfo pipelineViewportStateCreateInfo{};
			pipelineViewportStateCreateInfo.viewportCount = viewportCount;
			pipelineViewportStateCreateInfo.scissorCount = scissorCount;
			pipelineViewportStateCreateInfo.flags = flags;
			return pipelineViewportStateCreateInfo;
		}

		vk::PipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo()
		{
			vk::PipelineMultisampleStateCreateInfo info = {};
			info.pNext = nullptr;
			info.sampleShadingEnable = VK_FALSE;
			//multisampling defaulted to no multisampling (1 sample per pixel)
			info.rasterizationSamples = vk::SampleCountFlagBits::e1;
			info.minSampleShading = 1.0f;
			info.pSampleMask = nullptr;
			info.alphaToCoverageEnable = VK_FALSE;
			info.alphaToOneEnable = VK_FALSE;
			return info;
		}


		vk::PipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo(
			vk::SampleCountFlagBits rasterizationSamples,
			vk::PipelineMultisampleStateCreateFlags flags)
		{
			vk::PipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo{};
			pipelineMultisampleStateCreateInfo.rasterizationSamples = rasterizationSamples;
			pipelineMultisampleStateCreateInfo.flags = flags;
			return pipelineMultisampleStateCreateInfo;
		}

		vk::PipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo(
			const vk::DynamicState* pDynamicStates,
			uint32_t dynamicStateCount,
			vk::PipelineDynamicStateCreateFlags flags)
		{
			vk::PipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo{};
			pipelineDynamicStateCreateInfo.pDynamicStates = pDynamicStates;
			pipelineDynamicStateCreateInfo.dynamicStateCount = dynamicStateCount;
			pipelineDynamicStateCreateInfo.flags = flags;
			return pipelineDynamicStateCreateInfo;
		}

		vk::PipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo(
			const std::vector<vk::DynamicState>& pDynamicStates,
			vk::PipelineDynamicStateCreateFlags flags)
		{
			vk::PipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo{};
			pipelineDynamicStateCreateInfo.pDynamicStates = pDynamicStates.data();
			pipelineDynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(pDynamicStates.size());
			pipelineDynamicStateCreateInfo.flags = flags;
			return pipelineDynamicStateCreateInfo;
		}

		vk::PipelineTessellationStateCreateInfo pipelineTessellationStateCreateInfo(uint32_t patchControlPoints)
		{
			vk::PipelineTessellationStateCreateInfo pipelineTessellationStateCreateInfo{};
			pipelineTessellationStateCreateInfo.patchControlPoints = patchControlPoints;
			return pipelineTessellationStateCreateInfo;
		}

		vk::GraphicsPipelineCreateInfo pipelineCreateInfo(
			vk::PipelineLayout layout,
			vk::RenderPass renderPass,
			vk::PipelineCreateFlags flags)
		{
			vk::GraphicsPipelineCreateInfo pipelineCreateInfo{};
			pipelineCreateInfo.layout = layout;
			pipelineCreateInfo.renderPass = renderPass;
			pipelineCreateInfo.flags = flags;
			pipelineCreateInfo.basePipelineIndex = -1;
			pipelineCreateInfo.basePipelineHandle = vk::Pipeline();
			return pipelineCreateInfo;
		}

		vk::GraphicsPipelineCreateInfo pipelineCreateInfo()
		{
			vk::GraphicsPipelineCreateInfo pipelineCreateInfo{};
			pipelineCreateInfo.basePipelineIndex = -1;
			pipelineCreateInfo.basePipelineHandle = vk::Pipeline();
			return pipelineCreateInfo;
		}

		vk::ComputePipelineCreateInfo computePipelineCreateInfo(
			vk::PipelineLayout layout,
			vk::PipelineCreateFlags flags)
		{
			vk::ComputePipelineCreateInfo computePipelineCreateInfo{};
			computePipelineCreateInfo.layout = layout;
			computePipelineCreateInfo.flags = flags;
			return computePipelineCreateInfo;
		}

		vk::PushConstantRange pushConstantRange(
			vk::ShaderStageFlags stageFlags,
			uint32_t size,
			uint32_t offset)
		{
			vk::PushConstantRange pushConstantRange{};
			pushConstantRange.stageFlags = stageFlags;
			pushConstantRange.offset = offset;
			pushConstantRange.size = size;
			return pushConstantRange;
		}

		vk::BindSparseInfo bindSparseInfo()
		{
			vk::BindSparseInfo bindSparseInfo{};
			return bindSparseInfo;
		}

		/** @brief Initialize a map entry for a shader specialization constant */
		vk::SpecializationMapEntry specializationMapEntry(uint32_t constantID, uint32_t offset, size_t size)
		{
			vk::SpecializationMapEntry specializationMapEntry{};
			specializationMapEntry.constantID = constantID;
			specializationMapEntry.offset = offset;
			specializationMapEntry.size = size;
			return specializationMapEntry;
		}

		/** @brief Initialize a specialization constant info structure to pass to a shader stage */
		vk::SpecializationInfo specializationInfo(uint32_t mapEntryCount, const vk::SpecializationMapEntry* mapEntries, size_t dataSize, const void* data)
		{
			vk::SpecializationInfo specializationInfo{};
			specializationInfo.mapEntryCount = mapEntryCount;
			specializationInfo.pMapEntries = mapEntries;
			specializationInfo.dataSize = dataSize;
			specializationInfo.pData = data;
			return specializationInfo;
		}
	}
}