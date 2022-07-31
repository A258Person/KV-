#include <iostream>
#include <cstring>
#include <cstdlib>
#include <string>
#include <mutex>
#include <cmath>
#include <fstream>
#include <typeinfo>
#include <functional>

#define STORE_FILE "dumpFile"

using namespace std;

mutex mtx;              //定义互斥量
string delimiter = ":"; //定义分隔符

//跳表节点类
template <class K, class V>
class Node
{
public:
    Node();               //无参构造函数
    Node(K, V, int);      //构造函数
    ~Node();              //析构函数
    K get_key() const;    //返回key
    V get_value() const;  //返回value
    void set_value(V);    //设置value
    Node<K, V> **forward; //双层指针，一层指向其他层，一层指向当前层的后续元素
    int node_level;       //当前节点所属的层

private:
    K key;
    V value;
};

template <class K, class V>
Node<K, V>::Node(const K _key, const V _value, int _level)
{
    this->key = _key;
    this->value = _value;
    this->node_level = _level;

    //初始化forward第一层,level+1高的数组
    this->forward = new Node<K, V> *[_level + 1];

    //初始化数据
    memset(this->forward, 0, sizeof(Node<K, V> *) * (_level + 1));
}

template <class K, class V>
Node<K, V>::~Node()
{
    delete[] forward;
}

template <class K, class V>
K Node<K, V>::get_key() const
{
    return this->key;
}

template <class K, class V>
V Node<K, V>::get_value() const
{
    return this->value;
}

template <class K, class V>
void Node<K, V>::set_value(V _value)
{
    this->value = _value;
}

//跳表类
template <class K, class V, class mysort>
class SkipList
{
public:
    SkipList(int);                      //初始化时要制定最大层数
    ~SkipList();                        //析构
    int get_random_level();             //获得一个随机数
    Node<K, V> *create_node(K, V, int); //创建一个新的节点
    void display_list();                //打印展示跳表
    int insert_element(K, V);           //插入元素
    bool search_element(K);             //查找元素
    void delete_element(K);             //删除元素
    int size();                         //获得元素总个数
    void dump_file();                   //转储文件
    void load_file(){};                 //加载文件

    mysort sorts;

private:
    //从一个字符串中解析key value
    void get_key_value_from_string(const string &str, string *key, string *value);
    //判断是否解析成功,是否有效
    bool is_valid_string(const string &str);

    int _max_level;        //最大层
    int _skip_list_level;  //当前层
    Node<K, V> *_header;   //头节点
    ofstream _file_writer; //写
    ifstream _file_reader; //读
    int _element_count;    //元素总个数
};

//插入元素
template <class K, class V, class mysort>
int SkipList<K, V, mysort>::insert_element(K key, V value)
{
    mtx.lock();                          //上锁
    Node<K, V> *current = this->_header; //用cur保存头节点，控制移动

    //创建update数组并初始化
    // update是用来放置node的数组，在操作完node->forward[i]之后
    Node<K, V> *update[_max_level + 1];
    memset(update, 0, sizeof(Node<K, V> *) * (_max_level + 1));

    //从最高级别跳跃表开始找
    for (int i = _max_level; i >= 0; i--)
    {
        // while (current->forward[i] != nullptr && current->forward[i]->get_key() < key)
        while (current->forward[i] != nullptr && sorts(current->forward[i]->get_key(), key))
        {
            current = current->forward[i];
        }
        update[i] = current;
    }

    //一直找到0级，正向指针指向右节点，需要插入键
    current = current->forward[0];

    //如果当前跳表已经有这个key了 返回1
    if (current != NULL && current->get_key() == key)
    {
        cout << "key: " << key << ", exists" << endl;
        mtx.unlock();
        return 1;
    }

    //如果current是NULL，这意味着我们已经到达了0层的尽头
    //如果current的key不等于key，这意味着我们必须在update[0]和current节点之间插入节点
    if (current == NULL || current->get_key() != key)
    {
        // 为节点生成一个随机层
        int random_level = get_random_level();

        //如果随机的层数比当前层数高，那么更新当前层，用指向头的指针更新
        if (random_level > _skip_list_level)
        {
            for (int i = _skip_list_level + 1; i < random_level + 1; i++)
            {
                update[i] = _header;
            }
            _skip_list_level = random_level;
        }

        // 创建一个新的节点
        Node<K, V> *inserted_node = create_node(key, value, random_level);

        // 插入节点
        for (int i = 0; i <= random_level; i++)
        {
            inserted_node->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = inserted_node;
        }
        cout << "成功插入 key:" << key << ", value:" << value << endl;
        _element_count++;
    }
    mtx.unlock(); //解锁
    return 0;     //插入成功返回0
}

//展示跳表
template <typename K, typename V, typename mysort>
void SkipList<K, V, mysort>::display_list()
{

    cout << "\n*****Skip List*****\n";
    for (int i = 0; i <= _skip_list_level; i++)
    {
        Node<K, V> *node = this->_header->forward[i];
        cout << "Level " << i << ": ";
        while (node != NULL)
        {
            cout << node->get_key() << ":" << node->get_value() << ";";
            node = node->forward[i];
        }
        cout << endl;
    }
}

//删除元素
template <typename K, typename V, typename mysort>
void SkipList<K, V, mysort>::delete_element(K key)
{
    mtx.lock(); //上锁
    Node<K, V> *current = this->_header;
    Node<K, V> *update[_max_level + 1];
    memset(update, 0, sizeof(Node<K, V> *) * (_max_level + 1));

    // 从跳跃表的最高级别开始
    for (int i = _skip_list_level; i >= 0; i--)
    {
        // while (current->forward[i] != NULL && current->forward[i]->get_key() < key)
        while (current->forward[i] != NULL && sorts(current->forward[i]->get_key(), key))
        {
            current = current->forward[i];
        }
        update[i] = current;
    }

    current = current->forward[0];
    if (current != NULL && current->get_key() == key)
    {

        //从最底层开始，删除每一层的current节点
        for (int i = 0; i <= _skip_list_level; i++)
        {

            //如果在第i级，下一个节点不是目标节点，则终止循环。
            if (update[i]->forward[i] != current)
                break;

            update[i]->forward[i] = current->forward[i];
        }

        // 移除没有元素的层
        while (_skip_list_level > 0 && _header->forward[_skip_list_level] == 0)
        {
            _skip_list_level--;
        }

        cout << "成功删除 key " << key << endl;
        _element_count--;
    }
    mtx.unlock(); //解锁
    return;
}

//查找元素
template <typename K, typename V, typename mysort>
bool SkipList<K, V, mysort>::search_element(K key)
{

    cout << "查找元素中-----------------" << endl;
    Node<K, V> *current = _header;

    // 从跳跃表的最高级别开始
    for (int i = _skip_list_level; i >= 0; i--)
    {

        //  while (current->forward[i] && current->forward[i]->get_key() < key)
        while (current->forward[i] && sorts(current->forward[i]->get_key(), key))
        {
            current = current->forward[i];
        }
    }

    // 到达0级，指针指向我们搜索的右节点
    current = current->forward[0];

    // 如果找到了就返货true，否则false
    if (current and current->get_key() == key)
    {
        cout << "找到 key: " << key << ", value: " << current->get_value() << endl;
        return true;
    }

    cout << "没找到 Key:" << key << endl;
    return false;
}

// 将内存中的数据转储到文件
template <typename K, typename V, typename mysort>
void SkipList<K, V, mysort>::dump_file()
{

    cout << "转储文件中-----------------" << endl;
    _file_writer.open(STORE_FILE);
    Node<K, V> *node = this->_header->forward[0];

    while (node != NULL)
    {
        _file_writer << node->get_key() << ":" << node->get_value() << "\n";
        cout << node->get_key() << ":" << node->get_value() << ";\n";
        node = node->forward[0];
    }

    _file_writer.flush();
    _file_writer.close();
    return;
}

// 从磁盘加载数据
template <>
void SkipList<string, string, function<bool(string, string)>>::load_file()
{
    _file_reader.open(STORE_FILE);
    cout << "load_file-----------------" << endl;
    string line;
    string *key = new string();
    string *value = new string();
    while (getline(_file_reader, line))
    {
        get_key_value_from_string(line, key, value);
        if (key->empty() || value->empty())
        {
            continue;
        }
        insert_element((*key).c_str(), *value);
        cout << "key:" << *key << "value:" << *value << endl;
    }
    _file_reader.close();
}

template <>
void SkipList<int, string, function<bool(int, int)>>::load_file()
{
    _file_reader.open(STORE_FILE);
    cout << "load_file-----------------" << endl;
    string line;
    string *key = new string();
    string *value = new string();
    while (getline(_file_reader, line))
    {
        get_key_value_from_string(line, key, value);
        if (key->empty() || value->empty())
        {
            continue;
        }
        insert_element(atoi((*key).c_str()), *value);
        cout << "key:" << *key << "value:" << *value << endl;
    }
    _file_reader.close();
}

template <typename K, typename V, typename mysort>
void SkipList<K, V, mysort>::get_key_value_from_string(const string &str, string *key, string *value)
{

    if (!is_valid_string(str))
    {
        return;
    }
    *key = str.substr(0, str.find(delimiter));
    *value = str.substr(str.find(delimiter) + 1, str.length());
}

template <typename K, typename V, typename mysort>
bool SkipList<K, V, mysort>::is_valid_string(const string &str)
{

    if (str.empty())
    {
        return false;
    }
    if (str.find(delimiter) == std::string::npos)
    {
        return false;
    }
    return true;
}

//构造节点
template <class K, class V, class mysort>
Node<K, V> *SkipList<K, V, mysort>::create_node(K k, V v, int level)
{
    Node<K, V> *temp = new Node<K, V>(k, v, level);
    return temp;
}

//返回最大容量
template <class K, class V, class mysort>
int SkipList<K, V, mysort>::size()
{
    return _element_count;
}

//构造函数
template <class K, class V, class mysort>
SkipList<K, V, mysort>::SkipList(int maxlevel)
{
    this->_max_level = maxlevel;
    this->_skip_list_level = 0;
    this->_element_count = 0;

    K _key;
    V _value;
    this->_header = new Node<K, V>(_key, _value, maxlevel);
}

//析构函数
template <class K, class V, class mysort>
SkipList<K, V, mysort>::~SkipList()
{
    if (_file_writer.is_open())
    {
        _file_writer.close();
    }
    if (_file_reader.is_open())
    {
        _file_reader.close();
    }
    // delete _header;
    Node<K, V> *current = this->_header;

    // 从跳跃表的最高级别开始
    for (int i = _skip_list_level; i >= 0; i--)
    {
        while (current->forward[i] != NULL)
        {
            Node<K, V> *temp = current;
            current = current->forward[i];
            delete temp;
        }
    }
}

//获取随机层数
template <class K, class V, class mysort>
int SkipList<K, V, mysort>::get_random_level()
{
    int k = 1;
    while (rand() % 2)
    {
        k++;
    }
    k = (k < _max_level) ? k : _max_level;
    return k;
}