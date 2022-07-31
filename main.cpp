#include <iostream>
#include <unistd.h>
#include "myskiplist.hpp"
#include <string>
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
        int res = strcmp(a1.c_str(), a2.c_str());
        return res < 0 ? true : false;
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

void test2()
{
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
}

int main()
{
    test();  // int string类型的操作
    test2(); // string string类型的操作
    return 0;
}
