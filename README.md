# DevCollab Pro — Native C++ & OOP Edition
**Intelligent Multi-Dimensional Code Evaluation & Hackathon Assessment Platform**
*Department of Computer Engineering, Vishwakarma Institute of Technology (VIT), Pune*  
*Guide: Dr. Yogesh Sharma | Group: SY06*

---

## 🚀 100% C++ Native Architecture (Zero Python)

Every subsystem in **DevCollab Pro** is engineered in **pure modern C++ (C++14)** using classic **Object-Oriented Programming (OOP)** principles and robust **Data Structures & Algorithms (DSA)**:

```
                                  +---------------------------------------+
                                  |     DevCollab Pro Native C++ Server   |
                                  |     (Winsock2 HTTP / Port 8080)       |
                                  +---------------------------------------+
                                                      |
                  +-----------------------------------+-----------------------------------+
                  |                                   |                                   |
                  v                                   v                                   v
      +-----------------------+           +-----------------------+           +-----------------------+
      |   CodeVault (DBMS)    |           |   CodeSentry (OOP)    |           |     AlgoPro (DSA)     |
      | • Myers Delta Storage |           | • 5 Design Patterns   |           | • Empirical Big-O     |
      | • 3-Way Merge Conflict|           | • AST Visitor & Rules |           | • Max-Heap Ranking    |
      | • 60%+ Storage Saved  |           | • Maintainability Idx |           | • Performance Tracer  |
      +-----------------------+           +-----------------------+           +-----------------------+
                  |                                   |                                   |
                  +-----------------------------------+-----------------------------------+
                                                      |
                                                      v
                                  +---------------------------------------+
                                  |       Scoring Engine (Observer)       |
                                  |  Weighted Multi-Dimensional Composite  |
                                  +---------------------------------------+
                                                      |
                                                      v
                                  +---------------------------------------+
                                  |   Web UI (Glassmorphic / Dark Mode)   |
                                  |       http://localhost:8080           |
                                  +---------------------------------------+
```

---

## 🏛️ OOP Principles & 5 Design Patterns Implemented

### 1. Strategy Pattern (`include/CodeSentry.h`, `src/CodeSentry.cpp`)
* **Interface:** `IAnalysisRule` defining `virtual void evaluate(...) = 0;`
* **Concrete Strategies:**
  * `CyclomaticComplexityRule`: Evaluates control flow branching, nesting depth, and decision points.
  * `SecurityAuditRule`: Flags SQL injection patterns, unsafe C-string operations (`strcpy`, `gets`, `sprintf`), and raw pointer memory leak risks.
  * `CodeSmellRule`: Catches functions with $>4$ parameters, hardcoded magic numbers, and bloated methods.
  * `StyleNamingRule`: Validates file size, clean formatting, and modular decomposition.
* **Benefit:** New rules can be plugged in dynamically at runtime without modifying existing analyzer code (Open/Closed Principle).

### 2. Visitor Pattern (`include/CodeSentry.h`, `src/CodeSentry.cpp`)
* **AST Node Hierarchy:** `ASTNode` $\rightarrow$ `FunctionNode`, `LoopNode`, `BranchNode`, `StatementNode`.
* **Visitor Interface:** `IASTVisitor` with overloaded `visit()` methods.
* **Concrete Visitor:** `MetricsCollectorVisitor` traverses AST nodes to calculate statements, decision points, nesting levels, and function metrics without polluting the AST data structures.

### 3. Factory Pattern (`include/CodeSentry.h`, `src/CodeSentry.cpp`)
* **Factory Class:** `CodeAnalyzerFactory::createAnalyzer(language)`
* **Concrete Products:** `CppCodeAnalyzer`, `JavaCodeAnalyzer`, `CCodeAnalyzer` implementing `ICodeAnalyzer`.
* **Benefit:** Decouples client code from concrete language analyzer instantiations.

### 4. Observer Pattern (`include/IObserver.h`, `src/ScoringEngine.cpp`, `src/Database.cpp`)
* **Subject Interface:** `IEvaluationSubject` implemented by `ScoringEngine`.
* **Observer Interface:** `IEvaluationObserver` implemented by `Database`.
* **Benefit:** When an evaluation completes or judge scores are submitted, all registered observers (Database, Audit logs, Leaderboard) receive real-time notifications automatically.

### 5. Singleton Pattern (`include/ConfigManager.h`, `src/ConfigManager.cpp`)
* **Class:** `ConfigManager`
* **Implementation:** Private constructor, deleted copy/move constructors, thread-safe static accessor `getInstance()`.
* **Benefit:** Centralized, consistent configuration for scoring rubric weights, ports, and metadata.

---

## ⚡ Data Structures & Algorithms (DSA)

1. **Max-Heap Leaderboard (`include/AlgoPro.h`, `src/AlgoPro.cpp`):**
   * Implemented custom Max-Heap with `siftUp`, `siftDown`, `insert` in $O(\log n)$, and `extractMax` in $O(\log n)$ to manage live hackathon tournament standings.
2. **Empirical Big-O Curve Fitting (`AlgoPro::classifyComplexity`):**
   * Times execution across scaled inputs ($N = 100 \dots 10,000$).
   * Fits measurements using regression analysis to classify algorithm complexity into $O(1)$, $O(\log N)$, $O(N)$, $O(N \log N)$, or $O(N^2)$.
3. **Myers Line-by-Line Delta Storage (`src/CodeVault.cpp`):**
   * Computes additions (`+`), deletions (`-`), and unchanged lines (` `), saving **60%+ storage space** compared to redundant full-copy commits.
4. **Three-Way Merge Conflict Detection (`CodeVault::detectThreeWayMergeConflict`):**
   * Evaluates a common base ancestor against local and remote branches to detect and isolate merge conflicts.

---

## 📊 Multi-Dimensional Scoring Formula (100 Points)

$$\text{Composite Score} = \text{Func}(30\%) + \text{Quality}(15\%) + \text{Perf}(10\%) + \text{DB}(10\%) + \text{Docs}(10\%) + \text{Sec}(5\%) + \text{Inno}(10\%) + \text{UI}(5\%) + \text{Pres}(5\%)$$

---

## 🛠️ How to Build & Run

### Method 1: Using the 1-Click Build Script
1. Open PowerShell or Command Prompt in this folder.
2. Run:
   ```cmd
   build.bat
   ```
3. Start the server:
   ```cmd
   devcollab_server.exe 8080
   ```
4. Open your browser and visit:  
   👉 **`http://localhost:8080`**

### Method 2: Manual G++ Compilation
```cmd
g++ -std=c++14 -O2 -Iinclude src/*.cpp -o devcollab_server.exe -lws2_32
.\devcollab_server.exe 8080
```

---

## 📁 File Structure

```
DevCollab_CPP/
├── build.bat                   <- 1-Click compilation script
├── devcollab_server.exe        <- Compiled native C++ binary
├── README.md                   <- Complete documentation & guide
├── include/                    <- C++ Header files & OOP Interfaces
│   ├── AlgoPro.h               <- DSA Empirical Profiler & Max-Heap
│   ├── CodeSentry.h            <- OOP AST Parser & 5 Design Patterns
│   ├── CodeVault.h             <- DBMS Version Control & Delta Storage
│   ├── ConfigManager.h         <- Singleton Configuration Manager
│   ├── Database.h              <- 3NF Relational Store & Observer
│   ├── HttpServer.h            <- Native Winsock2 HTTP Web Server
│   ├── IObserver.h             <- Observer Pattern Interfaces
│   ├── Models.h                <- Core Data Models & Structs
│   └── ScoringEngine.h         <- Multi-Dimensional Composite Formula
├── src/                        <- C++ Implementation files
│   ├── AlgoPro.cpp             <- Benchmarking, Regression & Max-Heap
│   ├── CodeSentry.cpp          <- AST Traversal & Strategy Rules
│   ├── CodeVault.cpp           <- Myers Diff & 3-Way Merge
│   ├── ConfigManager.cpp       <- Singleton Implementation
│   ├── Database.cpp            <- Storage & Real-time Ranking
│   ├── HttpServer.cpp          <- HTTP Request Router & Static Server
│   ├── main.cpp                <- Server Entrypoint
│   └── ScoringEngine.cpp       <- Scoring Calculation & Notification
└── public/                     <- Modern Glassmorphic Web UI
    ├── index.html              <- Semantic HTML5 SPA
    ├── style.css               <- Obsidian Dark Mode & Animations
    └── app.js                  <- Interactive REST API client & Charts
```
