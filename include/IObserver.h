#ifndef IOBSERVER_H
#define IOBSERVER_H

#include <string>
#include <vector>
#include <memory>
#include "Models.h"

// Observer Pattern: Observer Interface
class IEvaluationObserver {
public:
    virtual ~IEvaluationObserver() {}
    virtual void onEvaluationCompleted(const LeaderboardEntry& entry) = 0;
    virtual void onLeaderboardUpdated(const std::vector<LeaderboardEntry>& topEntries) = 0;
};

// Observer Pattern: Subject Interface
class IEvaluationSubject {
public:
    virtual ~IEvaluationSubject() {}
    virtual void registerObserver(std::shared_ptr<IEvaluationObserver> observer) = 0;
    virtual void removeObserver(std::shared_ptr<IEvaluationObserver> observer) = 0;
    virtual void notifyEvaluationCompleted(const LeaderboardEntry& entry) = 0;
    virtual void notifyLeaderboardUpdated(const std::vector<LeaderboardEntry>& topEntries) = 0;
};

#endif // IOBSERVER_H
