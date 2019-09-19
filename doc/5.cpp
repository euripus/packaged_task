class A
{
    int * data;

public:
    A(int v)
    {
        data  = new int;
        *data = v;
    }
    ~A() { delete data; }

    // copy semantics
    A(A & a)
    {
        data  = new int;
        *data = a.val();
    }
    A & operator=(A & a)
    {
        *data = a.val();
        return *this;
    }

    // move semantics
    A(A && a) : data(a.data) { a.data = 0; }
    A & operator=(A && a)
    {
        swap(data, a.data);
        return *this;
    }

    int val() { return data == 0 ? 0 : *data; }
};

//=======================================================//

#include <any>
#include <cassert>
#include <string>
#include <vector>

int main()
{
    // (1)
    {
        std::any x = std::make_any<std::string>(3, 'z');
        assert(std::any_cast<std::string>(x) == "zzz");
    }

    // (2)
    {
        std::allocator<int> alloc;
        std::any            x = std::make_any<std::vector<int>>({3, 1, 4}, alloc);

        const auto & vec = std::any_cast<const std::vector<int> &>(x);
        assert(vec[0] == 3);
        assert(vec[1] == 1);
        assert(vec[2] == 4);
    }
}
