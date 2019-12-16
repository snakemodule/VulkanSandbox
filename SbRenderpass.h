#pragma once

#include "vulkan/vulkan.h"
#include <map>

#include <memory>

#include "SbRenderpassAttachment.h"


/*

class SbSubpass {
public:
	VkSubpassDescription desc;

	std::map<const uint32_t, VkAttachmentReference> colorRefs;
	std::map<const uint32_t, VkAttachmentReference> depthStencilRefs;
	std::map<const uint32_t, VkAttachmentReference> inputRefs;

	SbSubpass();
	~SbSubpass();

};

class SbSubpassDependency {
public:
	VkSubpassDependency dep;

	std::unique_ptr<SbSubpass> src = nullptr;
	std::unique_ptr<SbSubpass> dst = nullptr;

	SbSubpassDependency();
	~SbSubpassDependency();
};


class SbRenderpass
{
public:
	SbRenderpass();
	~SbRenderpass();

	std::map<const uint32_t, SbRenderpassAttachment> attachments; //attachments to be used in this 


	std::map<const uint32_t, SbSubpass> subpasses; //the subpasses that will use the attachments


	std::map<const uint32_t, SbSubpassDependency> subpassDeps; 
	//the dependencies (that will be partially generated from usage of attachments??? 
	//since dependency and attachment formats in references should be codetermined?)

	

};

*/
