import pandas as pd
import matplotlib.pyplot as plt

# 1. 讀取 C++ 引擎剛才輸出的黃金交易紀錄
try:
    df = pd.read_csv('logs/Best_RSI_Strategy_trades.csv')
except FileNotFoundError:
    print("找不到 CSV 檔案，請確認 C++ 是否已成功執行並產出 Best_RSI_Strategy_trades.csv")
    exit()

# 2. 將微秒時間戳轉換為易讀的 datetime 格式
df['datetime'] = pd.to_datetime(df['timestamp'], unit='us')

# 3. 計算累計資金曲線 (Equity Curve)
df['cumulative_pnl'] = df['net_pnl'].cumsum()

# 4. 建立高質量的雙軸圖表 (適合放進期末影片)
fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(14, 8), gridspec_kw={'height_ratios': [3, 1]}, sharex=True)

# ===== 上半部：價格與買賣點標記 =====
# 畫出成交價格連線
ax1.plot(df['datetime'], df['price'], color='black', alpha=0.4, label='Execution Price')

# 標記買入點 (綠色上箭頭)
buy_signals = df[df['action'] == 'BUY']
ax1.scatter(buy_signals['datetime'], buy_signals['price'], marker='^', color='green', s=120, label='Buy Signal', zorder=5)

# 標記賣出點 (紅色下箭頭)
sell_signals = df[df['action'] == 'SELL']
ax1.scatter(sell_signals['datetime'], sell_signals['price'], marker='v', color='red', s=120, label='Sell Signal', zorder=5)

ax1.set_title('QuantCore: RSI Strategy Backtest Execution', fontsize=16, fontweight='bold')
ax1.set_ylabel('Price (USDT)', fontsize=12)
ax1.legend(loc='upper right')
ax1.grid(True, linestyle='--', alpha=0.5)

# ===== 下半部：資金曲線 (Equity Curve) =====
ax2.plot(df['datetime'], df['cumulative_pnl'], color='blue', linewidth=2, label='Net PnL')
ax2.fill_between(df['datetime'], df['cumulative_pnl'], 0, where=(df['cumulative_pnl'] >= 0), color='green', alpha=0.2)
ax2.fill_between(df['datetime'], df['cumulative_pnl'], 0, where=(df['cumulative_pnl'] < 0), color='red', alpha=0.2)

ax2.set_title('Cumulative Equity Curve', fontsize=14, fontweight='bold')
ax2.set_ylabel('PnL (USDT)', fontsize=12)
ax2.set_xlabel('Time', fontsize=12)
ax2.grid(True, linestyle='--', alpha=0.5)

# 5. 輸出成高畫質圖片並顯示
plt.tight_layout()
plt.savefig('logs/backtest_dashboard.png', dpi=300)
print("圖表已成功儲存至 logs/backtest_dashboard.png")
# plt.show()