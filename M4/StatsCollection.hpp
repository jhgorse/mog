///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file StatsCollection.hpp
///
/// Copyright (c) 2014, BoxCast, Inc. All rights reserved.
///
/// This library is free software; you can redistribute it and/or modify it under the terms of the
/// GNU Lesser General Public License as published by the Free Software Foundation; either version
/// 3.0 of the License, or (at your option) any later version.
///
/// This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
/// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
/// the GNULesser General Public License for more details.
///
/// You should have received a copy of the GNU Lesser General Public License along with this
/// library; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
/// Boston, MA 02110-1301 USA
///
/// @brief This file declares the StatsCollection class, which tracks a single statistic over some
/// period of time defined by the collection capacity and frequency of entry.
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __STATS_COLLECTION_HPP__
#define __STATS_COLLECTION_HPP__

#include <cmath>   // for math stuff
#include <cstddef> // for standard type declarations


///////////////////////////////////////////////////////////////////////////////////////////////////
/// The StatsCollection class collects a single statistic and provides the average and standard
/// deviation of that statistic as they are tracked.
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T>
class StatsCollection
{
public:

	/// Constructor.
	inline explicit StatsCollection(size_t capacity)
	 : m_Storage(new T[capacity])
	 , m_Capacity(capacity)
	 , m_Count(0)
	 , m_InsertIndex(0)
	 , m_RemoveIndex(0)
	 , m_Total(0)
	 , m_TotalSquared(0)
	 , m_Average(0)
	 , m_StdDev(0)
	{ }


	/// Destructor
	inline ~StatsCollection()
	{
		delete[] m_Storage;
	}


	/// Capacity accessor
	inline size_t Capacity() const { return m_Capacity; }


	/// Count accessor
	inline size_t Count() const { return m_Count; }
	
	
	/// Whether or not the collection is full
	inline bool IsFull() const { return m_Count == m_Capacity; }


	/// Average accessor
	inline T Average() const { return m_Average; }


	/// Standard deviation accessor
	inline T StandardDeviation() const { return m_StdDev; }


	/// Insert an item into the collection
	void Insert(const T& item)
	{
		T oldTotal;
		T oldTotalSquared;
		
		// First see if we have to remove one
		if (IsFull())
		{
			T itemRemoved = m_Storage[m_RemoveIndex];

			// Subtract the removed item from the totals
			oldTotal = m_Total;
			oldTotalSquared = m_TotalSquared;
			m_Total -= itemRemoved;
			m_TotalSquared -= (itemRemoved * itemRemoved);
			
			// Ensure we didn't overflow; if we did, just clear everything for now.
			if ((m_Total > oldTotal) || (m_TotalSquared > oldTotalSquared))
			{
				Clear();
				return;
			}

			// Increment the remove index & decrement the count
			m_RemoveIndex = (m_RemoveIndex + 1) % m_Capacity;
			m_Count--;
		}

		// Insert the new item
		m_Storage[m_InsertIndex] = item;

		// Add the new item to the totals
		oldTotal = m_Total;
		oldTotalSquared = m_TotalSquared;
		m_Total += item;
		m_TotalSquared += (item * item);
		
		// Ensure we didn't overflow; if we did, just clear everything for now.
		if ((m_Total < oldTotal) || (m_TotalSquared < oldTotalSquared))
		{
			Clear();
			return;
		}

		// Increment the insert index and increment the count
		m_InsertIndex = (m_InsertIndex + 1) % m_Capacity;
		m_Count++;

		// Update the running stats
		m_Average = m_Total / m_Count;
		m_StdDev = static_cast<T>(sqrt((m_TotalSquared * m_Count) - (m_Total * m_Total)) / m_Count);
	} // END Insert()
	
	
	// Clear (empty) the collection
	void Clear()
	{
		m_Count = 0;
		m_InsertIndex = 0;
		m_RemoveIndex = 0;
		m_Total = 0;
		m_TotalSquared = 0;
		m_Average = 0;
		m_StdDev = 0;
	}

protected:

private:

	/// The actual storage for items
	T * const m_Storage;


	/// The number of items this stats collection can hold
	const size_t m_Capacity;


	/// The number of items currently in the collection
	size_t m_Count;


	/// The insert index
	size_t m_InsertIndex;
	
	
	/// The remove index
	size_t m_RemoveIndex;


	/// The running total
	T m_Total;
	
	
	/// The running total squared
	T m_TotalSquared;


	/// The running average
	T m_Average;
	
	
	/// The running standard deviation
	T m_StdDev;
};

#endif // __STATS_COLLECTION_HPP__
