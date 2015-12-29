#pragma once

#include "Common.h"
#include "MemoryBuffer.h"

namespace vnet{

/**
 * @brief Argument variant struct.
 */
struct Variant
{
	enum Type
	{
		None,
		Bool,
		Int,
		Float,
		String,
		Bytes
	};
	Type Type;

	union
	{
		bool BoolValue;
		int32 IntValue;
		float FloatValue;
		std::string* StringValue;
		std::vector<uint8>* BytesValue;
	} Data;

	Variant()
	{}

	Variant(const Variant& argument)
		: Type(argument.Type)
	{
		switch (Type)
		{
		case None:
			break;
		case Bool:
			Data.BoolValue = argument.Data.BoolValue;
			break;
		case Int:
			Data.IntValue = argument.Data.IntValue;
			break;
		case Float:
			Data.FloatValue = argument.Data.FloatValue;
			break;
		case String:
			Data.StringValue = new std::string(*argument.Data.StringValue);
			break;
		case Bytes:
			Data.BytesValue = new std::vector<uint8>(*argument.Data.BytesValue);
			break;
		default:
			break;
		}
	}

	Variant(Variant&& argument)
		: Type(argument.Type)
	{
		switch (Type)
		{
		case None:
			break;
		case Bool:
			Data.BoolValue = argument.Data.BoolValue;
			break;
		case Int:
			Data.IntValue = argument.Data.IntValue;
			break;
		case Float:
			Data.FloatValue = argument.Data.FloatValue;
			break;
		case String:
			Data.StringValue = argument.Data.StringValue;
			argument.Data.StringValue = nullptr;
			argument.Type = None;
			break;
		case Bytes:
			Data.BytesValue = argument.Data.BytesValue;
			argument.Data.BytesValue = nullptr;
			argument.Type = None;
			break;
		default:
			break;
		}
	}

	explicit Variant(bool value)
		: Type(Bool)
	{
		Data.BoolValue = value;
	}

	explicit Variant(int32 value)
		: Type(Int)
	{
		Data.IntValue = value;
	}

	explicit Variant(float value)
		: Type(Float)
	{
		Data.FloatValue = value;
	}

	Variant(const std::string& value)
		: Type(String)
	{
		Data.StringValue = new std::string(value);
	}

	Variant(const char* text)
		: Type(String)
	{
		Data.StringValue = new std::string(text);
	}

	Variant(const uint8* bytes, uint32 nbBytes)
		: Type(Bytes)
	{
		Data.BytesValue = new std::vector<uint8>(nbBytes);
		memcpy(&(*Data.BytesValue)[0], bytes, nbBytes);
	}

	~Variant()
	{
		switch (Type)
		{
		case None:
			break;
		case Bool:
			break;
		case Int:
			break;
		case Float:
			break;
		case String:
			delete Data.StringValue;
			break;
		case Bytes:
			delete Data.BytesValue;
			break;
		default:
			break;
		}
	}

	// Casts.
	bool AsBool() const
	{
		return Data.BoolValue;
	}

	int32 AsInt() const
	{
		return Data.IntValue;
	}

	float AsFloat() const
	{
		return Data.FloatValue;
	}

	const std::string& AsString() const
	{
		return *Data.StringValue;
	}

	const std::vector<uint8>& AsBytes() const
	{
		return *Data.BytesValue;
	}

	// To.
	bool ToBool() const
	{
		return Data.BoolValue;
	}

	int32 ToInt() const
	{
		return Data.IntValue;
	}

	float ToFloat() const
	{
		return Data.FloatValue;
	}

	std::string ToString() const
	{
		return (*Data.StringValue);
	}

	std::vector<uint8> ToBytes() const
	{
		return *Data.BytesValue;
	}

	//
	bool IsBool() const
	{
		return Type == Bool;
	}

	bool IsInt() const
	{
		return Type == Int;
	}

	bool IsFloat() const
	{
		return Type == Float;
	}

	bool IsString() const
	{
		return Type == String;
	}

	bool IsBytes() const
	{
		return Type == Bytes;
	}

	//
	void SetType(const std::string& type) 
	{
		if (type == "bool")
			Type = Bool;
		else if (type == "int")
			Type = Int;
		else if (type == "float")
			Type = Float;
		else if (type == "string")
		{
			Data.StringValue = new std::string();
			Type = String;
		}
		else if (type == "bytes")
		{
			Data.BytesValue = new std::vector<uint8>();
			Type = Bytes;
		}
		else
			Type = None;
	}

	std::string TypeToString() const
	{
		switch (Type)
		{
		case Bool:
			return "bool";
		case Int:
			return "int";
		case Float:
			return "float";
		case String:
			return "string";
		case Bytes:
			return "bytes";
		default:
			return "none";
		}
	}

	// Operators.
	friend Archive& operator << (Archive& archive, const Variant& argument)
	{
		std::string type = argument.TypeToString();
		archive << type;

		switch (argument.Type)
		{
		case Bool:
		{
			std::string value = argument.Data.BoolValue ? "true" : "false";
			archive << value;
			break;
		}
		case Int:
			archive << std::to_string(argument.Data.IntValue);
			break;
		case Float:
			archive << std::to_string(argument.Data.FloatValue);
			break;
		case String:
			archive << *argument.Data.StringValue;
			break;
		case Bytes:
			archive << *argument.Data.BytesValue;
			break;
		default:
			break;
		}

		return archive;
	}

	friend Archive& operator >> (Archive& archive, Variant& argument)
	{
		std::string type;
		archive >> type;
		argument.SetType(type);

		switch (argument.Type)
		{
		case Bool:
		{
			std::string value;
			archive >> value;
			argument.Data.BoolValue = value == "true" ? true : false;
			break;
		}
		case Int:
		{
			std::string value;
			archive >> value;
			argument.Data.IntValue = std::stoi(value);
			break;
		}
		case Float:
		{
			std::string value;
			archive >> value;
			argument.Data.FloatValue = std::stof(value);
			break;
		}
		case String:
		{
			archive >> *argument.Data.StringValue;
			break;
		}
		case Bytes:
		{
			archive >> *argument.Data.BytesValue;
			break;
		}
		default:
			break;
		}

		return archive;
	}
};

} // End vnet.