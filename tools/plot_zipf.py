import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from pathlib import Path

CSV_PATH = Path("C:/Вуз/Вуз 3 курс/7 sem/infoshr/lab_cpp/build/zipf_data.csv")

def main():
    if not CSV_PATH.exists():
        print(f"Файл {CSV_PATH} не найден. Сначала запусти C++ программу.")
        return

    print("Загрузка данных...")
    try:
        df = pd.read_csv(CSV_PATH, encoding='utf-8', encoding_errors='replace')
    except:
        df = pd.read_csv(CSV_PATH, encoding='cp1251', encoding_errors='replace')
    
    df = df.sort_values(by='frequency', ascending=False).reset_index(drop=True)
    
    df['rank'] = df.index + 1
    
    print(f"Топ-10 самых частых слов:\n{df.head(10)}")

    ranks = df['rank'].values
    freqs = df['frequency'].values

    C = freqs[0]
    zipf_theoretical = C / ranks

    plt.figure(figsize=(10, 6))
    
    plt.loglog(ranks, freqs, label='Real Data (Corpus)', color='blue', alpha=0.6, linewidth=2)
    
    plt.loglog(ranks, zipf_theoretical, label='Theoretical Zipf Law (f = C/r)', color='red', linestyle='--', linewidth=2)

    plt.title('Zipf Law: Rank vs Frequency (Log-Log Scale)')
    plt.xlabel('Rank (log)')
    plt.ylabel('Frequency (log)')
    plt.legend()
    plt.grid(True, which="both", ls="-", alpha=0.2)
    
    output_img = "zipf_plot.png"
    plt.savefig(output_img)
    print(f"\nГрафик сохранен в файл: {output_img}")
    plt.show()

if __name__ == "__main__":
    main()