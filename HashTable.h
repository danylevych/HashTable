#pragma once

#include <string>
#include <memory>
#include <initializer_list>


template<typename Type>
struct HashFunction
{
	size_t operator()(Type key)
	{
		size_t hash = static_cast<size_t>(key) << (sizeof(Type) * 8);
		hash *= 0xf883a77aa; // "random" number.
		return (hash ^ (hash >> 32));
	}
};

template<>
struct HashFunction<std::string>
{
	size_t operator()(const std::string& key)
	{
		size_t hash = 5381;

		for (auto item : key)
		{
			hash = ((hash << 5) + hash) + static_cast<size_t>(item);
		}

		return hash;
	}
};



template<typename KeyType, typename ValType>
class HashTable
{
private:
	/*
		This is a part of all classes.
	*/

	class iteratorBase;
	class iterator;
	class const_iterator;
	class reverse_iterator;
	class const_reverse_iterator;


private:
	size_t size;
	size_t sizeOfBucket = 13;

	class Node
	{
	public:
		KeyType key;
		ValType value;
		Node* next;
		Node* previous;

	public:
		Node(KeyType key, ValType value, Node* previous = nullptr, Node* next = nullptr)
			: key(key), value(value), previous(previous), next(next)
		{	}
	};

	Node** table;


public:
	HashTable()
		: table(new Node* [sizeOfBucket] {}), size(0)
	{	}

	HashTable(const std::initializer_list<std::pair<const KeyType, ValType>>& list)
		: table(new Node* [sizeOfBucket] {}), size(list.size())
	{
		for (auto& item : list)
		{
			Insert(item);
		}
	}

	HashTable(const HashTable& other)
	{
		Copy(other);
	}

	HashTable(HashTable&& other)
	{
		MoveTabel(other);
	}

	~HashTable()
	{
		Clear();
		delete[] table;
	}


public:
	size_t Size() const { return size; }

	void Clear()
	{
		for (size_t i = 0; i < sizeOfBucket; i++)
		{
			Node* head = table[i];
			table[i] = nullptr;

			while (head)
			{
				Node* temp = head;
				head = head->next;
				delete temp;
			}
		}

		size = 0;
	}

	bool Empty() const { return size == 0; }

	iterator Find(const KeyType& key)
	{
		size_t index = HashFunction<KeyType>()(key) % sizeOfBucket;

		Node*& current = table[index];

		if (current == nullptr)
		{
			return iterator();
		}

		while (current->next != nullptr && current->key != key)
		{
			current = current->next;
		}

		if (current->key == key)
		{
			return iterator(table, current, sizeOfBucket, index);
		}
		else
		{
			return iterator();
		}
	}

	const_iterator Find(const KeyType& key) const
	{
		size_t index = HashFunction<KeyType>()(key) % sizeOfBucket;

		Node*& current = table[index];

		if (current == nullptr)
		{
			return const_iterator();
		}

		while (current->next != nullptr && current->key != key)
		{
			current = current->next;
		}

		if (current->key == key)
		{
			return const_iterator(table, current, sizeOfBucket, index);
		}
		else
		{
			return const_iterator();
		}
	}

	template<typename ...Arg>
	void Emplace(const KeyType& key, Arg ... args)
	{
		size_t index = HashFunction<KeyType>()(key) % sizeOfBucket;

		if (table[index] == nullptr)
		{
			table[index] = new Node(key, args...);
			return;
		}

		Node*& current = table[index];

		while (current->next != nullptr)
		{
			if (current->key == key)  // The key exist.
			{
				current->value = ValType(args...);
				return;
			}
			current = current->next;
		}

		current->next = new Node(key, ValType(args...), current);

		size++;
	}

	std::pair<bool, iterator> Insert(const_iterator& iter)
	{
		return Insert(iter->first, iter->second);
	}

	std::pair<bool, iterator> Insert(const KeyType& key, ValType value)
	{
		size_t index = HashFunction<KeyType>()(key) % sizeOfBucket;

		if (table[index] == nullptr)
		{
			table[index] = new Node(key, value);
			return std::make_pair(true, iterator(table, table[index], sizeOfBucket, index));
		}

		Node*& current = table[index];

		while (current->next != nullptr)
		{
			if (current->key == key)  // The key exist.
			{
				current->value = value;
				return std::make_pair(false, iterator(table, current, sizeOfBucket, index));
			}
			current = current->next;
		}

		current->next = new Node(key, value, current);

		size++;

		return std::make_pair(true, iterator(table, current->next, sizeOfBucket, index));
	}

	std::pair<bool, iterator> Insert(const std::pair<const KeyType, ValType>& insertPair)
	{
		return Insert(insertPair.first, insertPair.second);
	}

	bool Erase(const KeyType& key)
	{
		size_t index = HashFunction<KeyType>()(key) % sizeOfBucket;

		Node* current = table[index];
		Node* previous = current;

		if (current == nullptr)
		{
			return false;
		}

		while (current->next != nullptr && current->key != key)  // Finding a deleting node.
		{
			previous = current;
			current = current->next;
		}

		if (current->key == key)  // The node was found.
		{
			if (previous == current)  // The deleting node is the first in bucket.
			{
				table[index] = current->next;
				if(current->next != nullptr)  // Deleting node has a next node.
				{
					table[index]->previous = nullptr;
				}

				delete current;
			}
			else if (current->next == nullptr)  // Deleting node is a final node in the chain.
			{
				previous->next = nullptr;
				delete current;
			}
			else  // Deleting node is a chain middle node.
			{
				previous->next = current->next;
				current->next->previous = previous;
				
				delete current;
			}

			size--;
			return true;
		}
		else
		{
			return false;
		}
	}

	bool Erase(iterator& iter)
	{
		if (!iter)  // The node doesn't exist.
		{
			return false;
		}

		Node* current = iter.iteratorBase::node;  // Taking the iterator pointing node.
		
		if (current->previous == nullptr)  // The deleting node is the first in bucket.
		{
			size_t index = HashFunction<KeyType>()(iter->first) % sizeOfBucket;

			table[index] = current->next;
			if (current->next != nullptr)  // Deleting node has a next node.
			{
				table[index]->previous = nullptr;
			}

			delete current;
		}
		else if (current->next == nullptr)  // Deleting node is a final node in the chain.
		{
			current->previous->next = nullptr;

			delete current;
		}
		else // Deleting node is a chain middle node.
		{
			current->previous->next = current->next;
			current->next->previous = current->previous;

			delete current;
		}

		size--;
		iter = ++iter;  // Moving iterator to the next node.
		return true;
	}


public:
	// ---------------------- OVERLODING OPERATORS ---------------------- 
	ValType& operator[](const KeyType& key)
	{
		size_t index = HashFunction<KeyType>()(key) % sizeOfBucket;

		Node*& current = table[index];

		if (current == nullptr)  // If the bucket array does not contain the key.
		{
			Insert(key, ValType{});
			return current->value;
		}

		while (current->next != nullptr && current->key != key)
		{
			current = current->next;
		}

		if (current->key == key)
		{
			return current->value;
		}
		else
		{
			Insert(key, ValType{});  // Not found - add to table.
		}
	}

	const ValType& operator[](const KeyType& key) const
	{
		size_t index = HashFunction<KeyType>()(key) % sizeOfBucket;

		Node*& current = table[index];

		if (current == nullptr)  // If the key in a buckt, that doesn't exist.
		{
			throw std::out_of_range("The key doesn't exist.");
		}

		while (current->next != nullptr && current->key != key)
		{
			current = current->next;
		}

		if (current->key == key)
		{
			return current->value;
		}
		else
		{
			throw std::out_of_range("The key doesn't exist.");  // Not found - throw an exception.
		}
	}

	HashTable& operator=(HashTable&& other)
	{
		if (this != other)
		{
			MoveTable(other);
		}
		return *this;
	}

	HashTable& operator=(const HashTable& other)
	{
		if (this != other)
		{
			Copy(other);
		}
		return *this;
	}


#pragma region ITERATORS CLASSES 
private:
	class iteratorBase
	{
	private:
		friend class HashTable;

	protected:
		size_t bucketID;
		size_t sizeOfBucket;
		Node* node;
		Node** table;

	public:
		iteratorBase(Node** table = nullptr, Node* node = nullptr, size_t sizeOfBucket = 0, size_t bucketID = 0)
			: table(table),
			node(node),
			sizeOfBucket(sizeOfBucket),
			bucketID(bucketID)
		{	}

		virtual iteratorBase& operator++() = 0;
		virtual iteratorBase& operator--() = 0;


		virtual std::pair<const KeyType, ValType> operator*()
		{
			if (node == nullptr)
			{
				throw std::out_of_range("You want to access the null value");
			}

			return std::make_pair(node->key, node->value);
		}

		virtual std::unique_ptr<std::pair<const KeyType&, ValType&>> operator->()
		{
			if (node == nullptr)
			{
				throw std::out_of_range("You want to access the null value");
			}

			return std::make_unique<std::pair<const KeyType&, ValType&>>(node->key, node->value);
		}

		virtual operator bool() const
		{
			return node != nullptr;
		}

		virtual bool operator==(std::nullptr_t right)
		{
			return node == right;
		}

		virtual ~iteratorBase() = default;
	};


public:
	class iterator : public iteratorBase
	{
	protected:
		bool isEnd;  // The tag say this object pointing on the end of container.

	public:
		iterator(Node** table = nullptr, Node* node = nullptr, size_t sizeOfBucket = 0, size_t bucketID = 0, bool isEnd = false)
			:
			iteratorBase(table, node, sizeOfBucket, bucketID),
			isEnd(isEnd)
		{	}

		virtual iterator& operator++() override
		{
			if (iteratorBase::node == nullptr)  // Access to doesn't existing value.
			{
				throw std::out_of_range("You want to access the null value");
			}

			if (iteratorBase::node->next == nullptr && iteratorBase::bucketID < iteratorBase::sizeOfBucket - 1)
			{
				// Move current node to next bucket in array.
				iteratorBase::node = iteratorBase::table[++iteratorBase::bucketID];
				while (iteratorBase::node == nullptr && iteratorBase::bucketID < iteratorBase::sizeOfBucket - 1)
				{
					iteratorBase::node = iteratorBase::table[++iteratorBase::bucketID];
				}
			}
			else
			{
				iteratorBase::node = iteratorBase::node->next;
			}

			return *this;
		}

		virtual iterator& operator--() override
		{
			if (isEnd)  // Skip one decrementing.
			{
				isEnd = false;
				return *this;
			}

			if (iteratorBase::node == nullptr)  // Access to doesn't existing value.
			{
				throw std::out_of_range("You want to access the null value");
			}

			if (iteratorBase::node->previous == nullptr && iteratorBase::bucketID > 1)
			{
				// Move current node to previous bucket in array.
				iteratorBase::node = iteratorBase::table[--iteratorBase::bucketID];
				while (iteratorBase::node == nullptr && iteratorBase::bucketID > 1)
				{
					iteratorBase::node = iteratorBase::table[--iteratorBase::bucketID];
				}
			}
			else
			{
				iteratorBase::node = iteratorBase::node->previous;
			}

			return *this;
		}

		virtual bool operator==(const iterator& right)
		{
			if (isEnd)
			{
				return iteratorBase::node->next == right.node;
			}
			else if (right.isEnd)
			{
				return right.node->next == iteratorBase::node;
			}

			return iteratorBase::node == right.node;
		}

		virtual bool operator!=(const iterator& right)
		{
			return !(*this == right);
		}

		virtual ~iterator() = default;
	};


	class const_iterator : public iterator
	{
	public:
		const_iterator(Node** table = nullptr, Node* node = nullptr, size_t sizeOfBucket = 0, size_t bucketID = 0, bool isEnd = false)
			: iterator(table, node, sizeOfBucket, bucketID, isEnd)
		{	}

		std::pair<const KeyType, const ValType> operator*() const
		{
			if (iteratorBase::node == nullptr)
			{
				throw std::out_of_range("You want to access the null value");
			}

			return std::make_pair(iteratorBase::node->key, iteratorBase::node->value);
		}

		std::unique_ptr<std::pair<const KeyType&, const ValType&>> operator->() const
		{
			if (iteratorBase::node == nullptr)
			{
				throw std::out_of_range("You want to access the null value");
			}

			return std::make_unique<std::pair<const KeyType&, const ValType&>>(iteratorBase::node->key, iteratorBase::node->value);
		}
	};


	class reverse_iterator : public iteratorBase
	{
	protected:
		bool isBegin;  // The tag say this object pointing on the begin of container.

	public:
		reverse_iterator(Node** table = nullptr, Node* node = nullptr, size_t sizeOfBucket = 0, size_t bucketID = 0, bool isBegin = false)
			:
			iteratorBase(table, node, sizeOfBucket, bucketID),
			isBegin(isBegin)
		{	}

		virtual reverse_iterator& operator--() override
		{
			if (iteratorBase::node == nullptr)  // Access to doesn't existing value.
			{
				throw std::out_of_range("You want to access the null value");
			}

			if (iteratorBase::node->next == nullptr && iteratorBase::bucketID < iteratorBase::sizeOfBucket - 1)
			{
				// Move current node to next bucket in array.
				iteratorBase::node = iteratorBase::table[++iteratorBase::bucketID];
				while (iteratorBase::node == nullptr && iteratorBase::bucketID < iteratorBase::sizeOfBucket - 1)
				{
					iteratorBase::node = iteratorBase::table[++iteratorBase::bucketID];
				}
			}
			else
			{
				iteratorBase::node = iteratorBase::node->next;
			}

			return *this;
		}

		virtual reverse_iterator& operator++() override
		{
			if (isBegin)  // Skip one inrementing.
			{
				isBegin = false;
				return *this;
			}

			if (iteratorBase::node == nullptr)  // Access to doesn't existing value.
			{
				throw std::out_of_range("You want to access the null value");
			}

			if (iteratorBase::node->previous == nullptr && iteratorBase::bucketID > 1)
			{
				// Move current node to previous bucket in array.
				iteratorBase::node = iteratorBase::table[--iteratorBase::bucketID];
				while (iteratorBase::node == nullptr && iteratorBase::bucketID > 1)
				{
					iteratorBase::node = iteratorBase::table[--iteratorBase::bucketID];
				}
			}
			else
			{
				iteratorBase::node = iteratorBase::node->previous;
			}

			return *this;
		}

		virtual bool operator==(const reverse_iterator& right)
		{
			if (isBegin)
			{
				return iteratorBase::node->previous == right.node;
			}
			else if (right.isBegin)
			{
				return right.node->previous == iteratorBase::node;
			}

			return iteratorBase::node == right.node;
		}

		virtual bool operator!=(const reverse_iterator& right)
		{
			return !(*this == right);
		}

		virtual ~reverse_iterator() = default;
	};


	class const_reverse_iterator : public iterator
	{
	public:
		const_reverse_iterator(Node** table = nullptr, Node* node = nullptr, size_t sizeOfBucket = 0, size_t bucketID = 0, bool isBegin = false)
			: reverse_iterator(table, node, sizeOfBucket, bucketID, isBegin)
		{	}

		std::pair<const KeyType, const ValType> operator*() const
		{
			if (iteratorBase::node == nullptr)
			{
				throw std::out_of_range("You want to access the null value");
			}

			return std::make_pair(iteratorBase::node->key, iteratorBase::node->value);
		}

		std::unique_ptr<std::pair<const KeyType&, const ValType&>> operator->() const
		{
			if (iteratorBase::node == nullptr)
			{
				throw std::out_of_range("You want to access the null value");
			}

			return std::make_unique<std::pair<const KeyType&, const ValType&>>(iteratorBase::node->key, iteratorBase::node->value);
		}
	};

#pragma endregion ITERATORS CLASSES 


public:
	// ---------------------- ITERATOR METHODS ----------------------  
	iterator begin()
	{
		auto result = FindFirst();

		return iterator(table, result.first, sizeOfBucket, result.second);
	}

	iterator end()
	{
		auto result = FindLast();

		return iterator(table, result.first, sizeOfBucket, result.second, true);
	}

	const_iterator cbegin() const
	{
		auto result = FindFirst();

		return const_iterator(table, result.first, sizeOfBucket, result.second);
	}

	const_iterator cend() const
	{
		auto result = FindLast();

		return const_iterator(table, result.first, sizeOfBucket, result.second);
	}


	reverse_iterator rbegin()
	{
		auto result = FindLast();

		return reverse_iterator(table, result.first, sizeOfBucket, result.second);
	}

	reverse_iterator rend()
	{
		auto result = FindFirst();

		return reverse_iterator(table, result.first, sizeOfBucket, result.second, true);
	}

	const_reverse_iterator crbegin() const
	{
		auto result = FindLast();

		return const_reverse_iterator(table, result.first, sizeOfBucket, result.second);
	}

	const_reverse_iterator crend() const
	{
		auto result = FindFirst();

		return const_reverse_iterator(table, result.first, sizeOfBucket, result.second, true);
	}


private:
	/*
		The methods required by HashTable to reduce the code.
	*/

	std::pair<Node*, size_t> FindFirst() const
	{
		if (!Empty())
		{
			Node* current = nullptr;

			for (size_t i = 0; i < sizeOfBucket; i++)
			{
				current = table[i];
				if (current != nullptr)
				{
					return std::make_pair(current, i);
				}
			}
		}

		throw std::out_of_range("The table is empty, you cannot access to values that doesn't exist.");
	}

	std::pair<Node*, size_t> FindLast() const
	{
		if (!Empty())
		{
			Node* current = nullptr;

			for (size_t i = sizeOfBucket - 1; i >= 0; i--)
			{
				current = table[i];
				if (current != nullptr)
				{
					while (current->next != nullptr)
					{
						current = current->next;
					}

					return std::make_pair(current, i);
				}
			}
		}

		throw std::out_of_range("The table is empty, you cannot access to values that doesn't exist.");
	}

	void Copy(const HashTable& other)
	{
		Clear();
		delete[] table;

		this->sizeOfBucket = other.sizeOfBucket;
		this->table = new Node * [other.sizeOfBucket];

		for (auto iter = other.cbegin(), end = other.cend(); iter != end; ++iter)
		{
			Insert(iter);
		}
	}

	void MoveTabel(HashTable&& other)
	{
		Clear();
		delete[] table;

		this->size = other.size;
		this->sizeOfBucket = other.sizeOfBucket;
		this->table = other.table;

		other.size = 0;
		other.sizeOfBucket = 0;
		other.table = nullptr;
	}
};