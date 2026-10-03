#ifndef SCORING_ENGINE_H
#define SCORING_ENGINE_H

#include <vector>
#include <memory>
#include "Models.h"
#include "IObserver.h"

// ScoringEngine: Evaluates multi-dimensional criteria & implements Subject for Observer pattern
class ScoringEngine : public IEvaluationSubject {
private:
    std::vector<std::shared_ptr<IEvaluationObserver>> observers;

public:
    ScoringEngine();

    // Calculate score breakdown based on metrics, test results, and judge inputs
    ScoreBreakdown computeScore(
        const QualityMetrics& quality,
        const AlgorithmProfile& profile,
        int testsPassed, int totalTests,
        bool hasDocumentation,
        double judgeInnovation = 8.5,
        double judgeUI = 8.0,
        double judgePresentation = 9.0
    );

    // Subject implementation (Observer Pattern)
    void registerObserver(std::shared_ptr<IEvaluationObserver> observer) override;
    void removeObserver(std::shared_ptr<IEvaluationObserver> observer) override;
    void notifyEvaluationCompleted(const LeaderboardEntry& entry) override;
    void notifyLeaderboardUpdated(const std::vector<LeaderboardEntry>& topEntries) override;
};

#endif // SCORING_ENGINE_H
