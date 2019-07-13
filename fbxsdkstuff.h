#pragma once
#include <fbxsdk.h>
#include <iostream>

class fbxsdkstuff
{
public:
	FbxManager* lSdkManager;

	void PrintFBXStructure();

	

	fbxsdkstuff();
	~fbxsdkstuff();

private:
	void PrintAttribute(FbxNodeAttribute* pAttribute);
	void PrintTabs();
	void PrintNode(FbxNode* pNode);
	FbxString GetAttributeTypeName(FbxNodeAttribute::EType type);

	int numTabs = 0;

	
};

