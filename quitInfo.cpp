#include <quitInfo.h>
#include <mutex>

void quitCode::setNoQuit(){
    m.lock();
    code = NOQUIT;
    m.unlock();
}

void quitCode::setError(){
    m.lock();
    code = ERROR;
    w.unlock();
    m.unlock();
}

void quitCode::setQuit(){
    m.lock();
    code = QUIT;
    w.unlock();
    m.unlock();
}

bool quitCode::isNoQuit(){
    bool isNoQ;
    m.lock();
    isNoQ = code == NOQUIT;
    m.unlock();
    return isNoQ;
}

bool quitCode::isQuit(){
    bool isQ;
    m.lock();
    isQ = code == QUIT;
    m.unlock();
    return isQ;
}

bool quitCode::isError(){
    bool isErr;
    m.lock();
    isErr = code == ERROR;
    m.unlock();
    return isErr;
}

void quitCode::wait(){
    w.lock();
}

quitCode::quitCode(){
    code = NOQUIT;
    w.lock();
}
