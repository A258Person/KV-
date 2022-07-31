#include <iostream>
#include <unistd.h>
#include "myskiplist.hpp"
#define FILE_PATH "./dumpFile"

class MyLess
{
public:
    bool operator()(int a1, int a2)
    {
        return a1 > a2;
    }
};

class MyStrcmp
{
public:
    bool operator()(string a1, string a2)
    {
        return a1 > a2;
    }
};

void test()
{
    SkipList<string, string, MyStrcmp> skipList(6);
    skipList.insert_element("a", "你好");
    skipList.insert_element("c", "我是");
    skipList.insert_element("b", "小D");
    skipList.insert_element("45", "这是我的Github");
    skipList.insert_element("65", "很高兴认识你");
    skipList.insert_element("100", "YYDS");
    skipList.insert_element("@", "一起努力冲秋招！！！");

    cout << "skipList size:" << skipList.size() << endl;

    skipList.dump_file();

    skipList.load_file();

    skipList.search_element("c");
    skipList.search_element("100");

    skipList.display_list();

    skipList.delete_element("b");
    skipList.delete_element("@");

    cout << "skipList size:" << skipList.size() << endl;

    skipList.display_list();

    cout << "----------------------------------" << endl;
}

int main()
{

    // 键值中的key用int型，如果用其他类型，需要自定义比较函数
    // 而且如果修改key的类型，同时需要修改skipList.load_file函数
    SkipList<int, string, MyLess> skipList(6);
    skipList.insert_element(1, "你好");
    skipList.insert_element(3, "我是");
    skipList.insert_element(7, "小D");
    skipList.insert_element(8, "这是我的Github");
    skipList.insert_element(9, "很高兴认识你");
    skipList.insert_element(19, "YYDS");
    skipList.insert_element(19, "一起努力冲秋招！！！");

    cout << "skipList size:" << skipList.size() << endl;

    skipList.dump_file();

    skipList.load_file();

    skipList.search_element(9);
    skipList.search_element(18);

    skipList.display_list();

    skipList.delete_element(3);
    skipList.delete_element(7);

    cout << "skipList size:" << skipList.size() << endl;

    skipList.display_list();

    cout << "----------------------------------" << endl;

    test();
    return 0;
}
