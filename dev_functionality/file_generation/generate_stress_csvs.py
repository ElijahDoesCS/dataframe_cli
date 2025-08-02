import os
import random

import random
import os

def geenerate_stress_csvs_num_rows():
    # Generate stress test CSV files for row count:
    exponents = list(range(1, 8))  # Exponents from 1 to 8
    for exp in exponents: 
        i = 10 ** exp
        with open(f"./dataframes/number_of_rows/example_rows_10^{exp}.csv", "w") as f:
            for j in range(i):
                for k in range(3):
                    cell = str(random.randint(1, 1000))
                    if k ==2:
                        f.write(cell + "\n")
                    else:
                        f.write(cell + ",")
        print(f'[✅] Generated stress test CSV file with 10^{exp} rows\n')

def generate_stress_csvs_char_length(headers):
    os.makedirs("./dataframes/row_character_size", exist_ok=True)
    exponents = list(range(2, 9))  # Exponents from 2 to 8
    for exp in exponents:
        i = 10 ** exp
        with open(f"./dataframes/row_character_size/example_row_length_{exp}.csv", "w") as f:
            row = ""
            for row_index in range(3): 
                if row_index == 0:
                    while len(row) < i:
                        table_item = str(random.randint(0, 1000000))
                        if (len(table_item) + len(row)) < i:
                            row += str(table_item) + ","
                        else:
                            row += str(table_item[:(i - len(row))])
                f.write(row + "\n")

def generate_stress_csvs_single_value(headers):
    exponents = list(range(1, 27))

    # Generate stress test CSV files for single values
    for exp in exponents:
        i = 10 ** exp
        with open(f"./dataframes/single_values/example_single_value_10^{exp}.csv", "w") as f:
            for j in range(10):
                for k in range(10):
                    if j == 0:
                        # Write the headers
                        if k != 9: 
                            f.write(random.choice(headers) + ",")
                        else:
                            f.write(random.choice(headers) + "\n")
                    else:
                        if k != 9:
                            f.write(str(random.randint(1, i)) + ",")
                        else:
                            f.write(str(random.randint(1, i)) + "\n")
            print(f'[✅] Generated stress test CSV file with 10^{exp} single values\n')

def generate_stress_csvs_num_columns():
    # Generate stress test CSV files for row width (columns per row):
    exponents = list(range(1, 6))

    for exp in exponents: # 1000000000, 10000000000, 100000000000, 1000000000000, 10000000000000
        i = 10 ** exp
        with open(f"./dataframes/number_of_columns/example_columns_{exp}.csv", "w") as f:
            for j in range(2):
                for k in range(i):
                    if k == i - 1:
                        f.write(str(1) + "\n")
                    else: 
                        f.write(str(1) + ",")  

if __name__ == "__main__":
    # Ensure the directory exists
    os.makedirs("./dataframes", exist_ok=True)

     # Load headers from wordlist file
    headers = []
    with open("./dev_functionality/file_generation/wordlist.txt", "r") as f:
        for line in f:
            headers.append(line.strip())
    print("Length of headers wordlist: " + str(len(headers)))

    # generate_stress_csvs_single_value(headers)
    # generate_stress_csvs_num_columns()
    # generate_stress_csvs_char_length(headers)
    geenerate_stress_csvs_num_rows()

    print("[✅] Generated stress test CSV files in ./dataframes\n")

# Generate stress test cases for 



# 3. Row Length (Total Characters)
# R1_row_length_10000.csv

# R2_row_length_100000.csv

# R3_row_length_1000000.csv

# 4. Total Row Count
# N1_100_rows.csv

# N2_1000_rows.csv

# N3_10000_rows.csv

# N4_100000_rows.csv

# N5_1000000_rows.csv

# 5. Special / Edge Values
# S1_quotes.csv

# S2_commas.csv

# S3_newlines.csv

# S4_empty.csv

# S5_mixed_weird.csv


