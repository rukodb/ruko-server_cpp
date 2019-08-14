#include "SaveScheduler.hpp"
#include "RukoServer.hpp"

void SaveScheduler::run() {
    while (isAlive) {
        updateEvent.lock();
        auto deltaTime = getTime() - lastSave;
        if ((deltaTime >= maxDeltaTime && writesSinceSave > 0) || writesSinceSave > maxDeltaWrites) {
            writesSinceSave = 0;
            lastSave += deltaTime;
            server.save();
        }
    }
}

void SaveScheduler::registerWrite() {
    ++writesSinceSave;
    updateEvent.unlock();
}

void SaveScheduler::shutdown() {
    isAlive = false;
    updateEvent.unlock();
    thread.join();
}
