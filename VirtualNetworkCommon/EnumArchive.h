#pragma once

#include "Archive.h"

namespace vnet {

template<class TEnum>
class TEnumAsByte
{
public:
	typedef TEnum EnumType;

	__inline TEnumAsByte()
	{}

	__inline TEnumAsByte(const TEnumAsByte &byteEnum)
		: _value(byteEnum._value)
	{}

	__inline TEnumAsByte(TEnum value)
		: _value(value)
	{}

	__inline explicit TEnumAsByte(uint8 value)
		: _value(value)
	{}

	__inline explicit TEnumAsByte(int32 value)
		: _value(value)
	{}

	__inline TEnumAsByte& operator=(TEnumAsByte byteEnum)
	{
		_value = byteEnum._value;
		return *this;
	}

	__inline TEnumAsByte& operator=(TEnum value)
	{
		_value = value;
		return *this;
	}

	/** Operators. */
	operator TEnum() const
	{
		return TEnum(_value);
	}

	bool operator==(TEnum value) const
	{
		return _value == value;
	}

	bool operator==(TEnumAsByte byteEnum) const
	{
		return _value == byteEnum._value;
	}

	TEnum GetValue() const
	{
		return TEnum(_value);
	}


	friend Archive& operator << (Archive& archive, TEnumAsByte<TEnum>& enumAsByte)
	{
		archive << enumAsByte._value;
		return archive;
	}

private:
	uint8 _value;
};


template<class TEnum>
class TEnumAsInt32
{
public:
	typedef TEnum EnumType;

	__inline TEnumAsInt32()
	{}

	__inline TEnumAsInt32(const TEnumAsByte &byteEnum)
		: _value(byteEnum._value)
	{}

	__inline TEnumAsInt32(TEnum value)
		: _value(value)
	{}

	__inline explicit TEnumAsInt32(int32 value)
		: _value(value)
	{}

	__inline TEnumAsInt32& operator=(TEnumAsInt32 byteEnum)
	{
		_value = byteEnum._value;
		return *this;
	}

	__inline TEnumAsInt32& operator=(TEnum value)
	{
		_value = value;
		return *this;
	}

	/** Operators. */
	operator TEnum() const
	{
		return TEnum(_value);
	}

	bool operator==(TEnum value) const
	{
		return _value == value;
	}

	bool operator==(TEnumAsInt32 byteEnum) const
	{
		return _value == byteEnum._value;
	}

	TEnum GetValue() const
	{
		return TEnum(_value);
	}


	friend Archive& operator << (Archive& archive, TEnumAsInt32<TEnum>& enumAsByte)
	{
		archive << enumAsByte._value;
		return archive;
	}

private:
	uint32 _value;
};

} // End vnet.