#include <iostream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <iomanip>

// Structure to store individual memory access details
struct AccessLog {
    int row;
    int column;
    int delay_cycles;
};

// ==========================================
// MODULE 1: DRAM TIMING SIMULATOR
// ==========================================
class DRAMTimingSimulator {
public:
    int ras_delay;
    int cas_delay;
    int precharge_delay;
    int total_cycles;
    int last_row;
    std::vector<AccessLog> access_log;

    DRAMTimingSimulator(int ras = 45, int cas = 15, int precharge = 28) {
        ras_delay = ras;
        cas_delay = cas;
        precharge_delay = precharge;
        total_cycles = 0;
        last_row = -1; // -1 means no row is currently open in the row buffer
    }

    void access_memory(int row, int column) {
        int delay = 0;

        if (last_row == -1) {
            // First access ever: Row activation + Column access
            delay = ras_delay + cas_delay;
        } else if (row == last_row) {
            // Row buffer HIT: Only Column Access Strobe delay
            delay = cas_delay;
        } else {
            // Row buffer MISS: Precharge old row + Activate new row + Column access
            delay = precharge_delay + ras_delay + cas_delay;
        }

        total_cycles += delay;
        last_row = row;

        access_log.push_back({row, column, delay});
        std::cout << "Accessed Row " << row << ", Column " << column 
                  << " -> Delay: " << delay << " cycles\n";
    }

    void summary() {
        std::cout << "\n--- DRAM ACCESS SUMMARY ---\n";
        std::cout << "Total Memory Accesses : " << access_log.size() << "\n";
        std::cout << "Total Execution Cycles: " << total_cycles << "\n";
    }

    void reset() {
        total_cycles = 0;
        last_row = -1;
        access_log.clear();
    }
};

// ==========================================
// MODULE 2: ACCESS PATTERN OPTIMIZER
// ==========================================
class AccessPatternOptimizer {
public:
    std::vector<std::pair<int, int>> original_sequence;

    AccessPatternOptimizer(const std::vector<std::pair<int, int>>& sequence) {
        original_sequence = sequence;
    }

    std::unordered_map<int, int> identify_hotspots() {
        std::unordered_map<int, int> row_frequency;
        for (const auto& req : original_sequence) {
            row_frequency[req.first]++;
        }
        return row_frequency;
    }

    std::vector<std::pair<int, int>> optimize_sequence() {
        // Group columns by their respective rows
        std::unordered_map<int, std::vector<int>> grouped_by_row;
        for (const auto& req : original_sequence) {
            grouped_by_row[req.first].push_back(req.second);
        }

        // Get unique rows sorted by access frequency (Hotspots first)
        std::vector<std::pair<int, int>> frequencies;
        for (const auto& pair : identify_hotspots()) {
            frequencies.push_back(pair);
        }
        std::sort(frequencies.begin(), frequencies.end(), [](const auto& a, const auto& b) {
            return a.second > b.second; // Descending order
        });

        // Reorder requests to minimize row changes
        std::vector<std::pair<int, int>> optimized_sequence;
        for (const auto& freq : frequencies) {
            int row = freq.first;
            for (int col : grouped_by_row[row]) {
                optimized_sequence.push_back({row, col});
            }
        }
        return optimized_sequence;
    }

    int simulate_sequence(const std::vector<std::pair<int, int>>& seq, int ras, int cas, int precharge) {
        int cycles = 0;
        int last = -1;
        for (const auto& req : seq) {
            if (last == -1) {
                cycles += (ras + cas);
            } else if (req.first == last) {
                cycles += cas;
            } else {
                cycles += (precharge + ras + cas);
            }
            last = req.first;
        }
        return cycles;
    }

    void report(int ras, int cas, int precharge) {
        std::cout << "\n--- MEMORY ACCESS PATTERN OPTIMIZER ---\n";
        std::cout << "Row Access Frequencies (Hotspots):\n";
        for (const auto& pair : identify_hotspots()) {
            std::cout << "  Row " << pair.first << ": " << pair.second << " times\n";
        }

        int original_cycles = simulate_sequence(original_sequence, ras, cas, precharge);
        std::vector<std::pair<int, int>> optimized_seq = optimize_sequence();
        int optimized_cycles = simulate_sequence(optimized_seq, ras, cas, precharge);

        std::cout << "Original Access Delay  : " << original_cycles << " cycles\n";
        std::cout << "Optimized Access Delay : " << optimized_cycles << " cycles\n";
        std::cout << "Access Delay Reduced by: " << (original_cycles - optimized_cycles) << " cycles\n";
    }
};

// ==========================================
// MODULE 3: PERFORMANCE EVALUATION MODULE
// ==========================================
class PerformanceEvaluator {
private:
    DRAMTimingSimulator& sim;
public:
    PerformanceEvaluator(DRAMTimingSimulator& dram_sim) : sim(dram_sim) {}

    void evaluate() {
        if (sim.access_log.empty()) {
            std::cout << "\n--- PERFORMANCE REPORT ---\nNo access logs found.\n";
            return;
        }

        std::cout << "\n--- PERFORMANCE REPORT ---\n";
        std::cout << "Total Accesses      : " << sim.access_log.size() << "\n";
        std::cout << "Total Cycles        : " << sim.total_cycles << "\n";
        std::cout << "Average Cycles      : " << std::fixed << std::setprecision(2)
                  << (double)sim.total_cycles / sim.access_log.size() << "\n";

        int min_delay = sim.access_log[0].delay_cycles;
        int max_delay = sim.access_log[0].delay_cycles;
        for (const auto& log : sim.access_log) {
            if (log.delay_cycles < min_delay) min_delay = log.delay_cycles;
            if (log.delay_cycles > max_delay) max_delay = log.delay_cycles;
        }

        std::cout << "Fastest Access      : " << min_delay << " cycles\n";
        std::cout << "Slowest Access      : " << max_delay << " cycles\n";
    }
};

// ==========================================
// MAIN EXECUTION
// ==========================================
int main() {
    // 1. Create a DRAM Simulator instance with default project timings
    DRAMTimingSimulator dram(45, 15, 28);

    std::cout << "=== RUNNING MODULE 1: SAMPLE ACCESSES ===\n";
    dram.access_memory(0, 1);
    dram.access_memory(2, 5);
    dram.access_memory(0, 3);
    dram.access_memory(4, 2);

    // 2. Evaluate performance of the raw sequence
    PerformanceEvaluator perf(dram);
    perf.evaluate();

    // 3. Setup a larger sample sequence to test the Module 2 Optimizer
    std::vector<std::pair<int, int>> sample_trace = {
        {0, 1}, {1, 2}, {0, 4}, {2, 1}, {1, 5}, {0, 2}, {2, 3}, {1, 1}, {0, 9}, {1, 7}
    };

    AccessPatternOptimizer optimizer(sample_trace);
    optimizer.report(dram.ras_delay, dram.cas_delay, dram.precharge_delay);

    return 0;
}
