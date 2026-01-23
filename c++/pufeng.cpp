#include <iostream>
#include <vector>
#include <thread>
#include <random>
#include <chrono>
#include <cmath>
#include <mutex>
#include <atomic>
#include <iomanip>
#include <string>

// 蒲丰投针实验线程函数
void buffon_needle_simulation(int num_throws, double needle_length, double line_spacing,
                              std::atomic<int>& intersections, 
                              std::atomic<int>& progress_counter) {
    // 为每个线程创建独立的随机数生成器
    std::random_device rd;
    std::mt19937 thread_rng(rd());
    
    // 创建均匀分布
    std::uniform_real_distribution<> distance_dist(0.0, line_spacing / 2.0);
    std::uniform_real_distribution<> angle_dist(0.0, M_PI);
    
    int local_intersections = 0;
    
    for (int throw_number = 0; throw_number < num_throws; ++throw_number) {
        // 随机生成针的中心到最近平行线的距离和角度
        double distance_to_line = distance_dist(thread_rng);
        double angle = angle_dist(thread_rng);
        
        // 判断针是否与线相交
        if (distance_to_line <= (needle_length / 2) * std::sin(angle)) {
            local_intersections++;
        }
        
        // 更新进度计数器
        progress_counter++;
    }
    
    // 累加交点数
    intersections += local_intersections;
}

// 显示进度条的函数
void show_progress_bar(int current, int total, double elapsed_seconds, int bar_width = 50) {
    float progress = static_cast<float>(current) / total;
    
    // 计算进度条填充数量
    int pos = static_cast<int>(bar_width * progress);
    
    // 创建进度条字符串
    std::string bar = "[";
    for (int i = 0; i < bar_width; ++i) {
        if (i < pos) bar += "=";
        else if (i == pos) bar += ">";
        else bar += " ";
    }
    bar += "]";
    
    // 计算ETA（预计剩余时间）
    double eta = 0.0;
    if (current > 0) {
        eta = elapsed_seconds / current * (total - current);
    }
    
    // 输出进度条
    std::cout << "\r";
    std::cout << bar;
    std::cout << " " << std::fixed << std::setprecision(2) << progress * 100.0 << "%";
    std::cout << " [" << current << "/" << total << "]";
    std::cout << " 已用时: " << std::setprecision(1) << elapsed_seconds << "s";
    std::cout << " ETA: " << std::setprecision(1) << eta << "s";
    std::cout.flush();
}

// 并行计算
double parallel_buffon_needle_simulation(int num_throws, double needle_length, 
                                         double line_spacing, int num_threads) {
    std::atomic<int> intersections(0);
    std::atomic<int> progress_counter(0);
    
    int throws_per_thread = num_throws / num_threads;
    
    // 启动时间
    auto start_time = std::chrono::steady_clock::now();
    
    // 创建线程向量
    std::vector<std::thread> threads;
    
    // 启动工作线程
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(buffon_needle_simulation, throws_per_thread, 
                            needle_length, line_spacing, 
                            std::ref(intersections), std::ref(progress_counter));
    }
    
    // 显示进度条
    bool show_progress = true;
    while (show_progress) {
        // 获取当前进度和耗时
        int current_progress = progress_counter.load();
        auto current_time = std::chrono::steady_clock::now();
        double elapsed_seconds = std::chrono::duration<double>(current_time - start_time).count();
        
        // 显示进度条
        show_progress_bar(current_progress, num_throws, elapsed_seconds);
        
        // 检查是否完成
        if (current_progress >= num_throws) {
            show_progress = false;
            break;
        }
        
        // 休眠一段时间避免过度占用CPU
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    // 完成时显示100%
    auto end_time = std::chrono::steady_clock::now();
    double total_elapsed = std::chrono::duration<double>(end_time - start_time).count();
    show_progress_bar(num_throws, num_throws, total_elapsed);
    std::cout << std::endl; // 换行
    
    // 等待所有工作线程完成
    for (auto& thread : threads) {
        thread.join();
    }
    
    // 使用公式估算π
    if (intersections > 0) {
        double pi_estimate = (2.0 * needle_length * num_throws) / (intersections * line_spacing);
        return pi_estimate;
    } else {
        return 0.0;
    }
}

int main() {
    
    // 参数设置
    double needle_length = 1.0;
    double line_spacing = 2.0;
    int num_throws = 20000000;  // 2000万次投针
    int num_threads = std::thread::hardware_concurrency();  // 使用硬件支持的线程数
    
    if (num_threads == 0) {
        num_threads = 8;  // 如果无法获取硬件线程数，默认使用8个线程
    }
    
    std::cout << "开始蒲丰投针实验..." << std::endl;
    std::cout << "投针次数: " << num_throws << std::endl;
    std::cout << "使用线程数: " << num_threads << std::endl;
    
    // 开始计时
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // 使用并行计算
    double pi_estimate = parallel_buffon_needle_simulation(num_throws, needle_length, 
                                                           line_spacing, num_threads);
    
    // 结束计时
    auto end_time = std::chrono::high_resolution_clock::now();
    auto elapsed_time = std::chrono::duration_cast<std::chrono::duration<double>>(end_time - start_time);
    
    std::cout << std::fixed << std::setprecision(10);
    std::cout << "\n估算的 π 值为: " << pi_estimate << std::endl;
    std::cout << "实际的 π 值为: " << M_PI << std::endl;
    std::cout << "相对误差: " << std::setprecision(6) 
              << std::abs(pi_estimate - M_PI) / M_PI * 100 << "%" << std::endl;
    std::cout << "总耗时: " << std::setprecision(2) << elapsed_time.count() << " 秒" << std::endl;
    
    return 0;
}
