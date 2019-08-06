#include <stack>
#include "singleton.h"

namespace
{

std::stack<common::SingletonFinalizer::Callback> cbs;
std::once_flag once;

}

namespace common
{

void SingletonFinalizer::Register(const Callback& cb)
{
    std::call_once(once, []()
    {
        // 指定された関数は、プログラムの終了処理において、static変数の破棄が行われる前に呼び出される。
        std::atexit(SingletonFinalizer::Finalize);
    });
    cbs.push(cb);
}

void SingletonFinalizer::Finalize()
{
    while(!cbs.empty()){
        auto& cb = cbs.top();
        cb();
        cbs.pop();
    }
}

}   // namespace common