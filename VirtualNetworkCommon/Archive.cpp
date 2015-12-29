// Core. 
//
// Author: Oznabikhin Dmitry
// Email: cmepthik@inbox.ru
//
// Copyright (c) GameX Corporation. All rights reserved.

#include "Archive.h"

namespace vnet {

Archive& Archive::operator >> ( std::string& value )
{
	if(IsSaving())
	{
		uint32 nbSymbols = value.size();
		ByteSerialize(&nbSymbols, sizeof(uint32));

		for (uint32 i = 0; i < value.size(); ++i)
		{
			char c = value[i];
			ByteSerialize(&c, sizeof(char));
		}
	}
	else if(IsLoading())
	{
		uint32 nbSymbols = 0;
		ByteSerialize(&nbSymbols, sizeof(uint32));
		value.resize(nbSymbols);

		for (uint32 i = 0; i < nbSymbols; ++i)
		{
			char c;
			ByteSerialize(&c, sizeof(char));
			value[i] = c;
		}		
	}

	return (*this);
}

Archive& Archive::operator<<(const std::string& value)
{
	if(IsSaving())
	{
        uint32 nbSymbols = value.size();
        ByteSerialize(&nbSymbols, sizeof(uint32));

		for (uint32 i = 0; i < value.size(); ++i)
		{
			char c = value[i];
			ByteSerialize(&c, sizeof(char));
		}		
	}

	return (*this);
}

}