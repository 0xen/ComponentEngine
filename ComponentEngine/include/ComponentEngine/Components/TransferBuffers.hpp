#pragma once

namespace ComponentEngine
{
	class TransferBuffers
	{
	public:
		virtual void BufferTransfer() = 0;
		virtual void SetBufferData() = 0;
	};
}