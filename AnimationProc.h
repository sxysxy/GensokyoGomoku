/*
    AnimationProc.h
        ����һ��ʱ��ġ����������̵ĳ���
                ÿһ������������һ��Э��
                ��������������ײ��н���
                        by ʯ���� 2020.07.06
*/
#pragma once

#include "Coroutine.h"
#include <string>

class AnimationProc {
    double startStamp, timeStamp = 0;
    Coroutine* coroutine;
    std::function<void(AnimationProc*)> onAnimationProc;
    std::function<void(void)> postAnimationProc;
    int endFlag = 0;
    size_t frameCount = 0;
public:
    
    AnimationProc(const std::function<void(AnimationProc *ani)> &f,
                            const std::function<void(void)> &postAni = nullptr) {
        onAnimationProc = f;
        coroutine = Coroutines::exec([&]() {
            onAnimationProc(this);
        }, false);
        postAnimationProc = postAni;
    }

    double getDeltaTime() {
        if (timeStamp == 0)
            return 0;
        else {
            auto current = GetCurrentTimeStamp();
            auto ret = current - timeStamp;
            timeStamp = current;
            return ret;
        }
    }

    void setEnd() {
        endFlag = true;
    }

    size_t getFrameCount() const {
        return frameCount;
    }

    double getAnimatedTime() const {
        return GetCurrentTimeStamp() - startStamp;
    }

    void update() {
        if (endFlag)
            return;
        if (timeStamp == 0)
            startStamp = timeStamp = GetCurrentTimeStamp();
        frameCount++;
        Coroutines::resume(coroutine);
        if (coroutine->getStatus() == Coroutine::DEAD) {
            setEnd();
            delete coroutine;
            coroutine = nullptr;
        }
        if (isEnd())
            if(postAnimationProc)
                postAnimationProc();
    }

    bool isEnd() const {
        return endFlag;
    }

    ~AnimationProc() {
        delete coroutine;
        coroutine = nullptr;
    }
    
};