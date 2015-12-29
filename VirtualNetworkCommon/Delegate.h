#pragma once

#include "Common.h"

namespace vnet {

template<typename ...Types>
class Delegate
{
    template<typename ...InnerTypes>
	class IConteiner
	{
	public:
		virtual ~IConteiner()
		{}

		virtual void Invoke(InnerTypes ...args) = 0;
	};

    template<typename TFunc, typename ...InnerTypes>
	class Conteiner
        : public IConteiner<InnerTypes...>
	{
	public:
		Conteiner(TFunc func)
			: _func(func)
		{}

		virtual void Invoke(InnerTypes ...args) override
		{
			_func(args...);
		}

	protected:
		TFunc _func;
	};

public:
	~Delegate()
	{
		Clear();
	}

	void Clear()
	{
		for(IConteiner<Types...>* conteiner : _conteiners)
			delete conteiner;
		_conteiners.clear();
	}

	void Invoke(Types ... args)
	{
		for(IConteiner<Types...>* conteiner : _conteiners)
			conteiner->Invoke(args...);
	}
	
	Delegate& operator ()(Types ... args)
    {
        Call(args...);
        return *this;
    }

	template<typename TFunc>
	Delegate& operator += (TFunc func)
	{
		auto conteiner = new Conteiner<TFunc, Types...>(func);
		_conteiners.push_back(conteiner);

		return *this;
	}

	template<typename TFunc>
	Delegate& operator = (TFunc func)
	{
		Clear();

		return (*this) += func;
	}

protected:
	std::vector<IConteiner<Types...>*> _conteiners;
};

} // End vnet.
