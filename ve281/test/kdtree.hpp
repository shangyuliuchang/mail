#include <tuple>
#include <vector>
#include <algorithm>
#include <cassert>
#include <stdexcept>
// #include <iostream>

/**
 * An abstract template base of the KDTree class
 */
template <typename...>
class KDTree;

/**
 * A partial template specialization of the KDTree class
 v* The time complexity of functions are based on n and k
 * n is the size of the KDTree
 * k is the number of dimensions
 * @typedef Key         key type
 * @typedef Value       value type
 * @typedef Data        key-value pair
 * @static  KeySize     k (number of dimensions)
 */
template <typename ValueType, typename... KeyTypes>
class KDTree<std::tuple<KeyTypes...>, ValueType>
{
public:
    typedef std::tuple<KeyTypes...> Key;
    typedef ValueType Value;
    typedef std::pair<const Key, Value> Data;
    static inline constexpr size_t KeySize = std::tuple_size<Key>::value;
    static_assert(KeySize > 0, "Can not construct KDTree with zero dimension");

protected:
    struct Node
    {
        Data data;
        Node *parent;
        Node *left = nullptr;
        Node *right = nullptr;

        Node(const Key &key, const Value &value, Node *parent) : data(key, value), parent(parent) {}

        const Key &key() { return data.first; }

        Value &value() { return data.second; }
    };

public:
    /**
     * A bi-directional iterator for the KDTree
     * ! ONLY NEED TO MODIFY increment and decrement !
     */
    class Iterator
    {
    private:
        KDTree *tree;
        Node *node;

        Iterator(KDTree *tree, Node *node) : tree(tree), node(node) {}

        /**
         * Increment the iterator
         * Time complexity: O(log n)
         */
        void increment()
        {
            // TODO: implement this function
            if (node == nullptr)
                return;
            if (node->right == nullptr)
            {
                Node *search = node;
                while (search->parent)
                {
                    if (search->parent->left == search)
                    {
                        node = search->parent;
                        return;
                    }
                    search = search->parent;
                }
                node = nullptr;
                return;
            }
            else
            {
                Node *search = node->right;
                while (search->left)
                    search = search->left;
                node = search;
                return;
            }
        }

        /**
         * Decrement the iterator
         * Time complexity: O(log n)
         */
        void decrement()
        {
            // TODO: implement this function
            if (node == tree->begin())
            {
                return;
            }
            else if (node == nullptr)
            {
                Node *search = tree->root;
                while (search->right)
                    search = search->right;
                node = search;
            }
            else if (node->left)
            {
                Node *search = node->left;
                while (search->right)
                    search = search->right;
                node = search;
                return;
            }
            else
            {
                Node *search = node;
                while (search->parent)
                {
                    if (search->parent->right == search)
                    {
                        node = search->parent;
                        return;
                    }
                    search = search->parent;
                }
                node = tree->begin();
                return;
            }
        }

    public:
        friend class KDTree;

        Iterator() = delete;

        Iterator(const Iterator &) = default;

        Iterator &operator=(const Iterator &) = default;

        Iterator &operator++()
        {
            increment();
            return *this;
        }

        Iterator operator++(int)
        {
            Iterator temp = *this;
            increment();
            return temp;
        }

        Iterator &operator--()
        {
            decrement();
            return *this;
        }

        Iterator operator--(int)
        {
            Iterator temp = *this;
            decrement();
            return temp;
        }

        bool operator==(const Iterator &that) const
        {
            return node == that.node;
        }

        bool operator!=(const Iterator &that) const
        {
            return node != that.node;
        }

        Data *operator->()
        {
            return &(node->data);
        }

        Data &operator*()
        {
            return node->data;
        }
    };

protected:                // DO NOT USE private HERE!
    Node *root = nullptr; // root of the tree
    size_t treeSize = 0;  // size of the tree

    /**
     * Find the node with key
     * Time Complexity: O(k log n)
     * @tparam DIM current dimension of node
     * @param key
     * @param node
     * @return the node with key, or nullptr if not found
     */
    template <size_t DIM>
    Node *find(const Key &key, Node *node)
    {
        constexpr size_t DIM_NEXT = (DIM + 1) % KeySize;
        // TODO: implement this function
        if (node == nullptr)
            return nullptr;
        if (node->data.first == key)
            return node;
        // if (compareKey<DIM, std::less<>>(key, node->data.first))
        if (std::get<DIM>(key) < std::get<DIM>(node->data.first))
        {
            return find<DIM_NEXT>(key, node->left);
        }
        else
        {
            return find<DIM_NEXT>(key, node->right);
        }
    }

    // void show(const Node *node, int depth)
    // {
    //     for (int i = 0; i < depth; i++)
    //         std::cout << '_';
    //     if (!node)
    //     {
    //         std::cout << std::endl;
    //         return;
    //     }
    //     std::cout << std::get<0>(node->data.first) << "," << std::get<1>(node->data.first) << "," << std::get<2>(node->data.first) << "," << node->data.second.data() << std::endl;
    //     show(node->left, depth + 1);
    //     show(node->right, depth + 1);
    // }
    /**
     * Insert the key-value pair, if the key already exists, replace the value only
     * Time Complexity: O(k log n)
     * @tparam DIM current dimension of node
     * @param key
     * @param value
     * @param node
     * @param parent
     * @return whether insertion took place (return false if the key already exists)
     */
    template <size_t DIM>
    bool insert(const Key &key, const Value &value, Node *&node, Node *parent)
    {
        constexpr size_t DIM_NEXT = (DIM + 1) % KeySize;
        // TODO: implement this function
        if (node == nullptr)
        {
            node = new Node(key, value, parent);
            treeSize++;
            // show(root, 0);
            return true;
        }
        if (node->data.first == key)
        {
            node->data.second = value;
            return false;
        }
        // if (compareKey<DIM, std::less<>>(key, node->data.first))
        if (std::get<DIM>(key) < std::get<DIM>(node->data.first))
        {
            return insert<DIM_NEXT>(key, value, node->left, node);
        }
        else
        {
            return insert<DIM_NEXT>(key, value, node->right, node);
        }
    }

    /**
     * Compare two keys on a dimension
     * Time Complexity: O(1)
     * @tparam DIM comparison dimension
     * @tparam Compare
     * @param a
     * @param b
     * @param compare
     * @return relationship of two keys on a dimension with the compare function
     */
    template <size_t DIM, typename Compare>
    static bool compareKey(const Key &a, const Key &b, Compare compare = Compare())
    {
        if (std::get<DIM>(a) != std::get<DIM>(b))
        {
            return compare(std::get<DIM>(a), std::get<DIM>(b));
        }
        return compare(a, b);
    }

    /**
     * Compare two nodes on a dimension
     * Time Complexity: O(1)
     * @tparam DIM comparison dimension
     * @tparam Compare
     * @param a
     * @param b
     * @param compare
     * @return the minimum / maximum of two nodes
     */
    template <size_t DIM, typename Compare>
    static Node *compareNode(Node *a, Node *b, Compare compare = Compare())
    {
        if (!a)
            return b;
        if (!b)
            return a;
        return compareKey<DIM, Compare>(a->key(), b->key(), compare) ? a : b;
    }

    /**
     * Find the minimum node on a dimension
     * Time Complexity: ?
     * @tparam DIM_CMP comparison dimension
     * @tparam DIM current dimension of node
     * @param node
     * @return the minimum node on a dimension
     */
    template <size_t DIM_CMP, size_t DIM>
    Node *findMin(Node *node)
    {
        constexpr size_t DIM_NEXT = (DIM + 1) % KeySize;
        // TODO: implement this function
        if (node == nullptr)
            return nullptr;
        if (node->left == nullptr && node->right == nullptr)
            return node;
        // if (DIM_CMP == DIM)
        // {
        //     if (node->left == nullptr)
        //         return node;
        //     return findMin<DIM_CMP, DIM_NEXT>(node->left);
        // }
        Node *right, *min;
        min = findMin<DIM_CMP, DIM_NEXT>(node->left);
        if (DIM_CMP != DIM)
        {
            right = findMin<DIM_CMP, DIM_NEXT>(node->right);
            min = compareNode<DIM_CMP, std::less<>>(min, right);
        }
        min = compareNode<DIM_CMP, std::less<>>(min, node);
        // if (left != nullptr && std::get<DIM_CMP>(left->data.first) < std::get<DIM_CMP>(min->data.first))
        //     min = left;
        // if (right != nullptr && std::get<DIM_CMP>(right->data.first) < std::get<DIM_CMP>(min->data.first))
        //     min = right;
        // if (left == nullptr)
        //     min = right;
        // else if (right == nullptr)
        //     min = left;
        // else if (std::get<DIM_CMP>(left->data.first) < std::get<DIM_CMP>(right->data.first))
        //     min = left;
        // else
        //     min = right;
        // if (min == nullptr)
        //     min = node;
        // else if (std::get<DIM_CMP>(min->data.first) >= std::get<DIM_CMP>(node->data.first))
        //     min = node;
        return min;
    }

    /**
     * Find the maximum node on a dimension
     * Time Complexity: ?
     * @tparam DIM_CMP comparison dimension
     * @tparam DIM current dimension of node
     * @param node
     * @return the maximum node on a dimension
     */
    template <size_t DIM_CMP, size_t DIM>
    Node *findMax(Node *node)
    {
        constexpr size_t DIM_NEXT = (DIM + 1) % KeySize;
        // TODO: implement this function
        if (node == nullptr)
            return nullptr;
        if (node->left == nullptr && node->right == nullptr)
            return node;
        // if (DIM_CMP == DIM)
        // {
        //     if (node->right == nullptr)
        //         return node;
        //     return findMax<DIM_CMP, DIM_NEXT>(node->right);
        // }
        Node *left, *max;
        max = findMax<DIM_CMP, DIM_NEXT>(node->right);
        if (DIM_CMP != DIM)
        {
            left = findMax<DIM_CMP, DIM_NEXT>(node->left);
            max = compareNode<DIM_CMP, std::greater<>>(max, left);
        }
        max = compareNode<DIM_CMP, std::greater<>>(max, node);
        // if (left != nullptr && std::get<DIM_CMP>(left->data.first) > std::get<DIM_CMP>(max->data.first))
        //     max = left;
        // if (right != nullptr && std::get<DIM_CMP>(right->data.first) > std::get<DIM_CMP>(max->data.first))
        //     max = right;
        // if (left == nullptr)
        //     max = right;
        // else if (right == nullptr)
        //     max = left;
        // else if (std::get<DIM_CMP>(left->data.first) > std::get<DIM_CMP>(right->data.first))
        //     max = left;
        // else
        //     max = right;
        // if (max == nullptr)
        //     max = node;
        // else if (std::get<DIM_CMP>(max->data.first) <= std::get<DIM_CMP>(node->data.first))
        //     max = node;
        return max;
    }

    template <size_t DIM>
    Node *findMinDynamic(size_t dim)
    {
        constexpr size_t DIM_NEXT = (DIM + 1) % KeySize;
        if (dim >= KeySize)
        {
            dim %= KeySize;
        }
        if (dim == DIM)
            return findMin<DIM, 0>(root);
        return findMinDynamic<DIM_NEXT>(dim);
    }

    template <size_t DIM>
    Node *findMaxDynamic(size_t dim)
    {
        constexpr size_t DIM_NEXT = (DIM + 1) % KeySize;
        if (dim >= KeySize)
        {
            dim %= KeySize;
        }
        if (dim == DIM)
            return findMax<DIM, 0>(root);
        return findMaxDynamic<DIM_NEXT>(dim);
    }

    /**
     * Erase a node with key (check the pseudocode in project description)
     * Time Complexity: max{O(k log n), O(findMin)}
     * @tparam DIM current dimension of node
     * @param node
     * @param key
     * @return nullptr if node is erased, else the (probably) replaced node
     */
    template <size_t DIM>
    Node *erase(Node *node, const Key &key)
    {
        constexpr size_t DIM_NEXT = (DIM + 1) % KeySize;
        // TODO: implement this function
        if (node == nullptr)
            return nullptr;
        if (key == node->data.first)
        {
            if (node->left == nullptr && node->right == nullptr)
            {
                if (node->parent == nullptr)
                    root = nullptr;
                delete node;
                treeSize--;
                return nullptr;
                // if (node->parent == nullptr)
                // {
                //     root = nullptr;
                //     delete node;
                //     treeSize--;
                //     return nullptr;
                // }
                // if (node->parent->left == node)
                // {
                //     node->parent->left == nullptr;
                //     delete node;
                //     treeSize--;
                //     return nullptr;
                // }
                // if (node->parent->right == node)
                // {
                //     node->parent->right == nullptr;
                //     delete node;
                //     treeSize--;
                //     return nullptr;
                // }
            }
            else if (node->right)
            {
                Node *min;
                min = findMin<DIM, DIM_NEXT>(node->right);
                Node *parent = node->parent;
                Node *left = node->left;
                Node *right = node->right;
                delete node;
                node = new Node(min->data.first, min->data.second, parent);
                node->left = left;
                node->right = right;
                // node->data = min->data;
                // erase<DIM_NEXT>(node->right, min->data.first);
                node->right = erase<DIM_NEXT>(node->right, min->data.first);
                if (parent == nullptr)
                    root = node;
            }
            else if (node->left)
            {
                Node *max;
                max = findMax<DIM, DIM_NEXT>(node->left);
                Node *parent = node->parent;
                Node *left = node->left;
                Node *right = node->right;
                delete node;
                node = new Node(max->data.first, max->data.second, parent);
                node->right = right;
                node->left = left;
                // node->data = max->data;
                // erase<DIM_NEXT>(node->left, max->data.first);
                node->left = erase<DIM_NEXT>(node->left, max->data.first);
                if (parent == nullptr)
                    root = node;
            }
        }
        else
        {
            // if (compareKey<DIM, std::less<>>(key, node->data.first))
            if (std::get<DIM>(key) < std::get<DIM>(node->data.first))
            {
                // erase<DIM_NEXT>(node->left, key);
                node->left = erase<DIM_NEXT>(node->left, key);
            }
            else
            {
                // erase<DIM_NEXT>(node->right, key);
                node->right = erase<DIM_NEXT>(node->right, key);
            }
        }
        return node;
    }

    template <size_t DIM>
    Node *eraseDynamic(Node *node, size_t dim)
    {
        constexpr size_t DIM_NEXT = (DIM + 1) % KeySize;
        if (dim >= KeySize)
        {
            dim %= KeySize;
        }
        if (dim == DIM)
            return erase<DIM>(node, node->key());
        return eraseDynamic<DIM_NEXT>(node, dim);
    }

    // TODO: define your helper functions here if necessary
    void copyTree(Node *src, Node *&des, Node *parent)
    {
        if (src == nullptr)
        {
            des = nullptr;
            return;
        }
        des = new Node(src->data.first, src->data.second, parent);
        treeSize++;
        copyTree(src->left, des->left, des);
        copyTree(src->right, des->right, des);
    }
    void eraseTree(Node *node)
    {
        if (node == nullptr)
            return;
        eraseTree(node->left);
        eraseTree(node->right);
        delete node;
        treeSize--;
    }
    // template <size_t DIM>
    // std::pair<Key, Value> findNthIn5(std::vector<std::pair<Key, Value>> &v, size_t start, size_t nth)
    // {
    //     typename std::vector<std::pair<Key, Value>>::iterator it;
    //     it = v.begin() + start + 5;
    //     if (it > v.end())
    //         it = v.end();
    //     std::vector<std::pair<Key, Value>> seg(v.begin() + start, it);
    //     std::pair<Key, Value> swap;
    //     size_t size = seg.size();
    //     for (size_t i = 0; i < size - 1; i++)
    //     {
    //         for (size_t j = i + 1; j < size; j++)
    //         {
    //             if (std::get<DIM>(seg[i].first) > std::get<DIM>(seg[j].first))
    //             {
    //                 swap = seg[i];
    //                 seg[i] = seg[j];
    //                 seg[j] = swap;
    //             }
    //         }
    //     }
    //     if (nth >= seg.size())
    //         return seg[seg.size() - 1];
    //     else
    //         return seg[nth];
    // }
    // template <size_t DIM>
    // std::pair<Key, Value> findNth(std::vector<std::pair<Key, Value>> &v, size_t nth)
    // {
    //     // constexpr size_t DIM_NEXT = (DIM + 1) % KeySize;
    //     if (v.size() <= 5)
    //         return findNthIn5<DIM>(v, 0, nth);
    //     std::vector<std::pair<Key, Value>> medians;
    //     for (size_t start = 0; start < v.size(); start += 5)
    //     {
    //         medians.push_back(findNthIn5<DIM>(v, start, 2));
    //     }
    //     std::pair<Key, Value> pivot;
    //     pivot = findNth<DIM>(medians, medians.size() / 2);
    //     medians.clear();
    //     std::vector<std::pair<Key, Value>> left;
    //     std::vector<std::pair<Key, Value>> right;
    //     for (size_t i = 0; i < v.size(); i++)
    //     {
    //         if (std::get<DIM>(v[i].first) < std::get<DIM>(pivot.first))
    //             left.push_back(v[i]);
    //         else if (v[i].first != pivot.first)
    //             right.push_back(v[i]);
    //     }
    //     v.clear();
    //     size_t leftsize = left.size();
    //     if (left.size() == nth)
    //         return pivot;
    //     else if (left.size() > nth)
    //     {
    //         right.clear();
    //         return findNth<DIM>(left, nth);
    //     }
    //     else
    //     {
    //         left.clear();
    //         return findNth<DIM>(right, nth - leftsize - 1);
    //     }
    // }
    template <size_t DIM>
    static bool myComp(std::pair<Key, Value> a, std::pair<Key, Value> b)
    {
        return std::get<DIM>(a.first) < std::get<DIM>(b.first);
    }
    template <size_t DIM>
    void constructTree(Node *&node, Node *parent, std::vector<std::pair<Key, Value>> &v, size_t begin, size_t end)
    {
        constexpr size_t DIM_NEXT = (DIM + 1) % KeySize;
        if (v.empty() || begin >= end)
        {
            node = nullptr;
            return;
        }
        // std::vector<std::pair<Key, Value>> left, right, copy(v);
        // std::pair<Key, Value> median = findNth<DIM>(copy, copy.size() / 2);
        // std::nth_element(v.begin() + begin, v.begin() + (begin + end) / 2, v.begin() + end);
        std::nth_element(v.begin() + begin, v.begin() + (begin + end) / 2, v.begin() + end, KDTree::myComp<DIM>);
        // for (size_t i = 0; i < v.size(); i++)
        // {
        //     if (std::get<DIM>(v[i].first) < std::get<DIM>(median.first))
        //     {
        //         left.push_back(v[i]);
        //     }
        //     else if (v[i].first != median.first)
        //     {
        //         right.push_back(v[i]);
        //     }
        // }
        // v.clear();
        node = new Node(v[(begin + end) / 2].first, v[(begin + end) / 2].second, parent);
        treeSize++;
        constructTree<DIM_NEXT>(node->left, node, v, begin, (begin + end) / 2);
        constructTree<DIM_NEXT>(node->right, node, v, (begin + end) / 2 + 1, end);
    }
    static bool equalKey(std::pair<Key, Value> a, std::pair<Key, Value> b)
    {
        return a.first == b.first;
    }

public:
    KDTree() = default;

    /**
     * Time complexity: O(kn log n)
     * @param v we pass by value here because v need to be modified
     */
    explicit KDTree(std::vector<std::pair<Key, Value>> v)
    {
        // TODO: implement this function
        if (v.empty())
        {
            treeSize = 0;
            root = nullptr;
            return;
        }
        std::stable_sort(v.begin(), v.end());
        typename std::vector<std::pair<Key, Value>>::reverse_iterator it;
        it = std::unique(v.rbegin(), v.rend(), KDTree::equalKey);
        v = std::vector<std::pair<Key, Value>>(it.base(), v.end());
        // std::vector<std::pair<Key, Value>> uni;
        // uni.push_back(v[v.size() - 1]);
        // if (v.size() > 1)
        // {
        //     for (int i = (int)v.size() - 2; i >= 0; i--)
        //     {
        //         if (!equalKey(v[i], uni[uni.size() - 1]))
        //         {
        //             uni.push_back(v[i]);
        //         }
        //     }
        // }
        // v.clear();
        constructTree<0>(root, nullptr, v, 0, v.size());
    }

    /**
     * Time complexity: O(n)
     */
    KDTree(const KDTree &that)
    {
        // TODO: implement this function
        copyTree(that.root, root, nullptr);
        // KeySize = that.KeySize;
        treeSize = that.treeSize;
    }

    /**
     * Time complexity: O(n)
     */
    KDTree &operator=(const KDTree &that)
    {
        // TODO: implement this function
        if (this != &that)
        {
            copyTree(that.root, root, nullptr);
            // KeySize = that.KeySize;
            treeSize = that.treeSize;
        }
        return *this;
    }

    /**
     * Time complexity: O(n)
     */
    ~KDTree()
    {
        // TODO: implement this function
        eraseTree(root);
    }

    Iterator begin()
    {
        if (!root)
            return end();
        auto node = root;
        while (node->left)
            node = node->left;
        return Iterator(this, node);
    }

    Iterator end()
    {
        return Iterator(this, nullptr);
    }

    Iterator find(const Key &key)
    {
        return Iterator(this, find<0>(key, root));
    }

    void insert(const Key &key, const Value &value)
    {
        insert<0>(key, value, root, nullptr);
    }

    template <size_t DIM>
    Iterator findMin()
    {
        return Iterator(this, findMin<DIM, 0>(root));
    }

    Iterator findMin(size_t dim)
    {
        return Iterator(this, findMinDynamic<0>(dim));
    }

    template <size_t DIM>
    Iterator findMax()
    {
        return Iterator(this, findMax<DIM, 0>(root));
    }

    Iterator findMax(size_t dim)
    {
        return Iterator(this, findMaxDynamic<0>(dim));
    }

    bool erase(const Key &key)
    {
        auto prevSize = treeSize;
        erase<0>(root, key);
        return prevSize > treeSize;
    }

    Iterator erase(Iterator it)
    {
        if (it == end())
            return it;
        auto node = it.node;
        if (!it.node->left && !it.node->right)
        {
            it.node = it.node->parent;
        }
        size_t depth = 0;
        auto temp = node->parent;
        while (temp)
        {
            temp = temp->parent;
            ++depth;
        }
        eraseDynamic<0>(node, depth % KeySize);
        return it;
    }

    size_t size() const { return treeSize; }
};