#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include "Models.h"
#include "IObserver.h"
#include "AlgoPro.h"

// In-Memory Relational 3NF Database & Leaderboard Observer
class Database : public IEvaluationObserver {
private:
    std::map<std::string, LeaderboardEntry> submissions;
    LeaderboardMaxHeap maxHeap;
    std::vector<std::string> contests;

public:
    Database();

    void seedDefaultData();

    // Store or update submission
    void recordSubmission(const LeaderboardEntry& entry);

    // Get live rankings
    std::vector<LeaderboardEntry> getRankings();

    // Get specific entry
    bool getEntry(const std::string& teamId, LeaderboardEntry& outEntry);

    // Update judge scores
    bool updateJudgeScores(const std::string& teamId, double innovation, double ui, double presentation);

    // Observer Pattern implementation
    void onEvaluationCompleted(const LeaderboardEntry& entry) override;
    void onLeaderboardUpdated(const std::vector<LeaderboardEntry>& topEntries) override;
};

#endif // DATABASE_H
