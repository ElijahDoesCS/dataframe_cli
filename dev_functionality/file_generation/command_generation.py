import os
import random

csv_dir = "./dataframes"
output_file = "./dev_functionality/valgrind/example_commands/commands.txt"

# Create an example command per dataframe file
csv_files = sorted([
    f for f in os.listdir(csv_dir)
    if f.startswith("example") and f.endswith(".csv")
])

with open(output_file, "w") as f_out:
    with open("./dev_functionality/file_generation/wordlist.txt", "r") as f_in:
        lines = f_in.read().splitlines()

    for csv in csv_files:
        
        # Choose 5 random words
        words = [random.choice(lines) for _ in range(5)]

        def random_arg():
            choice = random.choice(["word", "full", "number"])
            if choice == "word":
                return random.choice(words)
            elif choice == "full":
                return "full"
            else:
                return random.randint(1,10)

        # Generate ranges within 0–10 (They can be in reversed order)
        y_start = random_arg() 
        y_end = random_arg()
        x_start = random_arg()
        x_end = random_arg()

        # Write arguments as expected by load_data
        args = [
            f"./dataframes/{csv}",
            str(y_start),
            str(y_end),
            str(x_start),
            str(x_end)
        ]

        f_out.write(" ".join(args) + "\n")

print(f"[✅] Generated {len(csv_files)} test input line(s) in {output_file}")

