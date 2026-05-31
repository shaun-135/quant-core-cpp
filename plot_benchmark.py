import pandas as pd
import matplotlib.pyplot as plt

try:
    df = pd.read_csv('logs/benchmark.csv')
except FileNotFoundError:
    print("找不到 benchmark.csv")
    exit()

plt.figure(figsize=(10, 6))

# 畫出兩種資料結構的時間消耗
plt.plot(df['window_size'], df['linear_time_us'] / 1000, marker='o', color='red', label='LinearBuffer O(N)')
plt.plot(df['window_size'], df['circular_time_us'] / 1000, marker='o', color='green', label='CircularBuffer O(1)')

plt.title('Time Complexity Analysis: Linear vs Circular Buffer', fontsize=14, fontweight='bold')
plt.xlabel('Sliding Window Size (Period)', fontsize=12)
plt.ylabel('Execution Time (Milliseconds)', fontsize=12)
plt.legend(fontsize=12)
plt.grid(True, linestyle='--', alpha=0.7)

# 儲存圖片
plt.tight_layout()
plt.savefig('logs/benchmark_complexity.png', dpi=300)
print("效能對比圖已成功儲存至 logs/benchmark_complexity.png")
plt.show()