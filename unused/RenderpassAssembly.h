#pragma once

#include "DAGclasses.hpp"

#include <string>
#include <map>


class RenderpassAssembly {

public:
	RenderpassAssembly() {};
	~RenderpassAssembly() {};


	SbRenderpassProperties renderpass;


	void constructRenderpass() {
		SbSubpass writeAttachment;
		SbSubpass readAttachment;



	}

	SbSubpass				getSubpass(std::string);
	SbAttachment			getAttachment(std::string);
	SbInputDependency		getInputDependency(std::string);
	SbOutputDependency		getOutputDependency(std::string);
	SbSubpassDependency		getSubpassDependency(std::string);

	void scratchAssembly();

	//void connectSubpasses(SbSubpass preceding, SbSubpass subsequent, )


private:


	std::map<std::string, SbSubpass> subpasses;
	std::map<std::string, SbAttachment> attachments;
	std::map<std::string, SbInputDependency> inputDependencies;
	std::map<std::string, SbOutputDependency> outputDependencies;
	std::map<std::string, SbSubpassDependency> outputDependencies;
};

