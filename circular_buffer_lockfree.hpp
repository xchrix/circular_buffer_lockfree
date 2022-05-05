#pragma once

#include <stdint.h>
#include <atomic>
#include <algorithm>
#include <cstring>

namespace chrix
{
    template <typename T, size_t T_SIZE = sizeof(T)>
    class circular_buffer_lockfree
    {
    public:
        typedef circular_buffer_lockfree<T, T_SIZE> this_type;
        typedef T value_type;
        typedef T *pointer;
        typedef const T *const_pointer;
        typedef value_type &reference;
        typedef const value_type &const_reference;
        // typedef T&
        circular_buffer_lockfree(size_t size) : _in(0), _out(0)
        {
            _capacity = roundup_pow_of_two(size);
            _buffer = new value_type[_capacity];
        }

        ~circular_buffer_lockfree()
        {
            if (!_buffer)
                delete[] _buffer;
        }

        size_t Put(const_pointer buffer, size_t len)
        {
            len = std::min(len, _capacity - (_in - _out.load(std::memory_order_acq_rel)));
            // len = std::min(len, _capacity - (_in - _out));
            //  通用内存屏障，保证out读到正确的值，可能另外一个线程在修改out
            //  用smp_mb是因为上面读out，下面写_buffer
            //  smp_mb();

            size_t l = std::min(len, _capacity - (_in & (_capacity - 1)));
            memcpy(_buffer + (_in & (_capacity - 1)), buffer, l * sizeof(value_type));
            memcpy(_buffer, buffer + l, (len - l) * sizeof(value_type));
            // 写内存屏障，保证先写完_buffer，再修改in
            // smp_wmb();
            _in.fetch_add(len, std::memory_order_release);
            //_in += len;

            return len;
        }

        size_t Get(pointer buffer, size_t len)
        {
            len = std::min(len, _in.load(std::memory_order_acquire) - _out);
            // 读内存屏障，保证读到正确的in，可能另外一个线程正在修改in
            // 用smp_rmb是因为上面读in，下面读_buffer
            // smp_rmb();

            size_t l = std::min(len, _capacity - (_out & (_capacity - 1)));
            memcpy(buffer, _buffer + (_out & (_capacity - 1)), l * sizeof(value_type));
            memcpy(buffer + l, _buffer, (len - l) * sizeof(value_type));
            // 通用内存屏障，保证先把buffer读出去，再修改out
            // 上面读_bufer，下面写out
            // smp_mb();
            _out.fetch_add(len, std::memory_order_acq_rel);
            //_out += len;

            return len;
        }
// TODO: 将boost::circular_buffer的部分接口移植过来
        inline size_t capacity() { return _capacity; }
        inline size_t length() { return (_in - _out); }
        inline bool empty() { return _in <= _out; }

    private:
        inline bool is_power_of_2(size_t num)
        {
            return (num != 0 && (num & (num - 1)) == 0);
        }
        inline size_t hightest_one_bit(size_t num)
        {
            //使用模板元编程方式，使得运算次数能够根据计算机位宽变化
            //同时，常量控制的循环在编译期展开，提高速度
            for (size_t i = 1; i < sizeof(size_t) * 8; i = i * 2)
            {
                num |= (num >> i);
            }
#if 0
            num |= (num >> 1);
            num |= (num >> 2);
            num |= (num >> 4);
            num |= (num >> 8);
            num |= (num >> 16);
            num |= (num >> 32);
#endif
            return num - (num >> 1);
        }
        inline size_t roundup_pow_of_two(size_t num)
        {
            return num > 1 ? hightest_one_bit((num - 1) << 1) : 1;
        }
        uint8_t *_buffer;
        size_t _capacity;
        std::atomic<size_t> _in;
        std::atomic<size_t> _out;
    };

}