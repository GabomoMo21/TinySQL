#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include "query/IndexKey.hpp"

namespace tinysql
{
    // Índice en memoria basado en un árbol B.
    // Usa claves únicas igual que el BST actual.
    class BTreeIndex
    {
    public:
        BTreeIndex() = default;

        BTreeIndex(const BTreeIndex&) = delete;

        BTreeIndex& operator=(
            const BTreeIndex&
            ) = delete;

        BTreeIndex(BTreeIndex&&) noexcept = default;

        BTreeIndex& operator=(
            BTreeIndex&&
            ) noexcept = default;

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
        // Grado mínimo t = 3.
        // Cada nodo puede tener entre 2 y 5 claves, excepto la raíz.
        static constexpr std::size_t MinimumDegree = 3;
        static constexpr std::size_t MaxKeys =
            (2 * MinimumDegree) - 1;

        struct Node
        {
            explicit Node(
                bool leaf
            );

            bool leaf;

            std::vector<IndexKey> keys;
            std::vector<std::uint64_t> recordOffsets;

            std::vector<std::unique_ptr<Node>> children;
        };

        std::unique_ptr<Node> root_;

        std::size_t size_ =
            0;

        bool isFull(
            const Node& node
        ) const;

        bool insertNonFull(
            Node& node,
            const IndexKey& key,
            std::uint64_t recordOffset
        );

        void splitChild(
            Node& parent,
            std::size_t childIndex
        );

        const Node* findNode(
            const Node* node,
            const IndexKey& key,
            std::size_t& keyIndex
        ) const;

        void collectAll(
            const Node* node,
            std::vector<std::uint64_t>& offsets
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