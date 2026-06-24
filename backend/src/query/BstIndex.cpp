#include "query/BstIndex.hpp"

#include <cstdint>
#include <memory>
#include <vector>

namespace tinysql
{
    BstIndex::Node::Node(
        const IndexKey& key,
        std::uint64_t recordOffset
    )
        : key(key),
        recordOffset(recordOffset)
    {
    }

    bool BstIndex::insert(
        const IndexKey& key,
        std::uint64_t recordOffset
    )
    {
        return insertRecursive(
            root_,
            key,
            recordOffset
        );
    }

    bool BstIndex::insertRecursive(
        std::unique_ptr<Node>& node,
        const IndexKey& key,
        std::uint64_t recordOffset
    )
    {
        if (node == nullptr)
        {
            node =
                std::make_unique<Node>(
                    key,
                    recordOffset
                );

            ++size_;

            return true;
        }

        const int comparison =
            key.compare(
                node->key
            );

        if (comparison == 0)
        {
            // En esta etapa los índices se tratan como únicos.
            return false;
        }

        if (comparison < 0)
        {
            return insertRecursive(
                node->left,
                key,
                recordOffset
            );
        }

        return insertRecursive(
            node->right,
            key,
            recordOffset
        );
    }

    std::vector<std::uint64_t> BstIndex::findEqual(
        const IndexKey& key
    ) const
    {
        std::vector<std::uint64_t> offsets;

        const Node* foundNode =
            findNode(
                root_.get(),
                key
            );

        if (foundNode != nullptr)
        {
            offsets.push_back(
                foundNode->recordOffset
            );
        }

        return offsets;
    }

    const BstIndex::Node* BstIndex::findNode(
        const Node* node,
        const IndexKey& key
    ) const
    {
        if (node == nullptr)
        {
            return nullptr;
        }

        const int comparison =
            key.compare(
                node->key
            );

        if (comparison == 0)
        {
            return node;
        }

        if (comparison < 0)
        {
            return findNode(
                node->left.get(),
                key
            );
        }

        return findNode(
            node->right.get(),
            key
        );
    }

    std::vector<std::uint64_t> BstIndex::findGreaterThan(
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

    void BstIndex::collectGreaterThan(
        const Node* node,
        const IndexKey& key,
        std::vector<std::uint64_t>& offsets
    ) const
    {
        if (node == nullptr)
        {
            return;
        }

        if (node->key.compare(key) > 0)
        {
            collectGreaterThan(
                node->left.get(),
                key,
                offsets
            );

            offsets.push_back(
                node->recordOffset
            );

            collectGreaterThan(
                node->right.get(),
                key,
                offsets
            );
        }
        else
        {
            collectGreaterThan(
                node->right.get(),
                key,
                offsets
            );
        }
    }

    std::vector<std::uint64_t> BstIndex::findLessThan(
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

    void BstIndex::collectLessThan(
        const Node* node,
        const IndexKey& key,
        std::vector<std::uint64_t>& offsets
    ) const
    {
        if (node == nullptr)
        {
            return;
        }

        if (node->key.compare(key) < 0)
        {
            collectLessThan(
                node->left.get(),
                key,
                offsets
            );

            offsets.push_back(
                node->recordOffset
            );

            collectLessThan(
                node->right.get(),
                key,
                offsets
            );
        }
        else
        {
            collectLessThan(
                node->left.get(),
                key,
                offsets
            );
        }
    }

    void BstIndex::clear()
    {
        root_.reset();
        size_ = 0;
    }

    std::size_t BstIndex::size() const
    {
        return size_;
    }

    bool BstIndex::empty() const
    {
        return root_ == nullptr;
    }
}
