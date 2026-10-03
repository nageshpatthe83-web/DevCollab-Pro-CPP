#include "../include/ScoringEngine.h"
#include <algorithm>
#include <cmath>

ScoringEngine::ScoringEngine() {}

ScoreBreakdown ScoringEngine::computeScore(
    const QualityMetrics& quality,
    const AlgorithmProfile& profile,
    int testsPassed, int totalTests,
    bool hasDocumentation,
    double judgeInnovation,
    double judgeUI,
    double judgePresentation
) {
    ScoreBreakdown sb;

    // 1. Functionality Score (30% weight, max 30.0)
    double testRatio = (totalTests > 0) ? (static_cast<double>(testsPassed) / totalTests) : 1.0;
    sb.functionalityScore = std::round(testRatio * 30.0 * 10.0) / 10.0;

    // 2. Code Quality Score (15% weight, max 15.0)
    // Scaled by Maintainability Index (0-100) minus deductions for complexity/smells
    double qualityRatio = quality.maintainabilityIndex / 100.0;
    if (quality.cyclomaticComplexity > 12) qualityRatio *= 0.85;
    sb.codeQualityScore = std::round(std::max(0.0, std::min(15.0, qualityRatio * 15.0)) * 10.0) / 10.0;

    // 3. Performance Score (10% weight, max 10.0)
    // Rewards efficient Big-O complexity
    double perfPoints = 8.0;
    if (profile.empiricalComplexity == "O(1)" || profile.empiricalComplexity == "O(log N)") {
        perfPoints = 10.0;
    } else if (profile.empiricalComplexity == "O(N)" || profile.empiricalComplexity == "O(N log N)") {
        perfPoints = 9.2;
    } else if (profile.empiricalComplexity == "O(N^2)") {
        perfPoints = 6.8;
    }
    sb.performanceScore = perfPoints;

    // 4. Database & Delta Storage Score (10% weight, max 10.0)
    sb.databaseScore = 9.5;

    // 5. Documentation Score (10% weight, max 10.0)
    double docRatio = (quality.totalLines > 0) ? (static_cast<double>(quality.commentLines) / quality.totalLines) : 0.0;
    sb.documentationScore = hasDocumentation ? (std::min(10.0, 7.0 + docRatio * 15.0)) : 4.0;
    sb.documentationScore = std::round(sb.documentationScore * 10.0) / 10.0;

    // 6. Security Score (5% weight, max 5.0)
    double secPoints = 5.0 - (quality.securityFlawsCount * 2.0);
    sb.securityScore = std::max(1.0, secPoints);

    // 7. Judge Rubrics: Innovation (10%), UI/UX (5%), Presentation (5%)
    sb.innovationScore = std::min(10.0, std::max(0.0, judgeInnovation));
    sb.uiUxScore = std::min(5.0, std::max(0.0, judgeUI * 0.5));
    sb.presentationScore = std::min(5.0, std::max(0.0, judgePresentation * 0.5));

    // Composite Weighted Total (0-100)
    sb.compositeTotal = sb.functionalityScore + sb.codeQualityScore + sb.performanceScore + 
                        sb.databaseScore + sb.documentationScore + sb.securityScore + 
                        sb.innovationScore + sb.uiUxScore + sb.presentationScore;

    sb.compositeTotal = std::round(sb.compositeTotal * 10.0) / 10.0;
    return sb;
}

void ScoringEngine::registerObserver(std::shared_ptr<IEvaluationObserver> observer) {
    observers.push_back(observer);
}

void ScoringEngine::removeObserver(std::shared_ptr<IEvaluationObserver> observer) {
    observers.erase(std::remove(observers.begin(), observers.end(), observer), observers.end());
}

void ScoringEngine::notifyEvaluationCompleted(const LeaderboardEntry& entry) {
    for (auto& obs : observers) {
        if (obs) obs->onEvaluationCompleted(entry);
    }
}

void ScoringEngine::notifyLeaderboardUpdated(const std::vector<LeaderboardEntry>& topEntries) {
    for (auto& obs : observers) {
        if (obs) obs->onLeaderboardUpdated(topEntries);
    }
}
