#ifndef TRIPLE_BUFFERING_H_
#define TRIPLE_BUFFERING_H_

#include <mutex>

template<typename T>
class TripleBufferedItem
{
	typedef T data_type;
public:
	TripleBufferedItem()
	{
	}

	data_type & ProducerValue()
	{
		return _producerValue;
	}

	data_type & ConsumerValue()
	{
		return _consumerValue;
	}

	void Commit()
	{
		std::lock_guard<std::mutex> lg(this->_mutex);
		this->_intermediateValue = this->_producerValue;
	}

	void Commit()
	{
		std::lock_guard<std::mutex> lg(this->_mutex);
		this->_consumerValue = this->_intermediateValue;
	}

protected:
private:
	std::mutex _mutex;
	data_type _producerValue;
	data_type _intermediateValue;
	data_type _consumerValue;
};

#endif
