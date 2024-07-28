#pragma once
#include<bits/stdc++.h>

class ArenaAllocator{
    private:
        size_t m_size;
        std::byte* m_buffer;
        std::byte* m_offset;

    public:
        inline ArenaAllocator(size_t bytes):m_size(bytes){
            m_buffer = (std::byte*)malloc(m_size);
            m_offset = m_buffer;
        }

        template<typename T>
        T* alloc(){
            void* offset = m_offset;
            m_offset+=sizeof(T);
            return (T*)offset;
        }

        inline ArenaAllocator(const ArenaAllocator &other) = delete;

        inline ArenaAllocator operator=(const ArenaAllocator &other) = delete;

        inline ~ArenaAllocator(){
            free(m_buffer);
        }
};