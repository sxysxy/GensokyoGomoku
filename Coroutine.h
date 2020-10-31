/*
    Coroutine.h
        用户态协程实现
            by 石响宇 2020.07.06
 */

#pragma once

#include <Windows.h>
#include <functional>
#include <vector>
#include <stdexcept>
#include <atomic>

static double GetCurrentTimeStamp() {  //返回一个以 秒 为单位的时间戳
    long long t;
    GetSystemTimeAsFileTime(reinterpret_cast<LPFILETIME>(&t));
    return t / 10000000.0;
}

static void FiberProc(LPVOID co);

class Coroutine {
public:
    enum {
        READY,
        RUNNING,
        HUNGUP,
        DEAD
    };
private:
    LPVOID nativeFiberHandle = nullptr;

    Coroutine(const std::function<void(void)>& f, bool startOnCreate = false) {
        proc = f;

        nativeFiberHandle = CreateFiberEx(0, 0, FIBER_FLAG_FLOAT_SWITCH, FiberProc, this);
        status = READY;

        if (startOnCreate) {
            resume();
        }
    }
    friend class Coroutines;
    friend void FiberProc(LPVOID);

    std::function<void(void)> proc;
    std::atomic<int> status;
    
    void resume() {
        if (status == DEAD) {
            throw std::runtime_error("Switch to a dead coroutine");
        }
        else
            SwitchToFiber(nativeFiberHandle);
    }
public:
    
    int getStatus() const {
        if (status == Coroutine::RUNNING)
            return GetCurrentFiber() == nativeFiberHandle ? Coroutine::RUNNING : Coroutine::HUNGUP;
        return status;
    }

    void detach() {
        if (nativeFiberHandle) {
            DeleteFiber(nativeFiberHandle);
            nativeFiberHandle = nullptr;
        }
    }

    ~Coroutine() {
        detach();
    }
};

class Coroutines {
    //std::vector<Coroutine*> coroutines;

    std::vector<LPVOID> nativeFibers;
    std::vector<LPVOID> nativeCallstack;
    LPVOID currentNativeFiber;
    bool initedFlag = false;
public:

    static thread_local Coroutines globalCoroutinesManager;

    Coroutines() {
        if (globalCoroutinesManager.initedFlag)
            throw std::runtime_error("Volation of singletion mode of Coruotines");
        nativeFibers.push_back(ConvertThreadToFiberEx(nullptr, FIBER_FLAG_FLOAT_SWITCH));
        currentNativeFiber = nativeFibers.back();
        nativeCallstack.push_back(currentNativeFiber);
        initedFlag = true;
    }

    Coroutine* exec_(const std::function<void(void)>& f, bool startOnCreate = false) {
        auto co = new Coroutine(f, startOnCreate);
        //coroutines.push_back(co);
        return co;
    }

    void resume_(Coroutine* co) {
        currentNativeFiber = co->nativeFiberHandle;
        nativeCallstack.push_back(currentNativeFiber);
        co->resume();
    }

    void yield_() {
        nativeCallstack.pop_back();
        currentNativeFiber = nativeCallstack.back();
        SwitchToFiber(currentNativeFiber);
    }

    
    static inline Coroutine* exec(const std::function<void(void)>& f, bool startOnCreate = false) {
        return globalCoroutinesManager.exec_(f, startOnCreate);
    }
    static inline void resume(Coroutine* co) {
        globalCoroutinesManager.resume_(co);
    }
    static inline void yield() {
        globalCoroutinesManager.yield_();
    }
    static inline void sleep(double timeInSecond) {
        double s = GetCurrentTimeStamp();
        while (true) {
            double c = GetCurrentTimeStamp();
            if (c - s < timeInSecond) yield();
            else break;
        }
    }
};

static void FiberProc(LPVOID co) {
    Coroutine* c = (Coroutine*)co;
    c->status = Coroutine::RUNNING;
    try {
        c->proc();
    }
    catch (...) {
        //...应该做点什么
    }
    c->status = Coroutine::DEAD;
    //c->detach();
    Coroutines::yield();
}
