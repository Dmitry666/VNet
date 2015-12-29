#include "MemoryBuffer.h"

namespace vnet {

MemoryBuffer::~MemoryBuffer()
{
	if (_privateData)
	{
		free(_buffer);
	}
}

void MemoryBuffer::Realloc(uint32 nbBytes)
{
	_buffer = static_cast<uint8*>(realloc(_buffer, nbBytes));
}

MemoryReader::MemoryReader(const MemoryBuffer& buffer)
	: Archive()
	, _pos(0)
	, _buffer(buffer)
{
	_saving = false;
}

MemoryReader::~MemoryReader()
{}

void MemoryReader::Seek(uint32 pos)
{
	if (pos >= _buffer.GetSize())
		return;

	_pos = pos;
}

void MemoryReader::Serialize(void* buf, uint32 length)
{
	int32 copy = length;
	if (length + _pos > _buffer.GetSize())
		copy = _buffer.GetSize() - _pos;

	memcpy(buf, _buffer.GetData() + _pos, copy);
	_pos += copy;
}


/**
* @brief MemoryWriter::MemoryWriter
* @param buffer
*/
MemoryWriter::MemoryWriter(MemoryBuffer& buffer)
	: Archive()
	, _buffer(buffer)
{
	_saving = true;
}

MemoryWriter :: ~MemoryWriter(void)
{}

void MemoryWriter::Serialize(void* buf, uint32 length)
{
	_buffer.AddData(static_cast<uint8*>(buf), length);
}

} // End vnet.
