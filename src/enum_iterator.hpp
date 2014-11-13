// Taken from http://stackoverflow.com/questions/8498300/allow-for-range-based-for-with-enum-classes
// CC-BY-SA license. Attributed to http://stackoverflow.com/users/28817/deft-code
#pragma once

template<typename T>
class Enum
{
public:
	class Iterator
	{
	public:
		Iterator(int value) : value_(value) {}

		T operator*(void) const
		{
			return (T)value_;
		}

		void operator++(void)
		{
			++value_;
		}

		bool operator!=(Iterator rhs) const
		{
			return value_ != rhs.value_;
		}

		private:
			int value_;
	};
};

template<typename T> inline
typename Enum<T>::Iterator begin(Enum<T>)
{
	return typename Enum<T>::Iterator(static_cast<int>(T::First));
}

template<typename T> inline
typename Enum<T>::Iterator end(Enum<T>)
{
	return typename Enum<T>::Iterator(static_cast<int>(T::Last) + 1);
}
