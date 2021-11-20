#include "kdtree.hpp"
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <string>
bool equalKey(std::pair<int, int> a, std::pair<int, int> b)
{
    return a.first == b.first;
}
int main(void)
{
    // KDTree<std::tuple<int, int>, int> tree;
    // tree.insert(std::tuple<int, int>(1, 2), 3);
    // // std::cout << (std::tuple<int, int>(3, 2) < std::tuple<int, int>(2, 3)) << std::endl;
    // std::vector<std::pair<int, int>> a;
    // a.push_back(std::pair<int, int>(1, 1));
    // a.push_back(std::pair<int, int>(2, 2));
    // a.push_back(std::pair<int, int>(8, 3));
    // a.push_back(std::pair<int, int>(6, 4));
    // a.push_back(std::pair<int, int>(4, 5));
    // a.push_back(std::pair<int, int>(3, 6));
    // a.push_back(std::pair<int, int>(8, 7));
    // a.push_back(std::pair<int, int>(4, 8));
    // a.push_back(std::pair<int, int>(1, 9));
    // a.push_back(std::pair<int, int>(3, 10));
    // a.push_back(std::pair<int, int>(2, 11));
    // std::stable_sort(a.begin(), a.end());
    // for (size_t i = 0; i < a.size(); i++)
    // {
    //     std::cout << a[i].first << "   " << a[i].second << std::endl;
    // }
    // typename std::vector<std::pair<int, int>>::reverse_iterator it;
    // it = std::unique(a.rbegin(), a.rend(), equalKey);
    // a = std::vector<std::pair<int, int>>(a.rbegin(), it);
    // for (size_t i = 0; i < a.size(); i++)
    // {
    //     // std::cout << a[i] << std::endl;
    //     std::cout << a[i].first << "   " << a[i].second << std::endl;
    // }
    std::vector<std::pair<std::tuple<int, int>, int>> v;
    srand((unsigned int)time(NULL));
    srand(666);
    for (int i = 0; i < 10; i++)
    {
        v.push_back(std::pair<std::tuple<int, int>, int>(std::tuple<int, int>(rand(), rand()), rand()));
    }
    // v.push_back(std::pair<std::tuple<int, int>, int>(std::tuple<int, int>(1, 2), 2));
    // v.push_back(std::pair<std::tuple<int, int>, int>(std::tuple<int, int>(2, 4), 3));
    // v.push_back(std::pair<std::tuple<int, int>, int>(std::tuple<int, int>(5, 3), 4));
    // v.push_back(std::pair<std::tuple<int, int>, int>(std::tuple<int, int>(3, 2), 5));
    // v.push_back(std::pair<std::tuple<int, int>, int>(std::tuple<int, int>(2, 4), 6));
    // v.push_back(std::pair<std::tuple<int, int>, int>(std::tuple<int, int>(1, 3), 7));
    // v.push_back(std::pair<std::tuple<int, int>, int>(std::tuple<int, int>(8, 1), 8));
    // v.push_back(std::pair<std::tuple<int, int>, int>(std::tuple<int, int>(5, 3), 9));
    // v.push_back(std::pair<std::tuple<int, int>, int>(std::tuple<int, int>(3, 2), 10));
    KDTree<std::tuple<int, int>, int> tree2(v);
    // KDTree<std::tuple<int, int>, int> tree3(tree2);
    // KDTree<std::tuple<int, int>, int> *tree = new KDTree<std::tuple<int, int>, int>(v);
    // KDTree<std::tuple<int, int>, int>::Iterator it = tree.begin();
    // while (it != tree.end())
    // {
    //     std::cout << std::get<0>(it->first) << std::endl;
    //     it++;
    // }
    // int a, b, c;
    // a = rand();
    // b = rand();
    // c = rand();
    // std::cout << a << "   " << b << "   " << c << std::endl;
    // tree->insert(std::tuple<int, int>(a, b), c);
    // tree->erase(std::tuple<int, int>(11804, 27290));
    // KDTree<std::tuple<int, int>, int> tree2(*tree);
    // delete tree;
    // KDTree<std::tuple<int, int, int>, std::string> tree;
    // tree.insert({0, 0, 0}, std::string("VE370"));
    // tree.insert({-1, 2, 0}, std::string("VE281"));
    // tree.insert({-8, -1, 0}, std::string("VE495"));
    // tree.insert({1, -1, 0}, std::string("VE482"));
    // tree.insert({3, 92, -3}, std::string("VE475"));
    // KDTree<std::tuple<int, int, int>, std::string>::Iterator it(tree.begin());
    // for (it = tree.begin(); it != tree.end(); it++)
    //     std::cout << it->second.data() << std::endl;
    // std::cout << tree.findMin(2)->second.data() << std::endl;
    return 0;
}