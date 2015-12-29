#pragma once

#include "Common.h"

namespace vnet {


/**
 * @brief Base serialization buffer.
 *
 * Base class from serialization resources.
 */
class Archive
{
public:
	virtual ~Archive() {}

	virtual void Seek( uint32 ){;}

    /**
     * @brief Get cursor current position.
     * @return bytes offset.
     */
    virtual uint32 GetPosition() const { return 0;}
	uint32 Tell() const
	{
		return this->GetPosition();
	}

    /**
     * @brief Get archive size in bytes.
     * @return size in bytes.
     */
    virtual uint32 GetSize() const { return 0;}

    // TODO BAd syntax.
	virtual uint32 Position(){ return 0;}
	virtual uint32 Size(){ return 0;}

	virtual uint32 Position() const { return 0;}
	virtual uint32 Size() const { return 0;}
    // TODO. End Bad syntax.

	virtual void Serialize(void*, uint32){;}
	virtual void Close(){;}

public:
	/**
		Operators.
	*/
	Archive& operator >> (bool& value)
	{
		ByteSerialize(&value, sizeof(bool));
		return (*this);
	}

	Archive& operator << (const bool& value)
	{
		if (IsSaving())
		{
			bool nonConst = value;
			ByteSerialize(&nonConst, sizeof(bool));
		}

		return (*this);
	}

	//Archive& operator<<(BYTE& value)
	//{
	//	ByteSerialize(&value, sizeof(BYTE));
	//	return (*this);
	//}

	Archive& operator >> (char& value)
	{
		ByteSerialize(&value, sizeof(char));
		return (*this);
	}
	
	Archive& operator << (const char& value)
	{
		if (IsSaving())
		{
			char nonConst = value;
			ByteSerialize(&nonConst, sizeof(char));	
		}

		return (*this);
	}

	Archive& operator >> (uint8& value)
	{
		ByteSerialize(&value, sizeof(char));
		return (*this);
	}

	Archive& operator << (const uint8& value)
	{
		if (IsSaving())
		{
			uint8 nonConst = value;
			ByteSerialize(&nonConst, sizeof(uint8));
		}

		return (*this);
	}

	Archive& operator >> (int32& value)
	{
		ByteSerialize(&value, sizeof(int32));
		return (*this);
	}

	Archive& operator << (const int32& value)
	{
		if(IsSaving())
		{
			int32 nonConst = value;
			ByteSerialize(&nonConst, sizeof(int32));
		}

		return (*this);
	}

	//Archive& operator<<(DWORD& value)
	//{
	//	ByteSerialize(&value ,sizeof(DWORD));
	//	return (*this);
	//}

	Archive& operator >> (uint32& value)
	{
		ByteSerialize(&value, sizeof(uint32));
		return (*this);
	}

	Archive& operator << (const uint32& value)
	{
		if(IsSaving())
		{
			uint32 nonConst = value;
			ByteSerialize(&nonConst, sizeof(uint32));
		}

		return (*this);
	}

	Archive& operator >> (float& value)
	{
		ByteSerialize(&value, sizeof(float));
		return (*this);
	}

	Archive& operator << (const float& value)
	{
		if(IsSaving())
		{
			float nontConst = value;
			ByteSerialize(&nontConst, sizeof(float));
		}

		return (*this);
	}

	Archive& operator >> (double& value)
	{
        ByteSerialize(&value, sizeof(double));
		return (*this);
	}

	Archive& operator << (const double& value)
    {
        if(IsSaving())
        {
            double nontConst = value;
            ByteSerialize(&nontConst, sizeof(double));
        }

        return (*this);
    }


    /**
     * @brief Serialize string..
     * @param buf archive buffer.
     * @param value
     * @return archive buffer.
     */
	virtual Archive& operator >> (std::string& value);

    /**
     * @brief Serialize only from const.
     * @param buf archive buffer.
     * @param value
     * @return archive buffer.
     */
	virtual Archive& operator << (const std::string& value);

	template<typename Type>
	Archive& operator >> (std::vector<Type>& value)
	{
		uint32 nbElements;
		ByteSerialize(&nbElements, sizeof(uint32));

		value.resize(nbElements);
		for (uint32 i = 0; i < nbElements; ++i)
		{
			(*this) >> value[i];
		}

		return (*this);
	}

	template<typename Type>
	Archive& operator << (const std::vector<Type>& value)
	{
		if (IsSaving())
		{
			uint32 nbElements = value.size();
			ByteSerialize(&nbElements, sizeof(uint32));

			for (uint32 i = 0; i < nbElements; ++i)
			{
				(*this) << value[i];
			}
		}

		return (*this);
	}

	//_Export friend FBuffer& operator<<( FBuffer& buf, CHAR** value );

	Archive& ByteSerialize(void* buf, uint32 length)
	{
		for( uint32 i=0; i<length; i++ )
			Serialize( static_cast<uint8*>(buf) + i, 1 );
		return *this;
	}

	virtual bool IsSaving()
	{
		return _saving;
	}

	virtual bool IsLoading()
	{
		return !_saving;
	}

protected:
	//uint32 _size;
	bool _saving;
};

} // End vnet.
