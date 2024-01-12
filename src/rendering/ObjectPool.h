#pragma once
#include<stack>
#include<functional>

template<typename T>
class ObjectPool
{
private:
    std::stack<T*> _freeObjs{};

    std::function<void(T*)> _getOp;
    std::function<void(T*)> _retOp;
    std::function<T*()> _newOp;
    std::function<void(T*)> _releaseOp;

    T* DefaultCreate() { return new T(); }
    void DefaultDelect(T* obj) { delete obj; } 
public:
    ObjectPool(std::function<void(T*)> getOp = nullptr,
               std::function<void(T*)> retOp = nullptr,
               std::function<T*()> newOp = nullptr,
               std::function<void(T*)> releaseOp = nullptr)
        : _getOp(getOp), _retOp(retOp), _newOp(newOp), _releaseOp(releaseOp)
    {
    }

    T* Get()
    {
        T* obj{nullptr};
        if (!_freeObjs.empty())
        {
            obj = _freeObjs.top();
            _freeObjs.pop();
        }
        else 
        {
            obj = _newOp ? _newOp() : DefaultCreate();
        }

        if (_getOp)
            _getOp(obj);
        
        return obj;
    }

    void Return(T* obj)
    {
        if (obj == nullptr)
            return;
        
        if (_retOp)
            _retOp(obj);
        
        _freeObjs.push(obj);
    }

    void Release(T* obj)
    {
        if (_releaseOp)
            _releaseOp(obj);
        else
            DefaultDelect(obj);
    }

    size_t FreeObjectCount() const { return _freeObjs.size(); }
};