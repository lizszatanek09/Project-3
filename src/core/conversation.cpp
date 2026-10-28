#include <stdexcept>

#include "core/conversation.h"

Conversation::Conversation() : data_(nullptr), size_(0), capacity_(0){}

Conversation::~Conversation(){
    delete[] data_;
}

std::size_t Conversation::size() const noexcept{
    return size_;
}

const Message* Conversation::begin() const noexcept{
    return data_;
}

const Message* Conversation::end() const noexcept{
    if(data_ == nullptr) {
        return nullptr;
    }

    return data_ + size_;
}

void Conversation::append(Message m){

    if (size_ == capacity_)
    {
        std::size_t new_capacity;

        if (capacity_ == 0){
            new_capacity = 1;
        }

        else{
            new_capacity = capacity_ * 2;
        }

        Message* new_mess = new Message[new_capacity];

        for (std::size_t i = 0; i < size_; ++i){
            new_mess[i] = data_[i];
        }

        delete[] data_;
        data_ = new_mess;
        capacity_ = new_capacity;
    }

    data_[size_] = m;
    ++size_;
}

const Message& Conversation::at(std::size_t i) const{
    if (i >= size_){
        throw std::out_of_range("Index out of range");
    }
    return data_[i];
}

// Copy constuctor
Conversation::Conversation(const Conversation& convo) : data_(nullptr), size_(convo.size_), capacity_(convo.capacity_){
    if (capacity_ > 0){
        data_ = new Message[capacity_];

        for (std::size_t i = 0; i < size_; ++i){
            data_[i] = convo.data_[i];
        }
    }
}

// Copy assignment
Conversation& Conversation::operator=(const Conversation& convo){
    if (this == &convo){
        return *this;
    }

    Message* new_data = nullptr;

    if (convo.capacity_ > 0){
        new_data = new Message[convo.capacity_];

        for (std::size_t i = 0; i < convo.size_; ++i){
            new_data[i] = convo.data_[i];
        }
    }

    delete[] data_;
    data_ = new_data;
    size_ = convo.size_;
    capacity_ = convo.capacity_;
    return *this;
}

// Move constructor 
Conversation::Conversation(Conversation&& convo) noexcept : data_(convo.data_), size_(convo.size_), capacity_(convo.capacity_){
    convo.data_ = nullptr;
    convo.size_ = 0;
    convo.capacity_ = 0;
}

// Move assignment
Conversation& Conversation::operator=(Conversation&& convo) noexcept{
    if (this == &convo){
        return *this;
    }

    delete[] data_;
    data_ = convo.data_;
    size_ = convo.size_;
    capacity_ = convo.capacity_;
    convo.data_ = nullptr;
    convo.size_ = 0;
    convo.capacity_ = 0;
    return *this;
}

