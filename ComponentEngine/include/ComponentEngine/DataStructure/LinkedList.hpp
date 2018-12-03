#pragma once

namespace ComponentEngine
{
	template<typename T>
	struct LinkedList<T>;

	template<typename T>
	struct LinkedListNode
	{
		LinkedListNode(T d) : data(d) {}
		T data;
	private:
		friend LinkedList<T>;
		LinkedListNode* next = nullptr;
	};
	template<typename T>
	class LinkedList
	{
	public:
		bool Empty()
		{
			return m_head == nullptr;
		}
		void Push(T data)
		{
			LinkedListNode<T>* node = new LinkedListNode<T>(data);
			if (m_foot != nullptr)
			{
				m_foot->next = node;
			}
			m_foot = node;
			if (m_head == nullptr)
			{
				m_head = node;
			}
		}
		T Pop()
		{
			LinkedListNode<T>* next = m_head->next;
			T data = m_head->data;
			if (next == nullptr)
			{
				m_foot = nullptr;
				delete m_head;
				m_head = nullptr;
			}
			else
			{
				delete m_head;
				m_head = next;
			}
			return data;
		}
	private:
		LinkedListNode* m_head = nullptr;
		LinkedListNode* m_foot = nullptr;
	};
}