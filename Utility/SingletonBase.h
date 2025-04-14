template <typename T>
class SingletonBase
{
public:
    static T& GetInstance()
    {
        static T instance;
        return instance;
    }
protected:
    SingletonBase() {}
    ~SingletonBase() {}
public:
    SingletonBase(SingletonBase const&) = delete;
    SingletonBase& operator=(SingletonBase const&) = delete;
};