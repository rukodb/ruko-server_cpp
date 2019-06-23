#include "SaveScheduler.hpp"
#include "RukoServer.hpp"

void SaveScheduler::run() {
    while (true) {
        sleep(1000);
        auto deltaTime = getTime() - lastSave;
        if ((deltaTime >= maxDeltaTime && writesSinceSave > 0) || writesSinceSave > maxDeltaWrites) {
            writesSinceSave = 0;
            lastSave += deltaTime;
            server.save();
        }
    }
}
