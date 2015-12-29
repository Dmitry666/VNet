#pragma once

#include "Variant.h"

#include <list>
#include <algorithm>

namespace vnet {

/**
 * @brief Tree.
 */
struct PTree
{
    Variant Data;
	std::list<std::pair<std::string, PTree>> Children;

	PTree()
	{}

	PTree(const Variant& value)
        : Data(value)
    {}

	PTree(const PTree& tree)
        : Data(tree.Data)
        , Children(tree.Children)
	{}

	/**
	 * @brief Get value from name.
	 * @param key value key.
	 */
	Variant Get(const std::string& key) const
	{
		auto it = std::find_if(Children.begin(), Children.end(), [key](const std::pair<std::string, PTree>& pair){
			return pair.first == key;
		});

		if (it != Children.end())
			return it->second.Data;

		return Variant();
	}

	/**
	 * @brief Serialize pair list.
	 * @param archive archive buffer.
	 * @param pairs pair list.
	 * @return archive buffer.
	 */
	friend Archive& operator << (Archive& archive, const std::list<std::pair<std::string, PTree>>& pairs)
    {
        uint32 nbElements = pairs.size();
        archive << nbElements;
        for(auto& it : pairs)
        {
            archive << it.first;
            archive << it.second;
        }

        return archive;
    }

	/**
	 * @brief Deserialize pair list.
	 * @param archive archive buffer.
	 * @param pairs pair list.
	 * @return archive buffer.
	 */
	friend Archive& operator >> (Archive& archive, std::list<std::pair<std::string, PTree>>& pairs)
    {
        uint32 nbElements;
        archive >> nbElements;
        pairs.resize(nbElements);

        for(auto& it : pairs)
        {
			archive >> it.first;
            archive >> it.second;
        }

        return archive;
    }

	/**
	 * @brief Serialize property tree.
	 * @param archive archive buffer.
	 * @param tree property tree.
	 * @return archive buffer.
	 */
	friend Archive& operator << (Archive& archive, const PTree& tree)
    {
        archive << tree.Data;
        archive << tree.Children;

        return archive;
    }

	/**
	 * @brief Deserialize property tree.
	 * @param archive archive buffer.
	 * @param tree property tree.
	 * @return archive buffer.
	 */
	friend Archive& operator >> (Archive& archive, PTree& tree)
    {
        archive >> tree.Data;
        archive >> tree.Children;

        return archive;
    }

    //
	void Put(const std::string& key, const Variant& value)
    {
		Children.push_back(std::make_pair(key, PTree(value)));
    }

	void Put(const std::list<std::pair<std::string, PTree>>& values)
	{
		Children.insert(Children.end(), values.cbegin(), values.cend());
	}

	void Put(const std::string& key, const PTree& tree)
    {
        Children.push_back(std::make_pair(key, tree));
    }

	PTree& operator += (const PTree& tree)
	{
		Put(tree.Data.ToString(), tree);
		return (*this);
	}

	PTree& operator += (const std::list<std::pair<std::string, PTree>>& values)
	{
		Put(values);
		return (*this);
	}
};

} // End vnet.
