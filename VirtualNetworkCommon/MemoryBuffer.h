// Author: Oznabikhin Dmitry
// Email: gamexgroup@gmail.com
//
// Copyright (c) GameX Corporation. All rights reserved.

#pragma once

#include "Archive.h"

namespace vnet {

/**
 * @brief Memeory buffer.
 */
class MemoryBuffer
{
public:
	MemoryBuffer()
		: _nbBytes(0)
		, _buffer(nullptr)
		, _capicity(0)
		, _privateData(true)
	{}

	MemoryBuffer(uint32 nbBytes)
		: _nbBytes(nbBytes)
		, _buffer(nullptr)
		, _capicity(nbBytes)
		, _privateData(true)
	{
		Realloc(_capicity);
		memset(_buffer, 0x00, _nbBytes);
	}

	MemoryBuffer(uint8* bytes, uint32 nbBytes)
		: _nbBytes(nbBytes)
		, _buffer(bytes)
		, _capicity(nbBytes)
		, _privateData(false)
	{}

	MemoryBuffer(const uint8* bytes, uint32 nbBytes)
		: _nbBytes(nbBytes)
		, _capicity(nbBytes)
		, _privateData(false)
	{
		_buffer = const_cast<uint8*>(bytes); // TODO. Please don't use for write.
	}

	MemoryBuffer(const MemoryBuffer& buffer)
		: _nbBytes(buffer._nbBytes)
		, _buffer(nullptr)
		, _capicity(_nbBytes)
		, _privateData(true)
	{
		Realloc(_capicity);
		memcpy(_buffer, buffer._buffer, _nbBytes);
	}

	virtual ~MemoryBuffer();

    void Put(uint8* bytes, uint32 nbBytes)
    {
        _nbBytes = 0;
        _buffer = bytes;
        _capicity = nbBytes;
        _privateData = false;
    }

	/**
	* @brief Realloc and resize buffer;
	* @param nbBytes bytes count.
	*/
	void Resize(uint32 nbBytes)
	{
		if (_privateData)
		{
			_nbBytes = nbBytes;
			_capicity = nbBytes;
			Realloc(_capicity);
		}
	}

	/**
	* @brief Add new data in buffer.
	* @param data data bytes.
	* @param length bytes count.
	*/
	void AddData(const uint8* data, uint32 length)
	{
		if (_capicity < _nbBytes + length)
		{
			int32 num = _nbBytes + length;

			_capicity = num + 3 * num / 8 + 32;
			Realloc(_capicity);
		}

		memcpy(_buffer + _nbBytes, data, length);
		_nbBytes += length;
	}

	/**
	 * @brief Reset memory data.
	 */
	void Reset()
    {
		_nbBytes = 0;
    }

	/**
	* Inline methods.
	*/
	uint32 GetSize() const { return _nbBytes; }
	uint32 GetCapicity() const { return _capicity; }

	uint8* GetData() { return _buffer; }
	const uint8* GetData() const { return _buffer; }


	friend Archive& operator << (Archive& archive, MemoryBuffer& buffer)
	{
		if (archive.IsLoading())
		{
			uint32 nbBytes;
			archive << nbBytes;

			buffer.Resize(nbBytes);
			archive.ByteSerialize(buffer._buffer, nbBytes);
		}
		else if (archive.IsSaving())
		{
			archive << buffer._nbBytes;
			archive.ByteSerialize(buffer._buffer, buffer._nbBytes);
		}

		return archive;
	}

private:
	void Realloc(uint32 nbBytes);

private:
	uint32 _nbBytes;

	uint8* _buffer;
	uint32 _capicity;

	bool _privateData;
};


/**
 * @brief The BufferReader class
 */
class  MemoryReader : public Archive
{
public:
	MemoryReader(const MemoryBuffer& buffer);
	virtual ~MemoryReader();

	virtual void Seek(uint32 pos) override;
	virtual void Serialize(void* buf, uint32 length) override;

    /**
     * @brief Get cursor current position.
     * @return bytes offset.
     */
    virtual uint32 GetPosition() const override {return _pos;}

    /**
     * @brief Get archive size in bytes.
     * @return size in bytes.
     */
    virtual uint32 GetSize() const override {return _buffer.GetSize();}

    virtual uint32 Position() override { return _pos; } // TODO. Bad syntax.
    virtual uint32 Size() override { return _buffer.GetSize(); } // TODO. Bad syntax.

    virtual uint32 Position() const override { return _pos; } // TODO. Bad syntax.
    virtual uint32 Size() const override { return _buffer.GetSize(); } // TODO. Bad syntax.

	const uint8* GetData() const { return _buffer.GetData(); }

private:
	uint32 _pos;
	const MemoryBuffer& _buffer;
};


/**
 * @brief The BufferWriter class
 */
class  MemoryWriter : public Archive
{
public:
	MemoryWriter(MemoryBuffer& buffer);
	virtual ~MemoryWriter();

	// virtual uint32 GetPosition() const override {return _offset;}
	// virtual void Seek( uint32 ){;}

	virtual void Serialize(void* buf, uint32 length) override;

    /**
     * @brief Get archive size in bytes.
     * @return size in bytes.
     */
    virtual uint32 GetSize() const override {return _buffer.GetSize();}

    virtual uint32 Size() override { return _buffer.GetSize(); } // TODO. Bad syntax.
    virtual uint32 Size() const override { return _buffer.GetSize(); } // TODO. Bad syntax.

	const uint8* GetData() const { return _buffer.GetData(); }

protected:
	MemoryBuffer& _buffer;
};

} // End vnet.
