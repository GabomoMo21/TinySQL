#include "query/BTreeIndex.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

namespace tinysql
{
    BTreeIndex::Node::Node(
        bool leaf
    )
        : leaf(leaf)
    {
    }

    bool BTreeIndex::insert(
        const IndexKey& key,
        std::uint64_t recordOffset
    )
    {
        if (root_ == nullptr)
        {
            root_ =
                std::make_unique<Node>(
                    true
                );

            root_->keys.push_back(
                key
            );

            root_->recordOffsets.push_back(
                recordOffset
            );

            ++size_;

            return true;
        }

        std::size_t existingKeyIndex = 0;

        if (
            findNode(
                root_.get(),
                key,
                existingKeyIndex
            ) != nullptr
            )
        {
            // Índices únicos: no se aceptan claves repetidas.
            return false;
        }

        if (isFull(*root_))
        {
            std::unique_ptr<Node> oldRoot =
                std::move(root_);

            root_ =
                std::make_unique<Node>(
                    false
                );

            root_->children.push_back(
                std::move(oldRoot)
            );

            splitChild(
                *root_,
                0
            );
        }

        const bool inserted =
            insertNonFull(
                *root_,
                key,
                recordOffset
            );

        if (inserted)
        {
            ++size_;
        }

        return inserted;
    }

    bool BTreeIndex::isFull(
        const Node& node
    ) const
    {
        return node.keys.size() == MaxKeys;
    }

    bool BTreeIndex::insertNonFull(
        Node& node,
        const IndexKey& key,
        std::uint64_t recordOffset
    )
    {
        std::size_t position =
            node.keys.size();

        if (node.leaf)
        {
            while (
                position > 0 &&
                key.compare(
                    node.keys[position - 1]
                ) < 0
                )
            {
                --position;
            }

            node.keys.insert(
                node.keys.begin() +
                static_cast<std::ptrdiff_t>(position),
                key
            );

            node.recordOffsets.insert(
                node.recordOffsets.begin() +
                static_cast<std::ptrdiff_t>(position),
                recordOffset
            );

            return true;
        }

        while (
            position > 0 &&
            key.compare(
                node.keys[position - 1]
            ) < 0
            )
        {
            --position;
        }

        if (isFull(*node.children[position]))
        {
            splitChild(
                node,
                position
            );

            const int comparison =
                key.compare(
                    node.keys[position]
                );

            if (comparison == 0)
            {
                return false;
            }

            if (comparison > 0)
            {
                ++position;
            }
        }

        return insertNonFull(
            *node.children[position],
            key,
            recordOffset
        );
    }

    void BTreeIndex::splitChild(
        Node& parent,
        std::size_t childIndex
    )
    {
        Node& child =
            *parent.children[childIndex];

        std::unique_ptr<Node> rightNode =
            std::make_unique<Node>(
                child.leaf
            );

        IndexKey medianKey =
            child.keys[MinimumDegree - 1];

        const std::uint64_t medianOffset =
            child.recordOffsets[MinimumDegree - 1];

        for (
            std::size_t index = MinimumDegree;
            index < child.keys.size();
            ++index
            )
        {
            rightNode->keys.push_back(
                child.keys[index]
            );

            rightNode->recordOffsets.push_back(
                child.recordOffsets[index]
            );
        }

        if (!child.leaf)
        {
            for (
                std::size_t index = MinimumDegree;
                index < child.children.size();
                ++index
                )
            {
                rightNode->children.push_back(
                    std::move(child.children[index])
                );
            }

            child.children.resize(
                MinimumDegree
            );
        }

        child.keys.resize(
            MinimumDegree - 1
        );

        child.recordOffsets.resize(
            MinimumDegree - 1
        );

        parent.keys.insert(
            parent.keys.begin() +
            static_cast<std::ptrdiff_t>(childIndex),
            medianKey
        );

        parent.recordOffsets.insert(
            parent.recordOffsets.begin() +
            static_cast<std::ptrdiff_t>(childIndex),
            medianOffset
        );

        parent.children.insert(
            parent.children.begin() +
            static_cast<std::ptrdiff_t>(childIndex + 1),
            std::move(rightNode)
        );
    }

    std::vector<std::uint64_t> BTreeIndex::findEqual(
        const IndexKey& key
    ) const
    {
        std::vector<std::uint64_t> offsets;

        std::size_t keyIndex = 0;

        const Node* node =
            findNode(
                root_.get(),
                key,
                keyIndex
            );

        if (node != nullptr)
        {
            offsets.push_back(
                node->recordOffsets[keyIndex]
            );
        }

        return offsets;
    }

    const BTreeIndex::Node* BTreeIndex::findNode(
        const Node* node,
        const IndexKey& key,
        std::size_t& keyIndex
    ) const
    {
        if (node == nullptr)
        {
            return nullptr;
        }

        std::size_t index = 0;

        while (
            index < node->keys.size() &&
            key.compare(
                node->keys[index]
            ) > 0
            )
        {
            ++index;
        }

        if (
            index < node->keys.size() &&
            key.compare(
                node->keys[index]
            ) == 0
            )
        {
            keyIndex = index;

            return node;
        }

        if (node->leaf)
        {
            return nullptr;
        }

        return findNode(
            node->children[index].get(),
            key,
            keyIndex
        );
    }

    std::vector<std::uint64_t> BTreeIndex::findGreaterThan(
        const IndexKey& key
    ) const
    {
        std::vector<std::uint64_t> offsets;

        collectGreaterThan(
            root_.get(),
            key,
            offsets
        );

        return offsets;
    }

    std::vector<std::uint64_t> BTreeIndex::findLessThan(
        const IndexKey& key
    ) const
    {
        std::vector<std::uint64_t> offsets;

        collectLessThan(
            root_.get(),
            key,
            offsets
        );

        return offsets;
    }

    void BTreeIndex::collectAll(
        const Node* node,
        std::vector<std::uint64_t>& offsets
    ) const
    {
        if (node == nullptr)
        {
            return;
        }

        for (
            std::size_t index = 0;
            index < node->keys.size();
            ++index
            )
        {
            if (!node->leaf)
            {
                collectAll(
                    node->children[index].get(),
                    offsets
                );
            }

            offsets.push_back(
                node->recordOffsets[index]
            );
        }

        if (!node->leaf)
        {
            collectAll(
                node->children[node->keys.size()].get(),
                offsets
            );
        }
    }

    void BTreeIndex::collectLessThan(
        const Node* node,
        const IndexKey& key,
        std::vector<std::uint64_t>& offsets
    ) const
    {
        if (node == nullptr)
        {
            return;
        }

        for (
            std::size_t index = 0;
            index < node->keys.size();
            ++index
            )
        {
            const int comparison =
                node->keys[index].compare(
                    key
                );

            if (!node->leaf)
            {
                if (comparison < 0)
                {
                    collectAll(
                        node->children[index].get(),
                        offsets
                    );
                }
                else
                {
                    collectLessThan(
                        node->children[index].get(),
                        key,
                        offsets
                    );

                    return;
                }
            }

            if (comparison < 0)
            {
                offsets.push_back(
                    node->recordOffsets[index]
                );
            }
            else
            {
                return;
            }
        }

        if (!node->leaf)
        {
            collectLessThan(
                node->children[node->keys.size()].get(),
                key,
                offsets
            );
        }
    }

    void BTreeIndex::collectGreaterThan(
        const Node* node,
        const IndexKey& key,
        std::vector<std::uint64_t>& offsets
    ) const
    {
        if (node == nullptr)
        {
            return;
        }

        std::size_t index =
            node->keys.size();

        while (index > 0)
        {
            --index;

            const int comparison =
                node->keys[index].compare(
                    key
                );

            if (!node->leaf)
            {
                if (comparison > 0)
                {
                    collectAll(
                        node->children[index + 1].get(),
                        offsets
                    );
                }
                else
                {
                    collectGreaterThan(
                        node->children[index + 1].get(),
                        key,
                        offsets
                    );

                    return;
                }
            }

            if (comparison > 0)
            {
                offsets.push_back(
                    node->recordOffsets[index]
                );
            }
            else
            {
                return;
            }
        }

        if (!node->leaf)
        {
            collectGreaterThan(
                node->children[0].get(),
                key,
                offsets
            );
        }
    }

    void BTreeIndex::clear()
    {
        root_.reset();
        size_ = 0;
    }

    bool BTreeIndex::empty() const
    {
        return root_ == nullptr;
    }

    std::size_t BTreeIndex::size() const
    {
        return size_;
    }
}