#pragma once

namespace utils
{
	template <typename T, size_t Size>
	class static_vector : std::vector<T>
	{
	public:
		static_vector()
		{
			std::vector<T>::resize(Size);
		}

		const T& operator[](const size_t index) const
		{
			return std::vector<T>::operator[](index);
		}

		T& operator[](const size_t index)
		{
			return std::vector<T>::operator[](index);
		}

		const T* data() const
		{
			return std::vector<T>::data();
		}

		T* data()
		{
			return std::vector<T>::data();
		}

		size_t data_size() const
		{
			return std::vector<T>::size() * sizeof(T);
		}

		size_t size() const
		{
			return std::vector<T>::size();
		}

	};
}
