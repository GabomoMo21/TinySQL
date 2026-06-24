#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include "query/IndexKey.hpp"

namespace tinysql
{
    // Índice en memoria basado en un árbol binario de búsqueda.
    class BstIndex
    {
    public:
        BstIndex() = default;

        bool insert(
            const IndexKey& key,
            std::uint64_t recordOffset
        );

        std::vector<std::uint64_t> findEqual(
            const IndexKey& key
        ) const;

        std::vector<std::uint64_t> findGreaterThan(
            const IndexKey& key
        ) const;

        std::vector<std::uint64_t> findLessThan(
            const IndexKey& key
        ) const;

        void clear();

        bool empty() const;

        std::size_t size() const;

    private:
        struct Node
        {
            Node(
                const IndexKey& key,
                std::uint64_t recordOffset
            );

            IndexKey key;
            std::uint64_t recordOffset;

            std::unique_ptr<Node> left;
            std::unique_ptr<Node> right;
        };

        std::unique_ptr<Node> root_;

        std::size_t size_ =
            0;

        bool insertRecursive(
            std::unique_ptr<Node>& node,
            const IndexKey& key,
            std::uint64_t recordOffset
        );

        const Node* findNode(
            const Node* node,
            const IndexKey& key
        ) const;

        void collectGreaterThan(
            const Node* node,
            const IndexKey& key,
            std::vector<std::uint64_t>& offsets
        ) const;

        void collectLessThan(
            const Node* node,
            const IndexKey& key,
            std::vector<std::uint64_t>& offsets
        ) const;
    };
}
