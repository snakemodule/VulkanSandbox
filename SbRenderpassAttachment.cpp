#include "SbRenderpassAttachment.h"



SbRenderpassAttachment::SbRenderpassAttachment(size_t size)
	: image(size), mem(size), view(size)
{
}

SbRenderpassAttachment::~SbRenderpassAttachment()
{
}
