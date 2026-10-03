#include "../include/Database.h"
#include <iostream>

Database::Database() {
    seedDefaultData();
}

void Database::seedDefaultData() {
    // Seed Team SY06 (User's team from project docs)
    LeaderboardEntry teamSY06;
    teamSY06.teamId = "SY06";
    teamSY06.teamName = "DevCollab Innovators";
    teamSY06.projectName = "DevCollab Pro - Code Evaluation Engine";
    teamSY06.members = "Nagesh Patthe (Roll 11), Shantanu Patil (Roll 04), Jagdish Patole (Roll 10), Kunal Rahangdale (Roll 50)";
    teamSY06.submissionTime = "2026-10-04 00:15:00";
    teamSY06.latestCommitHash = "a4f8b91c2e3d4f50";

    teamSY06.scores.functionalityScore = 29.0;
    teamSY06.scores.codeQualityScore = 14.5;
    teamSY06.scores.performanceScore = 9.8;
    teamSY06.scores.databaseScore = 9.6;
    teamSY06.scores.documentationScore = 9.5;
    teamSY06.scores.securityScore = 4.9;
    teamSY06.scores.innovationScore = 9.5;
    teamSY06.scores.uiUxScore = 4.8;
    teamSY06.scores.presentationScore = 4.9;
    teamSY06.scores.compositeTotal = 96.5;

    recordSubmission(teamSY06);

    // Seed competitor team 1
    LeaderboardEntry teamSY02;
    teamSY02.teamId = "SY02";
    teamSY02.teamName = "AlgoKnights";
    teamSY02.projectName = "Distributed Graph Search Optimizer";
    teamSY02.members = "Aarav Sharma, Rohit Verma";
    teamSY02.submissionTime = "2026-10-03 22:40:00";
    teamSY02.latestCommitHash = "7bc3a19e88d2f11a";
    teamSY02.scores.functionalityScore = 27.5;
    teamSY02.scores.codeQualityScore = 13.0;
    teamSY02.scores.performanceScore = 9.0;
    teamSY02.scores.databaseScore = 8.5;
    teamSY02.scores.documentationScore = 8.0;
    teamSY02.scores.securityScore = 4.5;
    teamSY02.scores.innovationScore = 8.5;
    teamSY02.scores.uiUxScore = 4.2;
    teamSY02.scores.presentationScore = 4.5;
    teamSY02.scores.compositeTotal = 87.7;
    recordSubmission(teamSY02);

    // Seed competitor team 2
    LeaderboardEntry teamSY09;
    teamSY09.teamId = "SY09";
    teamSY09.teamName = "ByteForge";
    teamSY09.projectName = "Autonomous Query Plan Visualizer";
    teamSY09.members = "Priya Deshmukh, Sameer Joshi";
    teamSY09.submissionTime = "2026-10-03 23:10:00";
    teamSY09.latestCommitHash = "3e5a7b9c1d0f224b";
    teamSY09.scores.functionalityScore = 26.0;
    teamSY09.scores.codeQualityScore = 12.5;
    teamSY09.scores.performanceScore = 8.5;
    teamSY09.scores.databaseScore = 9.0;
    teamSY09.scores.documentationScore = 7.5;
    teamSY09.scores.securityScore = 4.0;
    teamSY09.scores.innovationScore = 8.0;
    teamSY09.scores.uiUxScore = 4.0;
    teamSY09.scores.presentationScore = 4.2;
    teamSY09.scores.compositeTotal = 83.7;
    recordSubmission(teamSY09);
}

void Database::recordSubmission(const LeaderboardEntry& entry) {
    submissions[entry.teamId] = entry;

    // Rebuild MaxHeap
    maxHeap.clear();
    for (const auto& pair : submissions) {
        maxHeap.insert(pair.second);
    }
}

std::vector<LeaderboardEntry> Database::getRankings() {
    return maxHeap.getSortedRankings();
}

bool Database::getEntry(const std::string& teamId, LeaderboardEntry& outEntry) {
    auto it = submissions.find(teamId);
    if (it != submissions.end()) {
        outEntry = it->second;
        return true;
    }
    return false;
}

bool Database::updateJudgeScores(const std::string& teamId, double innovation, double ui, double presentation) {
    auto it = submissions.find(teamId);
    if (it != submissions.end()) {
        it->second.scores.innovationScore = innovation;
        it->second.scores.uiUxScore = ui * 0.5;
        it->second.scores.presentationScore = presentation * 0.5;
        it->second.scores.compositeTotal = 
            it->second.scores.functionalityScore + it->second.scores.codeQualityScore + 
            it->second.scores.performanceScore + it->second.scores.databaseScore + 
            it->second.scores.documentationScore + it->second.scores.securityScore + 
            it->second.scores.innovationScore + it->second.scores.uiUxScore + 
            it->second.scores.presentationScore;

        recordSubmission(it->second);
        return true;
    }
    return false;
}

void Database::onEvaluationCompleted(const LeaderboardEntry& entry) {
    recordSubmission(entry);
}

void Database::onLeaderboardUpdated(const std::vector<LeaderboardEntry>& topEntries) {
    // Observer notification received
}
