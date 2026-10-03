// DevCollab Pro - Web Application Logic
// Communicates with Native C++ Backend on http://localhost:8080

const API_BASE = ''; // Same-origin relative URLs

// Clean OOP C++ Sample (Demonstrating Visitor & Strategy pattern)
const SAMPLE_CLEAN_CPP = `// DevCollab Pro: High-Performance Expression Evaluator
// Demonstrates OOP Principles & Design Patterns
#include <iostream>
#include <memory>
#include <vector>
#include <string>

// Abstract Strategy Interface
class IOperationStrategy {
public:
    virtual ~IOperationStrategy() = default;
    virtual double execute(double a, double b) const = 0;
};

// Concrete Strategy: Addition
class AddOperation : public IOperationStrategy {
public:
    double execute(double a, double b) const override {
        return a + b;
    }
};

// Concrete Strategy: Multiplication
class MultiplyOperation : public IOperationStrategy {
public:
    double execute(double a, double b) const override {
        return a * b;
    }
};

// Context Class
class Calculator {
private:
    std::unique_ptr<IOperationStrategy> strategy;
public:
    Calculator(std::unique_ptr<IOperationStrategy> s) : strategy(std::move(s)) {}
    
    void setStrategy(std::unique_ptr<IOperationStrategy> s) {
        strategy = std::move(s);
    }
    
    double calculate(double x, double y) const {
        return strategy ? strategy->execute(x, y) : 0.0;
    }
};

int main() {
    Calculator calc(std::make_unique<AddOperation>());
    std::cout << "Sum: " << calc.calculate(15.5, 24.5) << std::endl;
    return 0;
}
`;

// Complex / Smelly C++ Sample (Demonstrating detection of leaks & complexity)
const SAMPLE_SMELLY_CPP = `#include <iostream>
#include <cstring>
using namespace std;

// Warning: Raw pointers, high nesting, unsafe functions, SQL concat
void processCustomerData(int id, char* rawInput, int flagA, int flagB, int flagC, int flagD) {
    char buffer[64];
    strcpy(buffer, rawInput); // Vulnerability: Unsafe function

    int* dataArray = new int[500]; // Warning: Raw pointer without delete

    string query = "SELECT * FROM Users WHERE id = " + to_string(id); // Warning: SQL pattern

    // High cyclomatic complexity and deep nesting
    for (int i = 0; i < 100; i++) {
        if (i % 2 == 0) {
            for (int j = 0; j < 50; j++) {
                if (j % 3 == 0) {
                    for (int k = 0; k < 10; k++) {
                        if (k > 5) {
                            dataArray[i] = i * j * k;
                        }
                    }
                }
            }
        }
    }
}

int main() {
    cout << "Legacy Module" << endl;
    return 0;
}
`;

// State
let currentJudgeTeamId = '';

// DOM Initialization
document.addEventListener('DOMContentLoaded', () => {
  setupNavigation();
  setupEditor();
  setupAlgoPro();
  setupVCS();
  setupLeaderboard();
  setupJudgeModal();

  // Load initial backend status
  fetchBackendStatus();
  fetchLeaderboard();
  fetchVCSHistory();

  // Load clean sample by default
  const editor = document.getElementById('code-editor');
  if (editor) editor.value = SAMPLE_CLEAN_CPP;
});

// Navigation Handling
function setupNavigation() {
  const navButtons = document.querySelectorAll('.nav-btn');
  navButtons.forEach(btn => {
    btn.addEventListener('click', () => {
      const tabId = btn.getAttribute('data-tab');
      switchTab(tabId);
    });
  });
}

function switchTab(tabId) {
  document.querySelectorAll('.tab-pane').forEach(p => p.classList.remove('active'));
  document.querySelectorAll('.nav-btn').forEach(b => b.classList.remove('active'));

  const targetPane = document.getElementById(tabId);
  const targetBtn = document.querySelector(`[data-tab="${tabId}"]`);

  if (targetPane) targetPane.classList.add('active');
  if (targetBtn) targetBtn.classList.add('active');
}

// Backend Status Check
async function fetchBackendStatus() {
  try {
    const res = await fetch(`${API_BASE}/api/status`);
    if (!res.ok) throw new Error('Status fetch failed');
    const data = await res.json();

    document.getElementById('backend-status-text').textContent = 'C++ Backend Active';
    if (data.storageSavingsPercent !== undefined) {
      document.getElementById('metric-savings').textContent = `${data.storageSavingsPercent.toFixed(1)}%`;
    }
  } catch (err) {
    document.getElementById('backend-status-text').textContent = 'Backend Standby';
  }
}

// Editor & CodeSentry Setup
function setupEditor() {
  const editor = document.getElementById('code-editor');
  const btnClean = document.getElementById('btn-sample-clean');
  const btnSmelly = document.getElementById('btn-sample-smelly');
  const btnRun = document.getElementById('btn-run-evaluation');

  btnClean.addEventListener('click', () => {
    editor.value = SAMPLE_CLEAN_CPP;
    showToast('Loaded clean OOP C++ code template');
  });

  btnSmelly.addEventListener('click', () => {
    editor.value = SAMPLE_SMELLY_CPP;
    showToast('Loaded complex code with security & smell patterns');
  });

  btnRun.addEventListener('click', runEvaluation);
}

async function runEvaluation() {
  const code = document.getElementById('code-editor').value;
  const language = document.getElementById('select-language').value;
  const teamName = document.getElementById('input-team-name').value || 'Team SY06';
  const projectName = document.getElementById('input-project-name').value || 'DevCollab Service';
  const author = document.getElementById('input-author').value || 'Nagesh Patthe';

  const badge = document.getElementById('eval-status-badge');
  badge.textContent = 'Evaluating in C++...';
  badge.style.color = '#38bdf8';

  try {
    const res = await fetch(`${API_BASE}/api/evaluate`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ code, language, teamName, projectName, author })
    });

    if (!res.ok) throw new Error('Evaluation request failed');
    const data = await res.json();

    badge.textContent = 'Evaluation Complete';
    badge.style.color = '#10b981';

    // Populate Metrics
    document.getElementById('eval-composite-score').textContent = `${data.scores.compositeTotal} / 100`;
    document.getElementById('eval-maintainability').textContent = data.quality.maintainabilityIndex;
    document.getElementById('eval-complexity').textContent = data.quality.cyclomaticComplexity;
    document.getElementById('eval-algo-complexity').textContent = data.performance.complexity;

    // Breakdown Bars
    renderScoreBreakdown(data.scores);

    // Issues List
    renderIssues(data.quality.issues);

    // Refresh Leaderboard and VCS
    fetchLeaderboard();
    fetchVCSHistory();

    showToast(`Evaluation completed! Composite Score: ${data.scores.compositeTotal}`);
  } catch (err) {
    badge.textContent = 'Error';
    badge.style.color = '#f43f5e';
    showToast('Error running evaluation: ' + err.message);
  }
}

function renderScoreBreakdown(scores) {
  const container = document.getElementById('score-breakdown-bars');
  const items = [
    { label: 'Functionality (30%)', val: scores.functionality, max: 30 },
    { label: 'Code Quality (15%)', val: scores.codeQuality, max: 15 },
    { label: 'Performance (10%)', val: scores.performance, max: 10 },
    { label: 'Database & Delta (10%)', val: scores.database, max: 10 },
    { label: 'Documentation (10%)', val: scores.documentation, max: 10 },
    { label: 'Security Audit (5%)', val: scores.security, max: 5 },
    { label: 'Innovation (Judge 10%)', val: scores.innovation, max: 10 },
    { label: 'UI/UX (Judge 5%)', val: scores.uiUx, max: 5 },
    { label: 'Presentation (Judge 5%)', val: scores.presentation, max: 5 },
  ];

  container.innerHTML = items.map(item => {
    const pct = Math.min(100, Math.round((item.val / item.max) * 100));
    return `
      <div class="bar-row">
        <span style="min-width: 140px;">${item.label}</span>
        <div class="bar-bg">
          <div class="bar-fill" style="width: ${pct}%"></div>
        </div>
        <span style="min-width: 40px; text-align: right; font-weight: 600;">${item.val}</span>
      </div>
    `;
  }).join('');
}

function renderIssues(issues) {
  const container = document.getElementById('issues-list');
  const countSpan = document.getElementById('issues-count');
  countSpan.textContent = issues ? issues.length : 0;

  if (!issues || issues.length === 0) {
    container.innerHTML = '<p class="placeholder-text" style="color: #34d399;">✓ Perfect Clean Code! Zero vulnerabilities or anti-patterns detected.</p>';
    return;
  }

  container.innerHTML = issues.map(is => `
    <div class="issue-item ${is.severity}">
      <div class="issue-meta">
        <span style="color: #cbd5e1;">[Line ${is.line}] ${is.rule}</span>
        <span class="badge ${is.severity.toLowerCase()}">${is.severity}</span>
      </div>
      <div>${is.message}</div>
    </div>
  `).join('');
}

// AlgoPro Empirical Profiler Setup
function setupAlgoPro() {
  const btnRun = document.getElementById('btn-run-profile');
  btnRun.addEventListener('click', runAlgoProfiler);
}

async function runAlgoProfiler() {
  const algo = document.getElementById('select-algo').value;
  const badge = document.getElementById('profile-fit-badge');
  badge.textContent = 'Profiling...';

  try {
    const res = await fetch(`${API_BASE}/api/profile`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ algorithm: algo })
    });

    if (!res.ok) throw new Error('Profiler failed');
    const data = await res.json();

    badge.textContent = `Complexity: ${data.empiricalComplexity} (R² = ${data.rSquaredFit})`;

    // Populate Table
    const tbody = document.getElementById('profile-table-body');
    tbody.innerHTML = data.dataPoints.map(pt => `
      <tr>
        <td><strong>N = ${pt.n}</strong></td>
        <td>${pt.timeUs.toFixed(1)} µs</td>
        <td>${pt.comps.toLocaleString()}</td>
        <td>${pt.swaps.toLocaleString()}</td>
      </tr>
    `).join('');

    // Draw SVG Chart
    drawProfileChart(data.dataPoints, data.empiricalComplexity);

    // Notes
    updateAlgoNotes(algo, data.empiricalComplexity);
    showToast(`Empirical Big-O classified as ${data.empiricalComplexity}`);
  } catch (err) {
    badge.textContent = 'Error';
    showToast('Profiler error: ' + err.message);
  }
}

function drawProfileChart(points, complexity) {
  const svg = document.getElementById('profile-chart');
  if (!points || points.length === 0) return;

  const w = 700;
  const h = 320;
  const pad = { top: 30, right: 40, bottom: 40, left: 60 };

  const maxN = Math.max(...points.map(p => p.n));
  const maxTime = Math.max(...points.map(p => p.timeUs));

  const scaleX = (n) => pad.left + (n / maxN) * (w - pad.left - pad.right);
  const scaleY = (t) => h - pad.bottom - (t / (maxTime || 1)) * (h - pad.top - pad.bottom);

  let pathD = '';
  points.forEach((p, idx) => {
    const x = scaleX(p.n);
    const y = scaleY(p.timeUs);
    pathD += (idx === 0) ? `M ${x} ${y}` : ` L ${x} ${y}`;
  });

  let svgHtml = `
    <!-- Grid lines -->
    <line x1="${pad.left}" y1="${h - pad.bottom}" x2="${w - pad.right}" y2="${h - pad.bottom}" stroke="#334155" stroke-width="1.5" />
    <line x1="${pad.left}" y1="${pad.top}" x2="${pad.left}" y2="${h - pad.bottom}" stroke="#334155" stroke-width="1.5" />

    <!-- Y Axis Labels -->
    <text x="${pad.left - 10}" y="${scaleY(maxTime)}" fill="#94a3b8" font-size="11" text-anchor="end">${maxTime.toFixed(0)}µs</text>
    <text x="${pad.left - 10}" y="${scaleY(maxTime/2)}" fill="#94a3b8" font-size="11" text-anchor="end">${(maxTime/2).toFixed(0)}µs</text>
    <text x="${pad.left - 10}" y="${h - pad.bottom}" fill="#94a3b8" font-size="11" text-anchor="end">0µs</text>

    <!-- Curve Line -->
    <path d="${pathD}" fill="none" stroke="#38bdf8" stroke-width="3" stroke-linecap="round" />
  `;

  // Plot Data Points
  points.forEach(p => {
    const x = scaleX(p.n);
    const y = scaleY(p.timeUs);
    svgHtml += `
      <circle cx="${x}" cy="${y}" r="5" fill="#8b5cf6" stroke="#ffffff" stroke-width="1.5" />
      <text x="${x}" y="${h - pad.bottom + 18}" fill="#94a3b8" font-size="10" text-anchor="middle">N=${p.n}</text>
    `;
  });

  svg.innerHTML = svgHtml;
}

function updateAlgoNotes(algo, complexity) {
  const box = document.getElementById('algo-notes');
  const details = {
    quick_sort: { name: 'QuickSort', notes: 'Uses pivot partitioning (Divide and Conquer). Average case O(N log N) scaling with low cache miss rate.' },
    merge_sort: { name: 'MergeSort', notes: 'Guaranteed O(N log N) worst-case comparison sort using auxiliary array merging.' },
    bubble_sort: { name: 'BubbleSort', notes: 'Iteratively swaps adjacent out-of-order elements. Incurs quadratic O(N²) comparisons on reverse inputs.' },
    binary_search: { name: 'BinarySearch', notes: 'Halves the search space at each comparison. Logarithmic O(log N) complexity.' },
    linear_search: { name: 'LinearSearch', notes: 'Sequential single-pass scan through array elements. Exact linear O(N) comparisons.' }
  };

  const item = details[algo] || { name: algo, notes: 'Algorithm profiled successfully.' };
  box.innerHTML = `
    <h4>${item.name} Analysis (${complexity})</h4>
    <p>${item.notes}</p>
  `;
}

// CodeVault (VCS & Delta Storage) Setup
function setupVCS() {
  const btnCommit = document.getElementById('btn-create-commit');
  const btnRefresh = document.getElementById('btn-refresh-history');
  const btnMerge = document.getElementById('btn-test-merge');

  btnCommit.addEventListener('click', createCommit);
  btnRefresh.addEventListener('click', fetchVCSHistory);
  btnMerge.addEventListener('click', testThreeWayMerge);
}

async function createCommit() {
  const author = document.getElementById('commit-author').value || 'Nagesh';
  const message = document.getElementById('commit-message').value || 'Updated C++ logic';
  const code = document.getElementById('commit-code').value;

  if (!code) {
    showToast('Please enter code content to commit.');
    return;
  }

  try {
    const res = await fetch(`${API_BASE}/api/version-control/commit`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ author, message, code })
    });

    if (!res.ok) throw new Error('Commit failed');
    const data = await res.json();

    document.getElementById('commit-message').value = '';
    fetchVCSHistory();
    fetchBackendStatus();
    showToast(`Commit created! Delta savings: ${data.storageSavingsPercent.toFixed(1)}%`);
  } catch (err) {
    showToast('Commit error: ' + err.message);
  }
}

async function fetchVCSHistory() {
  try {
    const res = await fetch(`${API_BASE}/api/version-control/history`);
    if (!res.ok) throw new Error('Failed to fetch VCS history');
    const commits = await res.json();

    const list = document.getElementById('vcs-history-list');
    list.innerHTML = commits.map(c => `
      <div class="commit-card">
        <div class="commit-header">
          <span class="commit-hash">commit #${c.commitHash.substring(0, 10)}</span>
          <span class="commit-time">${c.timestamp}</span>
        </div>
        <div class="commit-msg">${c.message}</div>
        <div class="commit-meta">
          <span>Author: <strong>${c.author}</strong></span>
          <span>+${c.linesAdded} / -${c.linesDeleted} lines</span>
          <span class="delta-savings-tag">${c.storageSavingsPercent.toFixed(1)}% saved</span>
        </div>
        <div class="diff-preview">
          ${c.deltas.slice(0, 8).map(d => {
            const cls = d.op === '+' ? 'plus' : (d.op === '-' ? 'minus' : 'context');
            return `<div class="diff-line ${cls}">${d.op} [L${d.line}] ${escapeHtml(d.content)}</div>`;
          }).join('')}
          ${c.deltas.length > 8 ? `<div style="color: #64748b;">... (${c.deltas.length - 8} more delta lines)</div>` : ''}
        </div>
      </div>
    `).join('');
  } catch (err) {
    console.error(err);
  }
}

function testThreeWayMerge() {
  const resultBox = document.getElementById('merge-test-result');
  resultBox.textContent = 
`[3-Way Merge Test Simulation]
Base Ancestor:  int compute(int x) { return x * 2; }
Local (Branch): int compute(int x) { return x * 2 + 10; }
Remote (Branch): int compute(int x) { return x * 3; }

>>> Conflict Detected:
<<<<<<< HEAD (Local)
int compute(int x) { return x * 2 + 10; }
=======
int compute(int x) { return x * 3; }
>>>>>>> Remote

Status: CodeVault flagged merge conflict successfully!`;
}

// Leaderboard Setup
function setupLeaderboard() {
  const btnRefresh = document.getElementById('btn-refresh-leaderboard');
  const searchInput = document.getElementById('leaderboard-search');

  btnRefresh.addEventListener('click', fetchLeaderboard);
  searchInput.addEventListener('input', () => {
    fetchLeaderboard(searchInput.value.toLowerCase());
  });
}

async function fetchLeaderboard(filter = '') {
  try {
    const res = await fetch(`${API_BASE}/api/leaderboard`);
    if (!res.ok) throw new Error('Leaderboard fetch failed');
    let entries = await res.json();

    if (filter) {
      entries = entries.filter(e => 
        e.teamName.toLowerCase().includes(filter) || 
        e.projectName.toLowerCase().includes(filter) ||
        e.members.toLowerCase().includes(filter)
      );
    }

    const tbody = document.getElementById('leaderboard-body');
    tbody.innerHTML = entries.map(e => {
      let rankCls = 'rank-n';
      if (e.rank === 1) rankCls = 'rank-1';
      else if (e.rank === 2) rankCls = 'rank-2';
      else if (e.rank === 3) rankCls = 'rank-3';

      return `
        <tr>
          <td><span class="rank-badge ${rankCls}">${e.rank}</span></td>
          <td>
            <strong>${e.teamName}</strong>
            <div style="font-size: 0.75rem; color: #94a3b8;">${e.projectName}</div>
          </td>
          <td style="font-size: 0.8rem; color: #cbd5e1;">${e.members}</td>
          <td>${e.scores.codeQuality}</td>
          <td>${e.scores.performance}</td>
          <td>${e.scores.functionality}</td>
          <td>
            <span style="color: #c4b5fd;">${e.scores.innovation} / ${e.scores.uiUx} / ${e.scores.presentation}</span>
          </td>
          <td class="score-cell text-emerald">${e.compositeTotal}</td>
          <td>
            <button class="secondary-btn" style="padding: 0.3rem 0.6rem; font-size: 0.75rem;" onclick="openJudgeModal('${e.teamId}', '${escapeHtml(e.teamName)}')">
              Judge Score
            </button>
          </td>
        </tr>
      `;
    }).join('');
  } catch (err) {
    console.error(err);
  }
}

// Judge Modal Setup
function setupJudgeModal() {
  const modal = document.getElementById('judge-modal');
  const btnClose = document.getElementById('btn-close-modal');
  const btnCancel = document.getElementById('btn-cancel-modal');
  const btnSave = document.getElementById('btn-save-judge');

  const sInno = document.getElementById('slider-inno');
  const sUI = document.getElementById('slider-ui');
  const sPres = document.getElementById('slider-pres');

  sInno.addEventListener('input', () => { document.getElementById('val-inno').textContent = `${sInno.value} / 10`; });
  sUI.addEventListener('input', () => { document.getElementById('val-ui').textContent = `${sUI.value} / 10`; });
  sPres.addEventListener('input', () => { document.getElementById('val-pres').textContent = `${sPres.value} / 10`; });

  btnClose.addEventListener('click', () => modal.classList.add('hidden'));
  btnCancel.addEventListener('click', () => modal.classList.add('hidden'));

  btnSave.addEventListener('click', async () => {
    try {
      const res = await fetch(`${API_BASE}/api/judge`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          teamId: currentJudgeTeamId,
          innovation: parseFloat(sInno.value),
          ui: parseFloat(sUI.value),
          presentation: parseFloat(sPres.value)
        })
      });

      if (!res.ok) throw new Error('Failed to submit judge rubric');
      modal.classList.add('hidden');
      fetchLeaderboard();
      showToast('Judge scores submitted! Leaderboard updated.');
    } catch (err) {
      showToast('Error saving judge scores: ' + err.message);
    }
  });
}

function openJudgeModal(teamId, teamName) {
  currentJudgeTeamId = teamId;
  document.getElementById('modal-team-name').textContent = teamName;
  document.getElementById('judge-modal').classList.remove('hidden');
}

// Toast Helpers
function showToast(msg) {
  const t = document.getElementById('toast');
  t.textContent = msg;
  t.classList.remove('hidden');
  setTimeout(() => t.classList.add('hidden'), 3500);
}

function escapeHtml(str) {
  if (!str) return '';
  return str.replace(/&/g, '&amp;')
            .replace(/</g, '&lt;')
            .replace(/>/g, '&gt;')
            .replace(/"/g, '&quot;')
            .replace(/'/g, '&#039;');
}
