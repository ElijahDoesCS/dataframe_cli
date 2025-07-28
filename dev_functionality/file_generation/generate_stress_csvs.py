import csv
from itertools import cycle, islice
from pathlib import Path

# === CONFIGURATION ===
WORDLIST_PATH = Path("./dev_functionality/file_generation/wordlist.txt")
OUTPUT_DIR = Path("./dataframes")
START_INDEX = 6  # Start numbering from example6

# === UTILITY FUNCTIONS ===
def load_wordlist(path):
    if not path.exists():
        raise FileNotFoundError(f"Wordlist not found at {path}")
    with open(path, "r", encoding="utf-8") as f:
        return [line.strip() for line in f if line.strip()]

def generate_headers(wordlist, num_headers):
    return list(islice(cycle(wordlist), num_headers))

def generate_numeric_row(length, value=1):
    return [str(value)] * length

def write_csv(filename, headers, rows):
    filepath = OUTPUT_DIR / filename
    with open(filepath, "w", newline="", encoding="utf-8") as f:
        writer = csv.writer(f)
        writer.writerow(headers)
        writer.writerows(rows)
    print(f"✅ Created {filepath}")

# === TEST CASES ===
def generate_test_cases(wordlist):
    test_index = START_INDEX

    def next_name(desc):
        nonlocal test_index
        name = f"example{test_index}_{desc}.csv"
        test_index += 1
        return name

    # 1. Max single value length (digits only)
    for length in [10, 1000, 100000, 1000000]:
        headers = generate_headers(wordlist, 1)
        long_number = "1" * length
        write_csv(next_name(f"max_value_length_{length}"), headers, [[long_number]])

    # 2. Max number of columns (all digits)
    for cols in [10, 1000, 10000, 100000]:
        headers = generate_headers(wordlist, cols)
        row = generate_numeric_row(cols, 1)
        write_csv(next_name(f"max_columns_{cols}"), headers, [row])

    # 3. Max row total length (by character count)
    for total_chars in [10000, 100000, 1000000]:
        val = "1234567890"  # 10 digits
        num_cols = total_chars // len(val)
        headers = generate_headers(wordlist, num_cols)
        row = [val] * num_cols
        write_csv(next_name(f"max_row_chars_{total_chars}"), headers, [row])

    # 4. Max number of rows
    for num_rows in [100, 1000, 10000, 100000]:
        headers = generate_headers(wordlist, 5)
        row = generate_numeric_row(5, 7)
        rows = [row for _ in range(num_rows)]
        write_csv(next_name(f"max_rows_{num_rows}"), headers, rows)

    # 5. Special "weird" values — now replaced with edge **numerical** values
    specials = [
        ([str(0), str(1), str(999999999)], "small_large_values"),
        ([str(2**31 - 1), str(2**31 - 2), str(1)], "max_int_32bit"),
        ([str(10**100)] * 3, "very_large_integers"),
        (['0', '0', '0'], "zero_values"),
        ([str(int("1" * 1000))] * 3, "1000_digit_values")
    ]
    for values, label in specials:
        headers = generate_headers(wordlist, len(values))
        write_csv(next_name(f"special_{label}"), headers, [values])

    # 6. Combinations
    headers = generate_headers(wordlist, 1000)
    row = generate_numeric_row(1000, 42)
    write_csv(next_name("combo_1000x1000"), headers, [row for _ in range(1000)])

    headers = generate_headers(wordlist, 100)
    long_value = "9" * 1000
    write_csv(next_name("combo_100x1000char"), headers, [[long_value] * 100])

    headers = generate_headers(wordlist, 100000)
    row = generate_numeric_row(100000, 8)
    write_csv(next_name("combo_100000_values_short"), headers, [row])

# === MAIN ===
if __name__ == "__main__":
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    wordlist = load_wordlist(WORDLIST_PATH)
    generate_test_cases(wordlist)




