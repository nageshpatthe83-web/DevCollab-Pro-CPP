#ifndef ALGO_PRO_H
#define ALGO_PRO_H

#include <string>
#include <vector>
#include <memory>
#include "Models.h"

// Custom Max-Heap for real-time Leaderboard Ranking (DSA)
class LeaderboardMaxHeap {
private:
    std::vector<LeaderboardEntry> heap;

    int parent(int i) { return (i - 1) / 2; }
    int leftChild(int i) { return (2 * i) + 1; }
    int rightChild(int i) { return (2 * i) + 2; }

    void siftUp(int i);
    void siftDown(int i);

public:
    LeaderboardMaxHeap();

    void insert(const LeaderboardEntry& entry);
    bool extractMax(LeaderboardEntry& outEntry);
    std::vector<LeaderboardEntry> getSortedRankings();
    int size() const { return static_cast<int>(heap.size()); }
    void clear() { heap.clear(); }
};

// AlgoPro: Algorithm Empirical Profiler & Big-O Classifier
class AlgoPro {
public:
    AlgoPro();

    // Profile a selected algorithm with input scaling
    AlgorithmProfile profileAlgorithm(const std::string& algoName, int maxInputSize = 5000);

    // Profile custom submitted C++ logic metrics
    AlgorithmProfile profileCustomCode(const std::string& code);

private:
    // Benchmark implementations
    void benchmarkQuickSort(int n, long long& comps, long long& swaps, double& timeUs);
    void benchmarkMergeSort(int n, long long& comps, long long& swaps, double& timeUs);
    void benchmarkBubbleSort(int n, long long& comps, long long& swaps, double& timeUs);
    void benchmarkBinarySearch(int n, long long& comps, long long& swaps, double& timeUs);
    void benchmarkLinearSearch(int n, long long& comps, long long& swaps, double& timeUs);

    // Helpers
    void merge(std::vector<int>& arr, int l, int m, int r, long long& comps, long long& swaps);
    void mergeSortHelper(std::vector<int>& arr, int l, int r, long long& comps, long long& swaps);
    void quickSortHelper(std::vector<int>& arr, int low, int high, long long& comps, long long& swaps);

    // Regression curve fitting for empirical Big-O classification
    void classifyComplexity(const std::vector<ProfileDataPoint>& points, 
                            std::string& outComplexity, double& outRSquared);
};

#endif // ALGO_PRO_H
